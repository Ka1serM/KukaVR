// Copyright PUREVR

#pragma once

#include "Units/Highlevel/RigUnit_HighlevelBase.h"
#include "RigUnit_RotateTo.generated.h"

UENUM()
enum class ERobotAxis : uint8
{
	X,
	Y,
	Z,
	NegX,
	NegY,
	NegZ
};

/**
 * Rotates a bone or control to face a given effector transform.
 */
USTRUCT(meta=(DisplayName="Rotate To", Category="Hierarchy", Keywords="LookAt,Aim,Face,Rotate", NodeColor="0.2 0.8 0.4"))
struct PUREVR_API FRigUnit_RotateTo : public FRigUnit_HighlevelBaseMutable
{
	GENERATED_BODY()

	RIGVM_METHOD()
	virtual void Execute() override;

	FRigUnit_RotateTo()
	{
		EffectorTransform = FTransform::Identity;
		ForwardAxis = ERobotAxis::X;
		RotationAxis = ERobotAxis::Z;
		Weight = 1.f;
		bPropagateToChildren = true;
	}

	/** The bone or control to rotate */
	UPROPERTY(meta = (Input))
	FRigElementKey Item;

	/** Target effector transform to face */
	UPROPERTY(meta = (Input))
	FTransform EffectorTransform;

	/** Forward direction of the bone */
	UPROPERTY(meta = (Input, Constant))
	ERobotAxis ForwardAxis;

	/** Axis to rotate around (optional constraint) */
	UPROPERTY(meta = (Input, Constant))
	ERobotAxis RotationAxis;

	/** Blend weight */
	UPROPERTY(meta = (Input))
	float Weight;

	/** If true, propagate transform to children */
	UPROPERTY(meta = (Input, Constant))
	bool bPropagateToChildren;
};
