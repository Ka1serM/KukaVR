#include "RigUnit_RotateTo.h"
#include "Units/RigUnitContext.h"
#include "Math/ControlRigMathLibrary.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(RigUnit_RotateTo)

static FVector GetAxisVector(ERobotAxis Axis)
{
	switch (Axis)
	{
		case ERobotAxis::X:   return FVector::ForwardVector;
		case ERobotAxis::Y:   return FVector::RightVector;
		case ERobotAxis::Z:   return FVector::UpVector;
		case ERobotAxis::NegX: return -FVector::ForwardVector;
		case ERobotAxis::NegY: return -FVector::RightVector;
		case ERobotAxis::NegZ: return -FVector::UpVector;
		default: return FVector::ForwardVector;
	}
}

FRigUnit_RotateTo_Execute()
{
	DECLARE_SCOPE_HIERARCHICAL_COUNTER_RIGUNIT()

	URigHierarchy* Hierarchy = ExecuteContext.Hierarchy;
	if (Hierarchy == nullptr || !Item.IsValid())
	{
		return;
	}

	FCachedRigElement CachedItem(Item, Hierarchy);
	if (!CachedItem.UpdateCache(Item, Hierarchy))
	{
		return;
	}

	const FTransform EffectorXfo = EffectorTransform;
	FTransform BoneXfo = Hierarchy->GetGlobalTransform(CachedItem);

	// Compute forward vector of the bone in world space
	const FVector ForwardDir = BoneXfo.GetRotation().RotateVector(GetAxisVector(ForwardAxis));

	// Compute desired direction
	const FVector ToTarget = (EffectorXfo.GetLocation() - BoneXfo.GetLocation()).GetSafeNormal();

	if (ToTarget.IsNearlyZero())
	{
		return;
	}

	// Compute rotation delta
	FQuat DeltaRotation = FQuat::FindBetweenNormals(ForwardDir, ToTarget);

	// If constrained around a rotation axis, project the rotation
	FVector RotationAxisVector = GetAxisVector(RotationAxis);
	if (!RotationAxisVector.IsNearlyZero())
	{
		FVector AxisWorld = BoneXfo.GetRotation().RotateVector(RotationAxisVector);
		FVector ToTargetProj = FVector::VectorPlaneProject(ToTarget, AxisWorld).GetSafeNormal();
		FVector ForwardProj = FVector::VectorPlaneProject(ForwardDir, AxisWorld).GetSafeNormal();

		if (!ForwardProj.IsNearlyZero() && !ToTargetProj.IsNearlyZero())
		{
			float Angle = FMath::Acos(FVector::DotProduct(ForwardProj, ToTargetProj));
			FVector Cross = FVector::CrossProduct(ForwardProj, ToTargetProj);
			float Sign = FMath::Sign(FVector::DotProduct(Cross, AxisWorld));
			DeltaRotation = FQuat(AxisWorld, Angle * Sign);
		}
	}

	// Apply weighted rotation
	FQuat TargetRotation = DeltaRotation * BoneXfo.GetRotation();
	FQuat BlendedRotation = FQuat::Slerp(BoneXfo.GetRotation(), TargetRotation, FMath::Clamp(Weight, 0.f, 1.f));

	BoneXfo.SetRotation(BlendedRotation);
	Hierarchy->SetGlobalTransform(CachedItem, BoneXfo, bPropagateToChildren);
}
