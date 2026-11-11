#pragma once

#include "CoreMinimal.h"
#include "ControlRigComponent.h"
#include "ERobotState.h"
#include "FollowHandle.h"
#include "PUREVR/KeyFrames/AnimationManager/AnimationManager.h"
#include "PUREVR/KeyFrames/interpolator/Interpolator.h"
#include "PUREVR/RobotActor/SafeZoneManager/SafeZoneManager.h"
#include "PUREVR/RobotHand/RobotHandActor.h"

#include "PUREVR/BaseActor/BaseActor.h"
#include "PUREVR/Simulation/EGameEvent.h"

#include "PUREVR/PCM/RobotLineActor.h"

#include "RobotActor.generated.h"


UCLASS()
class PUREVR_API ARobotActor : public ABaseActor
{
    GENERATED_BODY()
private:
    bool autoModeCycle = true;
    bool bLogMessage = true;

    // Private constructor for singleton pattern
    ARobotActor();

public:
    UFUNCTION(BlueprintCallable, Category = "Robot")
    void TryAddKeyframe();
    
    //Player FPS Api. -> do not remove, responsible for edit mode adding keyframes
    //if no key frame is placed, the same is returned as currently grabbed 
    AKeyFrame* PlayerApiTryInsertAddKeyframe(FVector position, AKeyFrame *grabbed);


    //clears the current animation and returns a new follow frame IF in EDIT MODE
    AKeyFrame *ClearCurrentAnimation();
    AKeyFrame *ClearCurrentAnimationForce();

    virtual void BeginPlay() override;
    void Initialize();
    virtual void Tick(float DeltaTime) override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

    UFUNCTION(BlueprintCallable, Category = "Robot")
    void OnKeyframeEventReceived(EKeyframeEvent EventType, AKeyFrame* PayloadObject);

    UFUNCTION(BlueprintCallable, Category = "Robot")
    static ARobotActor* GetInstance();

    UFUNCTION(BlueprintCallable, Category = "Robot")
    void SetState(ERobotState NewState);
    ERobotState GetCurrentState() const { return CurrentState; }

    /** Movement speed in units per second */
    UPROPERTY(EditAnywhere, Category = "Robot")
    float MovementVelocity = 200.0f;

    /** Start position for the follow handle / end effector */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Robot")
    FVector FollowStartPosition = FVector(0.0f, 0.0f, 0.0f);

    /** Set the current target frame in Edit or Follow mode */
    void SetCurrentTargetFrame(AKeyFrame* Target);
    void ClearCurrentTargetFrame();

    //debug change state from keyboard, player controller base
    ERobotState PlayerApiEnterNextMode();

    //overrides the follow frame location if dropped frame non null
    //DO NOT REMOVE
    ERobotState PlayerApiEnterNextMode(AKeyFrame *frame);

    //also needed for player controller base
    int GetCurrentModeAsIndex();
    bool IsInRange(AKeyFrame *frame);

    //clears robot data on level switch
    void ClearRobotOnLevelSwitch();

protected:
    // Helper method to notify the simulation actor of events
    void NotifySimulationEvent(EGameEvent Event, void* EventPayload = nullptr);

    void ClearRobotHand();
    void ClearLineActor();
    void ClearFollowFrameVisibility();

private:
    // Tick Handlers
    void TickIdle(float DeltaTime);
    void TickPlayingAnimation(float DeltaTime);
    void TickEditMode(float DeltaTime);
    void TickFollowMode(float DeltaTime);

    // Enter Handlers
    void EnterIdle();
    void EnterPlayingAnimation();
    void EnterEditMode();
    void EnterFollowMode();

    // Idle Animation Helpers
    void PerformRoboticHeadMovement(float DeltaTime);
    void PerformWavingMotion(float WaveProgressParam);


    // End Effector
    void MoveEndEffectorTo(const FVector& WorldPosition);
    FRotator GetEndEffectorWorldRotation() const;
    FVector GetEndEffectorWorldPosition() const;

public:
    //End Effector with handstate
    bool HandHasItem(FVector &outLocation);
    

    //overrides the world follow frame position override (when entering idle mode)
    //triggered on robot switch if keyframe still grabbed by player
    //or on any follow or keyframe drop action.
    //do not remove, much needed for consitent follow frame location (in a logical way!)
    void OverrideFollowFrameWorldPositionOverride(AKeyFrame *frame);

private:
    static ARobotActor* Instance;

    UPROPERTY(VisibleAnywhere, Category = "Robot|State")
    ERobotState CurrentState = ERobotState::StateNone;

    UPROPERTY()
    UControlRigComponent* ControlRigComponent = nullptr;

    AnimationManager AnimManager;

    // Internal Follow frame
    AFollowHandle* FollowFrame = nullptr;
    FVector FollowFrameWorldOverride;
    bool bHasFollowFrameWorldOverride = false;
    FVector FollowFrameWorldPosTargeted();
    void SetFollowFrameDefaultLocation();

    // Current keyframe tracked in Edit mode
    AKeyFrame* CurrentTargetFrame = nullptr;

    //frame tracked for oob
    AKeyFrame *outOfRangeframe = nullptr;

    // Single interpolator for all modes
    LinearInterpolator Interp;

    float ModeTimeAccumulator = 0.0f;   // Accumulates deltaTime (no longer used)

    // Idle animation variables
    float IdleAnimationTimer = 0.0f;
    float IdleAnimationStrength = 100.0f; // Controls the strength of the idle animation
    float TwitchSpeed = 1.0f;          // Controls how fast the twitches happen
    float NextWaveTime = 0.0f;  // Time until next wave action
    bool bIsWaving = false;
    float WaveProgress = 0.0f;  // Progress of wave animation (0.0 to 1.0)
    FVector OriginalHeadPosition;  // Store original head position
    FRotator OriginalHeadRotation;  // Store original head rotation

    //zone updates and setup
    bool onTickSafeZoneUpdate = true;
    bool safeZoneFlagChanged = false;
    ASafeZoneManager *zone = nullptr;

    bool safeZoneScaleIsSetup = false;
    bool safeZoneScaleIsSetupTickMade = false;
    FVector defaultWorldPosSaved;
    void UpdateSafeZoneVisibility();
    void FindSafeZoneOnBeginPlay();
    void SetSafeZoneScaleOnBeginPlay();
    void SetZoneVisible(bool bflag);

    //hand
    USceneComponent *axis6EndPlate = nullptr;
    ARobotHandActor *handActor = nullptr;
    void CreateHandOnBeginPlay();
    
    bool HandStillExecutingCommand();
    void DispatchKeyFrameCommandToHand(AKeyFrame *frame);

    void LockHandBasedOnState();
    void LockHand(bool lock);

    //apply skeletal hook
    void AttachActorToSkeletalEndPlate(AActor *inAttach);
    void AttachActorToSkeletalEndPlate(USceneComponent *inAttach);



    // --- line actor and updates ---
    UPROPERTY()
    ARobotLineActor *lineActor = nullptr;
    void CreateLineActorOnBeginPlay();
    void SetLineState(ERobotState state);

    void UpdateLinePosition();
    void UpdateLinePosition(FVector &position);

    float DistanceToTarget(FVector &endeffectorPos);
};
