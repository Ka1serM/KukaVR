#pragma once

#include "CoreMinimal.h"
#include "PUREVR/BaseActor/BaseActor.h"
#include "PUREVR/RobotHand/ERobotHandCommand.h"
#include "PUREVR/KeyFrames/frame/scalarInterpolator/ScalarInterpolator.h"
#include "PUREVR/RobotGrabbable/PickupCompatible.h"

#include "PUREVR/Simulation/EGameEvent.h"

#include "RobotHandActor.generated.h"

UCLASS()
class PUREVR_API ARobotHandActor : public ABaseActor {
    GENERATED_BODY()

private:
    bool bDebugLoopGrabItemCommand = false;

    bool lockedHand = false;

public:
    static ARobotHandActor *MakeInstance(UWorld *world);

    ARobotHandActor();

    void LockInteraction(bool lock){
        lockedHand = lock;
    }

    virtual void BeginPlay() override;
    virtual void Tick(float deltatime) override;

    UPROPERTY(EditAnywhere, Category = "RobotHand")
    float innerLength = 40.0f; //40 stimmt so ca. nicht ändern.

    UPROPERTY(EditAnywhere, Category = "RobotHand")
    float SpehereCastRadius = 30.0f;

    void UpdateTransform(FVector &location, FRotator &rotation);

    //void SetVisible(bool flag);

    void SetGripDistance(float distance);

    AActor *MakeSpehereCast();
    AActor *MakeSpehereCast(float radius);

    //will prevent to re pickup dropped items
    APickupCompatible *MakeSpehereCastPickupable();

    void ExecuteCommand(ERobotHandCommand commandIn);
    void ExecuteCommand(ERobotHandCommand commandIn, bool force);
    void NotifyEvent(EGameEvent event);

    //pickup and drop interaction from keyframes
    ///drops current item and tries to pickup a new one
    void PickupItem();

    ///drops current item
    void DropItem();

    //has item picked up flag
    bool HasItem();

    ///dont allow robot movement if command is still executing.
    bool MovementAllowed(){
        return !commandExecuting; 
    }

private:
    void FindComponentsOnBeginPlay();

    float debugDistance = 0.0f;
    float debugSign = 1;

    USceneComponent *handleMain = nullptr;
    USceneComponent *innerLeft = nullptr;
    USceneComponent *innerRight = nullptr;
    USceneComponent *outerLeft = nullptr;
    USceneComponent *outerRight = nullptr;
    USceneComponent *fingerLeft = nullptr;
    USceneComponent *fingerRight = nullptr;

    USceneComponent *fingerRightPivot = nullptr;
    USceneComponent *fingerLeftPivot = nullptr;

    USceneComponent *itemSocketPivot = nullptr;

    void SetRotationLeftGrip(FRotator rotation);
    void SetRotationRightGrip(FRotator rotation);


    void TickGripOpenState(float deltatime);
    void TickDebugPickup();
    void TickUpdatePickuppedItem();
    AActor *pickedUpItem = nullptr;
    FVector CenterBetweenGrips();


    //command state
    bool commandExecuting = false;
    ERobotHandCommand command = ERobotHandCommand::ENone;//might not be needed
    ScalarInterpolator handInterpolator;
    void TickCommand(float deltatime);
    float CurrentGripDistance();
};