// Copyright PUREVR

#pragma once

#include "Units/Highlevel/RigUnit_HighlevelBase.h"
#include "RigUnit_RobotIK.generated.h"

/**
 * Simple per-item rotation limit (degrees).
 */
USTRUCT()
struct PUREVR_API FRigUnit_RobotIK_RotationLimit
{
	GENERATED_BODY()

	FRigUnit_RobotIK_RotationLimit()
		: Item(FRigElementKey(NAME_None, ERigElementType::Bone))
		, Limit(30.f)
	{}

	/** The item (bone/control) the limit applies to */
	UPROPERTY(meta = (Input))
	FRigElementKey Item;

	/** Limit angle in degrees (used as a per-step clamp) */
	UPROPERTY(meta = (Input, Constant))
	float Limit;
};

/**
 * Axis constraint defined in the *local space* of the bone/control.
 * Axis must be normalized (but we normalize in code if not).
 */
USTRUCT()
struct PUREVR_API FRigUnit_RobotIK_AxisConstraint
{
	GENERATED_BODY()

	FRigUnit_RobotIK_AxisConstraint()
		: Item(FRigElementKey(NAME_None, ERigElementType::Bone))
		, Axis(FVector::ForwardVector)
	{}

	/** The item (bone/control) the axis applies to */
	UPROPERTY(meta = (Input))
	FRigElementKey Item;

	/** Local-space axis (e.g. (1,0,0) for local X). Will be normalized internally. */
	UPROPERTY(meta = (Input))
	FVector Axis;
};

/**
 * Work data (cached between frames for performance)
 */
USTRUCT()
struct PUREVR_API FRigUnit_RobotIK_WorkData
{
	GENERATED_BODY()

	FRigUnit_RobotIK_WorkData()
	{}

	/** Per-chain transforms (global) used during solving. Index corresponds to CachedItems. */
	UPROPERTY()
	TArray<FTransform> ChainTransforms;

	/** Cached item handles for the chain */
	UPROPERTY()
	TArray<FCachedRigElement> CachedItems;

	/** Per-item rotation limits (degrees) - matched to CachedItems */
	UPROPERTY()
	TArray<float> RotationLimitsPerItem;

	/** Cached effector element (last in the chain) */
	UPROPERTY()
	FCachedRigElement CachedEffector;
};

/**
 * Robot IK - CCDIK implementation with local-space single-axis joint constraints.
 */
USTRUCT(meta=(DisplayName="Robot IK", Category="Hierarchy", Keywords="IK,Robot,CCDIK", NodeColor="0 0.8 1"))
struct PUREVR_API FRigUnit_RobotIK : public FRigUnit_HighlevelBaseMutable
{
	GENERATED_BODY()

	RIGVM_METHOD()
	virtual void Execute() override;

	FRigUnit_RobotIK()
		: EffectorTransform(FTransform::Identity)
		, Precision(0.1f)
		, Weight(1.f)
		, MaxIterations(10)
		, bStartFromTail(true)
		, BaseRotationLimit(30.f)
		, bPropagateToChildren(true)
	{}

	/** The chain to use (ordered from root -> effector) */
	UPROPERTY(meta = (Input, ExpandByDefault))
	TArray<FRigElementKey> Items;

	/** The transform of the effector in global space */
	UPROPERTY(meta = (Input))
	FTransform EffectorTransform;

	/** The precision for the solver (distance threshold in world units) */
	UPROPERTY(meta = (Input, Constant))
	float Precision;

	/** Blend weight for applying the result [0..1] */
	UPROPERTY(meta = (Input))
	float Weight;

	/** Maximum number of CCD iterations */
	UPROPERTY(meta = (Input, Constant))
	int32 MaxIterations;

	/** If true the direction of the solver is flipped (not used for single effector chain here but kept for API parity) */
	UPROPERTY(meta = (Input, Constant))
	bool bStartFromTail;

	/** Default per-step rotation limit (degrees) applied to each joint; can be overridden via RotationLimits */
	UPROPERTY(meta = (Input, Constant))
	float BaseRotationLimit;

	/** Optional per-item rotation limits (degrees). Matches by Item key. */
	UPROPERTY(meta = (Input, Constant))
	TArray<FRigUnit_RobotIK_RotationLimit> RotationLimits;

	/** Axis constraints defined in *local space* of the bone/control */
	UPROPERTY(meta = (Input, Constant))
	TArray<FRigUnit_RobotIK_AxisConstraint> AxisConstraints;

	/** If set to true children transforms will be recalculated */
	UPROPERTY(meta = (Input, Constant))
	bool bPropagateToChildren;

	/** Internal working data */
	UPROPERTY()
	FRigUnit_RobotIK_WorkData WorkData;
};
