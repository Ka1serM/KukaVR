#pragma once


#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "Components/SceneComponent.h"
#include "Camera/CameraComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/Engine.h"

#include "PUREVR/Player/FpsController/PlayerControllerBase.h"

#include "VRPawn.generated.h"

/// <summary>
/// base pawn class
/// </summary>
UCLASS()
class PUREVR_API AVRPawn : 
public APlayerControllerBase //TODO
//public APawn
{
	GENERATED_BODY()

private:
	static AVRPawn* VRPawnInstance;

	// Private constructor for singleton pattern
	AVRPawn();

public:
	

   

    // Getter function to access the singleton instance, creates if doesn't exist
	UFUNCTION(BlueprintCallable, Category = "VRPawn")
	static AVRPawn* GetInstance();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

    

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called when the actor is being removed from the game or destroyed
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;


	// Radius for grabbing objects around the motion controller
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VR Interaction")
	float GrabRadiusFromGripPosition = 20.0f;

	// Held component tracking for both hands
	UPROPERTY(BlueprintReadWrite, Category = "VR Interaction")
	class UGrabComponent* HeldComponentLeft = nullptr;

	UPROPERTY(BlueprintReadWrite, Category = "VR Interaction")
	class UGrabComponent* HeldComponentRight = nullptr;

	// Function that tries to grab an object near the motion controller
	UFUNCTION(BlueprintCallable, Category = "VR Interaction")
	class UGrabComponent* TryGrab(UMotionControllerComponent* MotionController);

	// Function that releases the held component
	UFUNCTION(BlueprintCallable, Category = "VR Interaction")
	bool TryRelease(UGrabComponent* HeldComponent);
};
