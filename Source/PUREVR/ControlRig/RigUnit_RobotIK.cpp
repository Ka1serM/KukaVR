// Copyright PUREVR

#include "RigUnit_RobotIK.h"
#include "Units/RigUnitContext.h"
#include "Math/ControlRigMathLibrary.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(RigUnit_RobotIK)

FRigUnit_RobotIK_Execute()
{
	DECLARE_SCOPE_HIERARCHICAL_COUNTER_RIGUNIT()

	URigHierarchy* Hierarchy = ExecuteContext.Hierarchy;
	if (Hierarchy == nullptr)
	{
		return;
	}

	// Quick sanity
	if (Items.Num() < 2)
	{
		UE_CONTROLRIG_RIGUNIT_REPORT_WARNING(TEXT("RobotIK: Need at least 2 items in the chain."));
		return;
	}

	// Local references to work data
	TArray<FTransform>& Chain = WorkData.ChainTransforms;
	TArray<FCachedRigElement>& CachedItems = WorkData.CachedItems;
	TArray<float>& RotationLimitsPerItem = WorkData.RotationLimitsPerItem;
	FCachedRigElement& CachedEffector = WorkData.CachedEffector;

	// Cache items if not cached yet (or if cache size mismatches)
	if (CachedItems.Num() != Items.Num())
	{
		CachedItems.Reset();
		for (const FRigElementKey& Key : Items)
		{
			CachedItems.Add(FCachedRigElement(Key, Hierarchy));
		}
		CachedEffector = CachedItems.Last();

		// Prepare rotation limits array sized to cached items
		RotationLimitsPerItem.SetNumUninitialized(CachedItems.Num());
		for (int32 i = 0; i < CachedItems.Num(); ++i)
		{
			RotationLimitsPerItem[i] = BaseRotationLimit;
		}

		// Map provided RotationLimits to the per-item array
		for (const FRigUnit_RobotIK_RotationLimit& RL : RotationLimits)
		{
			int32 FoundIdx = INDEX_NONE;
			for (int32 i = 0; i < CachedItems.Num(); ++i)
			{
				if (CachedItems[i].GetKey() == RL.Item)
				{
					FoundIdx = i;
					break;
				}
			}
			if (FoundIdx != INDEX_NONE)
			{
				RotationLimitsPerItem[FoundIdx] = RL.Limit;
			}
		}
	}

	// Initialize current global transforms for the chain from the hierarchy
	Chain.Reset();
	Chain.Reserve(CachedItems.Num());
	for (const FCachedRigElement& C : CachedItems)
	{
		Chain.Add(Hierarchy->GetGlobalTransform(C));
	}

	// Keep a convenience lambda to find an axis constraint for an item index (returns local axis)
	auto FindLocalAxisForIndex = [&](int32 Index, FVector& OutLocalAxis) -> bool
	{
		if (Index < 0 || Index >= CachedItems.Num())
			return false;

		const FRigElementKey& Key = CachedItems[Index].GetKey();
		for (const FRigUnit_RobotIK_AxisConstraint& AC : AxisConstraints)
		{
			if (AC.Item == Key)
			{
				OutLocalAxis = AC.Axis.GetSafeNormal();
				return true;
			}
		}
		return false;
	};

	const FVector EffectorTarget = EffectorTransform.GetLocation();
	const int32 ChainLength = Chain.Num();

	// CCD loop: iterate and apply per-joint single-axis constraints in local space (transformed to world each iteration)
	for (int32 Iter = 0; Iter < MaxIterations; ++Iter)
	{
		bool bSolved = true;

		// iterate joints from effector-1 back to root (we rotate joint i which affects all j > i)
		for (int32 i = ChainLength - 2; i >= 0; --i)
		{
			const FVector RootPos = Chain[i].GetLocation();
			const FVector CurrentEffectorPos = Chain.Last().GetLocation();

			FVector ToEffector = (CurrentEffectorPos - RootPos);
			FVector ToTarget = (EffectorTarget - RootPos);

			// handle degenerate cases
			if (ToEffector.SizeSquared() < SMALL_NUMBER || ToTarget.SizeSquared() < SMALL_NUMBER)
			{
				continue;
			}

			ToEffector.Normalize();
			ToTarget.Normalize();

			// Compute unconstrained rotation axis and angle (world-space)
			FVector RawRotAxis = FVector::CrossProduct(ToEffector, ToTarget);
			float Dot = FMath::Clamp(FVector::DotProduct(ToEffector, ToTarget), -1.f, 1.f);
			float Angle = FMath::Acos(Dot); // in radians

			// If nearly aligned, skip
			if (FMath::IsNearlyZero(Angle, KINDA_SMALL_NUMBER) || RawRotAxis.IsNearlyZero(KINDA_SMALL_NUMBER))
			{
				continue;
			}

			RawRotAxis = RawRotAxis.GetSafeNormal();

			// Apply rotation limit (per-step) in degrees -> convert to radians clamp
			float LimitDeg = RotationLimitsPerItem.IsValidIndex(i) ? RotationLimitsPerItem[i] : BaseRotationLimit;
			float LimitRad = FMath::DegreesToRadians(LimitDeg);
			if (FMath::Abs(Angle) > LimitRad)
			{
				Angle = (Angle > 0.f) ? LimitRad : -LimitRad;
			}

			// If there is a local axis constraint for this joint, transform it into world space
			FVector LocalAxis;
			bool bHasLocalAxis = FindLocalAxisForIndex(i, LocalAxis);

			FVector JointWorldAxis = FVector::ZeroVector;
			if (bHasLocalAxis)
			{
				// Current joint transform in world space is Chain[i]
				const FTransform& JointXfo = Chain[i];

				// Transform local axis to world (no scale)
				JointWorldAxis = JointXfo.TransformVectorNoScale(LocalAxis).GetSafeNormal();

				// If joint world axis is degenerate for some reason, fallback to unconstrained axis
				if (JointWorldAxis.IsNearlyZero())
				{
					JointWorldAxis = RawRotAxis;
				}

				// We must rotate only around JointWorldAxis. To produce the correct angular difference
				// we need to compute the signed angle between the projection of ToEffector and ToTarget onto the plane orthogonal to JointWorldAxis.
				// Project both vectors onto plane perpendicular to JointWorldAxis:
				FVector ToEffectorProj = FVector::VectorPlaneProject(ToEffector, JointWorldAxis);
				FVector ToTargetProj = FVector::VectorPlaneProject(ToTarget, JointWorldAxis);

				// If projections are degenerate (parallel to axis), then rotating around this axis won't help; skip in that case.
				if (ToEffectorProj.SizeSquared() < SMALL_NUMBER || ToTargetProj.SizeSquared() < SMALL_NUMBER)
				{
					continue;
				}
				ToEffectorProj.Normalize();
				ToTargetProj.Normalize();

				// Compute angle between projections and its sign around JointWorldAxis
				float CosProj = FMath::Clamp(FVector::DotProduct(ToEffectorProj, ToTargetProj), -1.f, 1.f);
				float SignedAngle = FMath::Acos(CosProj);

				// Determine sign using cross product
				FVector CrossProj = FVector::CrossProduct(ToEffectorProj, ToTargetProj);
				float Sign = FMath::Sign(FVector::DotProduct(CrossProj, JointWorldAxis));
				SignedAngle *= Sign;

				// Clamp signed angle by LimitRad
				if (FMath::Abs(SignedAngle) > LimitRad)
				{
					SignedAngle = FMath::Clamp(SignedAngle, -LimitRad, LimitRad);
				}

				// Use JointWorldAxis and SignedAngle as the delta rotation
				FQuat DeltaRot(JointWorldAxis, SignedAngle);

				// Apply DeltaRot to all subsequent joints (j >= i+1) positions & orientations
				for (int32 j = i + 1; j < ChainLength; ++j)
				{
					FVector Rel = Chain[j].GetLocation() - RootPos;
					Rel = DeltaRot.RotateVector(Rel);
					FTransform NewXfo = Chain[j];
					NewXfo.SetLocation(RootPos + Rel);

					FQuat NewRot = DeltaRot * Chain[j].GetRotation();
					NewXfo.SetRotation(NewRot);
					Chain[j] = NewXfo;
				}
			}
			else
			{
				// No local axis constraint -> apply unconstrained rotation around RawRotAxis by Angle
				FQuat DeltaRot(RawRotAxis, Angle);

				for (int32 j = i + 1; j < ChainLength; ++j)
				{
					FVector Rel = Chain[j].GetLocation() - RootPos;
					Rel = DeltaRot.RotateVector(Rel);
					FTransform NewXfo = Chain[j];
					NewXfo.SetLocation(RootPos + Rel);

					FQuat NewRot = DeltaRot * Chain[j].GetRotation();
					NewXfo.SetRotation(NewRot);
					Chain[j] = NewXfo;
				}
			}

			// Check convergence
			if ((EffectorTarget - Chain.Last().GetLocation()).Size() > Precision)
			{
				bSolved = false;
			}
		} // end joints loop

		if (bSolved)
		{
			break;
		}
	} // end iterations

	// Apply resulting transforms back to hierarchy (with weight blending)
	for (int32 i = 0; i < ChainLength; ++i)
	{
		FTransform NewXfo = Chain[i];
		if (!FMath::IsNearlyEqual(Weight, 1.f))
		{
			const FTransform OldXfo = Hierarchy->GetGlobalTransform(CachedItems[i]);
			NewXfo = FControlRigMathLibrary::LerpTransform(OldXfo, NewXfo, FMath::Clamp(Weight, 0.f, 1.f));
		}
		Hierarchy->SetGlobalTransform(CachedItems[i], NewXfo, bPropagateToChildren);
	}

	// Blend/set effector final transform
	if (CachedEffector.IsValid())
	{
		if (!FMath::IsNearlyEqual(Weight, 1.f))
		{
			const FTransform OldEff = Hierarchy->GetGlobalTransform(CachedEffector);
			const FTransform Blended = FControlRigMathLibrary::LerpTransform(OldEff, EffectorTransform, FMath::Clamp(Weight, 0.f, 1.f));
			Hierarchy->SetGlobalTransform(CachedEffector, Blended, bPropagateToChildren);
		}
		else
		{
			Hierarchy->SetGlobalTransform(CachedEffector, EffectorTransform, bPropagateToChildren);
		}
	}
}
