#include "VRPawn.h"
#include "Engine/OverlapResult.h"
#include "PUREVR/Simulation/SimulationActor.h"


// Initialize the singleton instance
AVRPawn* AVRPawn::VRPawnInstance = nullptr;

AVRPawn::AVRPawn()
{
 	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bTickEvenWhenPaused = true;

	// Set as singleton instance in constructor of the actual spawned actor
	VRPawnInstance = this;

    isVrPawn = true;
}

AVRPawn* AVRPawn::GetInstance()
{
	return VRPawnInstance;
}

// Called when the game starts or when spawned
void AVRPawn::BeginPlay()
{
	Super::BeginPlay();
	
	
}


void AVRPawn::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	// Clear the singleton instance if this is the active instance
	if (VRPawnInstance == this)
	{
		VRPawnInstance = nullptr;
	}

	Super::EndPlay(EndPlayReason);
}

// Called every frame
void AVRPawn::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}



UGrabComponent* AVRPawn::TryGrab(UMotionControllerComponent* MotionController)
{
	if (!MotionController)
	{
		UE_LOG(LogTemp, Warning, TEXT("TryGrab: MotionController is null"));
		return nullptr;
	}

	FVector GripPosition = MotionController->GetComponentLocation();

	// Set up collision parameters for the query
	FCollisionQueryParams CollisionParams;
	CollisionParams.AddIgnoredActor(this); // Ignore self
	CollisionParams.bTraceComplex = true;

	// Set up collision object parameters for specific object types
	FCollisionObjectQueryParams ObjectParams;
	ObjectParams.AddObjectTypesToQuery(ECC_PhysicsBody); // Use PhysicsBody which we configured for grabbable objects
	// Note: Only querying PhysicsBody since that's what grabbable objects are configured to respond to

	// Perform sphere overlap to find nearby objects
	TArray<FOverlapResult> OverlapResults;

	FCollisionShape CollisionShape = FCollisionShape::MakeSphere(GrabRadiusFromGripPosition);
	GetWorld()->OverlapMultiByObjectType(
		OverlapResults,
		GripPosition,
		FQuat::Identity,
		ObjectParams,
		CollisionShape,
		CollisionParams
	);

	// Find the closest grabbable component
	UGrabComponent* NearestGrabComponent = nullptr;
	float NearestDistanceSquared = BIG_NUMBER;

	for (const FOverlapResult& OverlapResult : OverlapResults)
	{
		if (AActor* OverlapActor = OverlapResult.GetActor())
		{
			// Get all grab components on the overlap actor
			TArray<UGrabComponent*> GrabComponents;
			OverlapActor->GetComponents<UGrabComponent>(GrabComponents);
			for (UActorComponent* Component : GrabComponents)
			{
				if (UGrabComponent* GrabComp = Cast<UGrabComponent>(Component))
				{
					// Calculate distance from grip position to this grab component
					FVector ComponentLocation = GrabComp->GetComponentLocation();
					float DistanceSquared = FVector::DistSquared(GripPosition, ComponentLocation);

					// Check if this is the nearest component found so far
					if (DistanceSquared < NearestDistanceSquared)
					{
						NearestDistanceSquared = DistanceSquared;
						NearestGrabComponent = GrabComp;
					}
				}
			}
		}
	}

	// If we found a grab component, handle the assignment logic
	if (NearestGrabComponent)
	{
		// Determine which hand this is by checking if it's the left motion controller
		// We'll use the component's name as an identifier to determine the hand
		UGrabComponent** TargetHeldComponentPtr = nullptr;
		UGrabComponent** OtherHeldComponentPtr = nullptr;

		// Check if this motion controller is the left one by name or other properties
		// This is a common approach in Unreal VR projects - we check the component name or attach parent
		FString MotionControllerName = MotionController->GetName();
		if (MotionControllerName.Contains(TEXT("Left")) || (MotionController->GetAttachParent() && MotionController->GetAttachParent()->GetName().Contains(TEXT("Left"))))
		{
			TargetHeldComponentPtr = &HeldComponentLeft;
			OtherHeldComponentPtr = &HeldComponentRight;
		}
		else
		{
			TargetHeldComponentPtr = &HeldComponentRight;
			OtherHeldComponentPtr = &HeldComponentLeft;
		}

		// If other hand was holding this component, clear our reference to it
		if (*OtherHeldComponentPtr && *OtherHeldComponentPtr == NearestGrabComponent)
		{
			*OtherHeldComponentPtr = nullptr;
		}

		// Call TryGrab on the grab component to actually grab it
		NearestGrabComponent->TryGrab(MotionController);

		// Set our reference to the grabbed component
		*TargetHeldComponentPtr = NearestGrabComponent;

		return NearestGrabComponent;
	}

	return nullptr;
}

bool AVRPawn::TryRelease(UGrabComponent* HeldComponent)
{
    if (!HeldComponent)
    {
        return false;
    }

    // Check if the component to release is currently held by either hand and release it
    if (HeldComponentLeft == HeldComponent)
    {
        if (HeldComponentLeft->TryRelease())
        {
            HeldComponentLeft = nullptr;
            return true;
        }
    }
    else if (HeldComponentRight == HeldComponent)
    {
        if (HeldComponentRight->TryRelease())
        {
            HeldComponentRight = nullptr;
            return true;
        }
    }

    return false;
}
