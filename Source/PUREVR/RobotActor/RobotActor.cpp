#include "RobotActor.h"
#include "FollowHandle.h"
#include "PUREVR/KeyFrames/frame/KeyFrame.h"
#include "Engine/World.h"
#include "DebugPlugin/DebugHelper.h"
#include "PUREVR/KeyFrames/frame/EKeyframeEvent.h"
#include "PUREVR/Simulation/SimulationActor.h"

ARobotActor* ARobotActor::Instance = nullptr;

ARobotActor::ARobotActor() : Super()
{
    // Set as singleton instance in constructor of the actual spawned actor
    Instance = this;
}

ARobotActor* ARobotActor::GetInstance()
{
    return Instance;
}

/*==================================
 * SIMULATION INTEGRATION
 *=================================*/

void ARobotActor::NotifySimulationEvent(EGameEvent Event, void* EventPayload)
{
    if (ASimulationActor* SimActor = ASimulationActor::GetInstance())
    {
        SimActor->OnGameEvent(Event, EventPayload);
    }
}

void ARobotActor::TryAddKeyframe()
{
    
    //new: auch edit animation here
    bool bFlag = CurrentState == ERobotState::StateEditAnimation || CurrentState == ERobotState::StateFollow;

    if(bFlag){
        DebugHelper::showScreenMessage("ARobotActor LEFTMOUSE UP TRY ADD KEYFRAME", FColor::Green);
        KeyFrameAnimation* animation = AnimManager.GetActiveAnimation();
        if (animation){
            animation->AddFrameAtPosition(FollowFrame->GetActorLocation());

            // Notify simulation of keyframe addition
            NotifySimulationEvent(EGameEvent::EAddKeyframe, FollowFrame);

            //animation->SetVisible(false);
        }
    }
}

//trigger by FPS Controller, DO NOT REMOVE!
AKeyFrame* ARobotActor::PlayerApiTryInsertAddKeyframe(FVector position, AKeyFrame *grabbed)
{
    //passed frame is the follow frame.
    if(CurrentState == ERobotState::StateFollow){
        if(grabbed == FollowFrame){
            //DebugHelper::showScreenMessage("ARobotActor::Place::FollowMode OK!", true); //force
            
            //add as default
            TryAddKeyframe();

            //hide animation in Follow Mode!
            if (AnimManager.GetActiveAnimation())
                AnimManager.GetActiveAnimation()->SetVisible(false);

            return grabbed;
        }
    }

    
    //new: auch edit animation here
    bool bFlag = CurrentState == ERobotState::StateEditAnimation; //|| CurrentState == ERobotState::StateFollow;

    if(bFlag){
        DebugHelper::showScreenMessage("ARobotActor LEFTMOUSE UP TRY ADD KEYFRAME", FColor::Green);

        //if (CurrentState == ERobotState::StateFollow) {
        KeyFrameAnimation* animation = AnimManager.GetActiveAnimation();
        if(animation){
            AKeyFrame *framePlaced = animation->PlayerApiAddFrameAtPosition(position, grabbed);
            if(framePlaced){
                // Notify simulation of keyframe addition
                NotifySimulationEvent(EGameEvent::EAddKeyframe, framePlaced);
            }

            //CHEESY FIXES
            /*if(CurrentState == ERobotState::StateFollow){
                DebugHelper::showScreenMessage("ARobotActor::Place::FollowMode!", true); //force
                //SetCurrentTargetFrame(framePlaced);
                FollowFrame = framePlaced;
            }*/

            return framePlaced;
        }
    }



    return grabbed;
}

void ARobotActor::BeginPlay()
{
    Super::BeginPlay();

    // Register this robot with the simulation actor
    ASimulationActor* SimActor = ASimulationActor::GetInstance();
    if (SimActor)
    {
        SimActor->RegisterRobot(this);
    }
    else
    {
        // If simulation actor isn't available yet, we'll set it in Initialize
    }

    // Locate ControlRig
    ControlRigComponent = FindComponentByClass<UControlRigComponent>();
    if (!ControlRigComponent)
    {
        UE_LOG(LogTemp, Error, TEXT("ARobotActor: ControlRigComponent not found!"));
        return;
    }

    // Spawn FollowHandle (guaranteed existence)
    FollowFrame = GetWorld()->SpawnActor<AFollowHandle>(GetActorLocation(), GetActorRotation());
    //FollowFrame->SetVisible(false);
    //FollowFrame->SetNumberShouldBeVisible(false);
    ClearFollowFrameVisibility();

    // Create test animation
    KeyFrameAnimation* TestAnim = AnimManager.CreateAnimation(GetWorld());
    if (!TestAnim) return;

    TestAnim->SetLooping(true);

    FindSafeZoneOnBeginPlay();
    CreateHandOnBeginPlay();
    SetSafeZoneScaleOnBeginPlay();

    const FVector Base = GetActorLocation();
    //TestAnim->AddFrameAtPosition(Base + FVector(200, 0, 100));
    //TestAnim->AddFrameAtPosition(Base + FVector(200, 200, 200));
    //TestAnim->AddFrameAtPosition(Base + FVector(0, 200, 150));

    AnimManager.SetActiveAnimation(TestAnim);

    // Start interpolation toward first frame
    if (TestAnim->GetCurrentTargetFrame())
    {
        Interp.BeginInterpolation(
            GetEndEffectorWorldPosition(),
            TestAnim->GetCurrentTargetFrame()->GetActorLocation(),
            MovementVelocity
        );
    }

    SetState(ERobotState::StateIdle);


    //Debug
//#ifdef WALTER
//#if WALTER
//    if(true)
        //SetState(ERobotState::StateFollow);
//#endif
//#endif

    CreateLineActorOnBeginPlay();
}

void ARobotActor::Initialize()
{
    // Initialize method called after all actors are registered and ready
    // This is where we can safely access other actors and set up references
    
    // Register with simulation actor - this time it should definitely be available
    ASimulationActor* SimActor = ASimulationActor::GetInstance();
    if (SimActor)
    {
        // The simulation actor should already have us as RobotReference due to registration
        // But we can make sure here if needed
        if (SimActor->RobotReference != this)
        {
            SimActor->RobotReference = this;
        }
    }
}
    
void ARobotActor::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    // Notify simulation that robot is being destroyed if needed
    ASimulationActor* SimActor = ASimulationActor::GetInstance();
    if (SimActor)
    {
        if (SimActor->RobotReference == this)
        {
            SimActor->RobotReference = nullptr;
        }
}

    // Clear the singleton instance if this is the active instance
    if (Instance == this)
{
    Instance = nullptr;
    }

    Super::EndPlay(EndPlayReason);
}

//wird nicht benutzt
void ARobotActor::OnKeyframeEventReceived(EKeyframeEvent EventType, AKeyFrame* PayloadObject)
{
    switch (EventType)
    {
    case EKeyframeEvent::KeyframeAdded:
        break;
    case EKeyframeEvent::KeyframeMoved:
            SetCurrentTargetFrame(PayloadObject);
        break;
    case EKeyframeEvent::KeyframeRemoved:
        break;
    default: ;
    }
}

void ARobotActor::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    UpdateSafeZoneVisibility();
    SetSafeZoneScaleOnBeginPlay(); //must be triggered if IK was in next tick after begin play

    //do not update if hand is still in motion / grip item.
    if(HandStillExecutingCommand()){
        return; 
    }
    LockHandBasedOnState();
    UpdateLinePosition();

    switch (CurrentState)
    {
    case ERobotState::StateIdle:
        TickIdle(DeltaTime);
        break;
    case ERobotState::StatePlayingAnimation:
        TickPlayingAnimation(DeltaTime);
        break;
    case ERobotState::StateEditAnimation:
        TickEditMode(DeltaTime);
        break;
    case ERobotState::StateFollow:
        TickFollowMode(DeltaTime);
        break;
    default: break;
    }
}

// Tick Handlers

void ARobotActor::TickIdle(float DeltaTime)
{
    // Update idle animation timer
    IdleAnimationTimer += DeltaTime;

    // Check if it's time to start a wave action (random interval between 15-45 seconds)
    if (!bIsWaving && IdleAnimationTimer >= NextWaveTime)
    {
        // Random chance to start waving (15% chance every 15-45 seconds)
        if (FMath::FRand() < 0.15f)
        {
            bIsWaving = true;
            WaveProgress = 0.0f;
        }
        else
        {
            // Set next wave time (15-45 seconds from now)
            NextWaveTime = IdleAnimationTimer + FMath::FRandRange(15.0f, 45.0f);
        }
    }

    if (bIsWaving)
    {
        // Update wave progress (completes in 2.5 seconds, adjusted by TwitchSpeed)
        // Lower TwitchSpeed values make the wave slower
        WaveProgress += DeltaTime / (2.5f / TwitchSpeed);

        if (WaveProgress >= 1.0f)
        {
            bIsWaving = false;
            WaveProgress = 0.0f;
            // Set next wave time (15-45 seconds from now)
            NextWaveTime = IdleAnimationTimer + FMath::FRandRange(15.0f, 45.0f);
        }

        // Perform waving motion - animate the end effector/arm
        PerformWavingMotion(WaveProgress);
    }
    else
    {
        // Perform robotic head movements (short, quick movements)
        PerformRoboticHeadMovement(DeltaTime);
    }
}

void ARobotActor::TickPlayingAnimation(float DeltaTime)
{
    KeyFrameAnimation* Anim = AnimManager.GetActiveAnimation();
    if (!Anim) return;

    AKeyFrame* TargetFrame = Anim->GetCurrentTargetFrame();
    if (!TargetFrame) return;

    //dispatch command
    DispatchKeyFrameCommandToHand(TargetFrame);


    Interp.UpdateEndPosition(TargetFrame->GetActorLocation());

    bool bCompleted = false;
    FVector NewPos = Interp.Tick(DeltaTime, bCompleted);
    MoveEndEffectorTo(NewPos);

    if (bCompleted)
    {
        // Handle preroll toward frame 0
        if (Anim->GetCurrentFrameIndex() == 0)
        {
            Anim->AdvanceSegment();
            AKeyFrame* Next = Anim->GetCurrentTargetFrame();
            if (Next)
            {
                Interp.BeginInterpolation(NewPos, Next->GetActorLocation(), MovementVelocity);
            }
            return;
        }

        // Normal animation flow
        Anim->AdvanceSegment();

        AKeyFrame* NextFrame = Anim->GetCurrentTargetFrame();
        if (NextFrame)
        {
            Interp.BeginInterpolation(NewPos, NextFrame->GetActorLocation(), MovementVelocity);
        }
        else
        {
            // End of animation sequence
            MoveEndEffectorTo(TargetFrame->GetActorLocation());
            // Notify simulation of animation completion
            NotifySimulationEvent(EGameEvent::ERobotStopAnimation, TargetFrame);
        }
    }
}

void ARobotActor::TickEditMode(float DeltaTime)
{
    if (!CurrentTargetFrame) return;

    FVector CurrentPos = GetEndEffectorWorldPosition();
    FVector TargetPos = CurrentTargetFrame->GetActorLocation();

    if (!CurrentPos.Equals(TargetPos, 0.01f))
    {
        Interp.UpdateEndPosition(TargetPos);
        FVector NewPos = Interp.Tick(DeltaTime);
        MoveEndEffectorTo(NewPos);
    }
}

void ARobotActor::TickFollowMode(float DeltaTime)
{
    FVector CurrentPos = GetEndEffectorWorldPosition();
    FVector TargetPos = FollowFrame->GetActorLocation();

    if (!CurrentPos.Equals(TargetPos, 0.01f))
    {
        Interp.UpdateEndPosition(TargetPos);
        FVector NewPos = Interp.Tick(DeltaTime);
        MoveEndEffectorTo(NewPos);
    }
}


// State Management


void ARobotActor::SetState(ERobotState NewState)
{
    ERobotState OldState = CurrentState;
    CurrentState = NewState;

    // Notify simulation of state change
    if (OldState != CurrentState)
    {
        NotifySimulationEvent(EGameEvent::ERobotStateChanged, nullptr);
    }

    switch (CurrentState)
    {
        case ERobotState::StateIdle:
            if(bLogMessage){
                DebugHelper::showScreenMessage("ARobotActor::SetState: IDLE");
            }
            EnterIdle();
        break;
        case ERobotState::StatePlayingAnimation:
            if(bLogMessage){
                DebugHelper::showScreenMessage("ARobotActor::SetState: PLAY ANIMATION");
            }
            EnterPlayingAnimation();
        break;
        case ERobotState::StateEditAnimation:
            if(bLogMessage){
                DebugHelper::showScreenMessage("ARobotActor::SetState: EDIT ANIMATION");
            }
            EnterEditMode();
            break;
        case ERobotState::StateFollow:
            if(bLogMessage){
                DebugHelper::showScreenMessage("ARobotActor::SetState: FOLLOW");
            }
            EnterFollowMode();
            break;
        default: break;
    }
}


// Enter State Handlers
void ARobotActor::EnterIdle()
{
    if (AnimManager.GetActiveAnimation())
        AnimManager.GetActiveAnimation()->SetVisible(false);

    //if (FollowFrame)
    //    FollowFrame->SetVisible(false);

    ClearFollowFrameVisibility();

    CurrentTargetFrame = nullptr;

    // Initialize idle animation variables
    IdleAnimationTimer = 0.0f;
    IdleAnimationStrength = 4.0f; // Increase the strength of the idle animation (was 1.0)
    TwitchSpeed = 0.5f;          // Make the twitches slower (was 1.0, lower value = slower)
    NextWaveTime = FMath::FRandRange(15.0f, 45.0f);  // First wave between 15-45 seconds
    bIsWaving = false;
    WaveProgress = 0.0f;

    // Store original position and rotation for reference (fixed base)
    OriginalHeadPosition = GetEndEffectorWorldPosition();
    OriginalHeadRotation = GetEndEffectorWorldRotation();

    
}

void ARobotActor::EnterPlayingAnimation()
{
    if (FollowFrame)
        FollowFrame->SetVisible(false);

    KeyFrameAnimation* Anim = AnimManager.GetActiveAnimation();
    if (!Anim) return;

    AKeyFrame* StartFrame = Anim->GetCurrentTargetFrame();
    if (!StartFrame) return;

    FVector CurrentPos = GetEndEffectorWorldPosition();
    FVector StartPos = StartFrame->GetActorLocation();

    // Begin preroll move toward frame 0
    Interp.BeginInterpolation(CurrentPos, StartPos, MovementVelocity);

    CurrentTargetFrame = nullptr;

    NotifySimulationEvent(EGameEvent::ERobotPlayAnimation);
}

void ARobotActor::EnterEditMode()
{
    if (AnimManager.GetActiveAnimation())
        AnimManager.GetActiveAnimation()->SetVisible(true);

    if (FollowFrame){
        FollowFrame->SetVisible(false);
    }
    

    FVector CurrentPos = GetEndEffectorWorldPosition();
    Interp.BeginInterpolation(CurrentPos, CurrentPos, MovementVelocity);

    CurrentTargetFrame = nullptr;

    NotifySimulationEvent(EGameEvent::ERobotEditAnimation);
}






void ARobotActor::SetFollowFrameDefaultLocation(){
    FVector FollowFramePosition = GetActorLocation() + FollowStartPosition;
    if(FollowFrame){
        FollowFrame->SetActorLocation(FollowFramePosition);
    }
} 



FVector ARobotActor::FollowFrameWorldPosTargeted(){
    // Position follow frame using the configured FollowStartPosition offset from the robot
    FVector FollowFramePosition = GetActorLocation() + FollowStartPosition;
    if(bHasFollowFrameWorldOverride){
        //can be overriden.
        FollowFramePosition = FollowFrameWorldOverride;
    }
    return FollowFramePosition;
}

void ARobotActor::EnterFollowMode()
{
    if (AnimManager.GetActiveAnimation())
        AnimManager.GetActiveAnimation()->SetVisible(false);


    FVector FollowFramePosition = FollowFrameWorldPosTargeted();
    FollowFrame->SetActorLocation(FollowFramePosition);
    FollowFrame->SetVisible(true);

    // Move the robot effector to the follow frame position so it's ready for interaction
    FVector FollowFramePos = FollowFrame->GetActorLocation();
    FVector CurrentPos = GetEndEffectorWorldPosition();
    Interp.BeginInterpolation(CurrentPos, FollowFramePos, MovementVelocity);

    CurrentTargetFrame = nullptr;

    
}

void ARobotActor::PerformRoboticHeadMovement(float DeltaTime)
{
    // Only move the end effector if it's not being used for anything else
    // Create robotic movements with smooth interpolation between positions
    // Always use the original stored position as the base to prevent drift

    // Use the stored original position as the fixed base
    FVector BasePosition = OriginalHeadPosition;

    // Use a timer-based approach with phases for robotic behavior
    // Scale the timer by TwitchSpeed (lower values = slower movement)
    float CycleDuration = 5.0f / TwitchSpeed; // Increase the cycle duration to make it slower
    float Phase = FMath::Fmod(IdleAnimationTimer, CycleDuration);
    float PhaseProgress = Phase / CycleDuration; // 0.0 to 1.0

    // Define positions for the robotic movement pattern
    FVector PositionA = BasePosition; // Starting/base position
    FVector PositionB = BasePosition + FVector(-3.0f, 0.0f, 2.0f) * IdleAnimationStrength; // Extended position

    FVector TargetPosition;

    if (PhaseProgress < 0.4f) // First pause (40% of the time) - stay at PositionA
    {
        TargetPosition = PositionA;
    }
    else if (PhaseProgress < 0.45f) // Quick movement (5% of the time) - interpolate from A to B
    {
        float Interpolation = (PhaseProgress - 0.4f) / 0.05f;
        // Use a curve to make movement quick at first (robotic feel)
        float CurvedInterpolation = FMath::Pow(Interpolation, 2.0f); // Quadratic ease in
        TargetPosition = FMath::Lerp(PositionA, PositionB, CurvedInterpolation);
    }
    else if (PhaseProgress < 0.8f) // Second pause (35% of the time) - stay at PositionB
    {
        TargetPosition = PositionB;
    }
    else if (PhaseProgress < 0.85f) // Quick movement back (5% of the time) - interpolate from B to A
    {
        float Interpolation = (PhaseProgress - 0.8f) / 0.05f;
        // Use a curve to make movement quick at first (robotic feel)
        float CurvedInterpolation = FMath::Pow(Interpolation, 2.0f); // Quadratic ease in
        TargetPosition = FMath::Lerp(PositionB, PositionA, CurvedInterpolation);
    }
    else // Final pause (15% of the time) - stay at PositionA
    {
        TargetPosition = PositionA;
    }

    // Move to the calculated position with interpolation
    MoveEndEffectorTo(TargetPosition);
}

void ARobotActor::PerformWavingMotion(float WaveProgressParam)
{
    // For the waving motion, create a robotic waving gesture with smooth interpolation
    // Use the original stored position as the base to prevent drift

    // Use the stored original position as the fixed base
    FVector BasePosition = OriginalHeadPosition;

    // Define the key positions for the wave motion (with increased distances)
    FVector WaveStartPos = BasePosition + FVector(4.0f, 2.5f, 1.5f) * IdleAnimationStrength;
    FVector WaveMidPos = BasePosition + FVector(5.0f, 4.0f, 2.0f) * IdleAnimationStrength;
    FVector WaveEndPos = BasePosition + FVector(4.5f, 2.0f, 1.8f) * IdleAnimationStrength;

    FVector TargetPosition;

    if (WaveProgressParam < 0.1f) // Initial pause at base position (10% of time)
    {
        TargetPosition = BasePosition;
    }
    else if (WaveProgressParam < 0.15f) // Quick move to start (5% of time) - interpolate with easing
    {
        float Interpolation = (WaveProgressParam - 0.1f) / 0.05f;
        float CurvedInterpolation = FMath::Pow(Interpolation, 2.0f); // Quadratic ease in
        TargetPosition = FMath::Lerp(BasePosition, WaveStartPos, CurvedInterpolation);
    }
    else if (WaveProgressParam < 0.35f) // Hold at start position (20% of time)
    {
        TargetPosition = WaveStartPos;
    }
    else if (WaveProgressParam < 0.4f) // Quick move to mid (5% of time) - interpolate with easing
    {
        float Interpolation = (WaveProgressParam - 0.35f) / 0.05f;
        float CurvedInterpolation = FMath::Pow(Interpolation, 2.0f); // Quadratic ease in
        TargetPosition = FMath::Lerp(WaveStartPos, WaveMidPos, CurvedInterpolation);
    }
    else if (WaveProgressParam < 0.6f) // Hold at mid position (20% of time)
    {
        TargetPosition = WaveMidPos;
    }
    else if (WaveProgressParam < 0.65f) // Quick move to end (5% of time) - interpolate with easing
    {
        float Interpolation = (WaveProgressParam - 0.6f) / 0.05f;
        float CurvedInterpolation = FMath::Pow(Interpolation, 2.0f); // Quadratic ease in
        TargetPosition = FMath::Lerp(WaveMidPos, WaveEndPos, CurvedInterpolation);
    }
    else if (WaveProgressParam < 0.85f) // Hold at end position (20% of time)
    {
        TargetPosition = WaveEndPos;
    }
    else if (WaveProgressParam < 0.9f) // Quick return to start (5% of time) - interpolate with easing
    {
        float Interpolation = (WaveProgressParam - 0.85f) / 0.05f;
        float CurvedInterpolation = FMath::Pow(Interpolation, 2.0f); // Quadratic ease in
        TargetPosition = FMath::Lerp(WaveEndPos, WaveStartPos, CurvedInterpolation);
    }
    else // Slow return to base (10% of time) - interpolate linearly
    {
        float ReturnAlpha = (WaveProgressParam - 0.9f) / 0.1f;
        TargetPosition = FMath::Lerp(WaveStartPos, BasePosition, ReturnAlpha);
    }

    // Move to the calculated position with interpolation
    MoveEndEffectorTo(TargetPosition);
}


// Target Frame


void ARobotActor::SetCurrentTargetFrame(AKeyFrame* Target)
{
    if (!Target) return;

    // Do NOT allow selecting FollowFrame as an edit target
    if (Target == FollowFrame)
    {
        UE_LOG(LogTemp, Warning, TEXT("Attempted to set CurrentTargetFrame to FollowFrame. Ignored."));
        return;
    }

    // Only valid in Edit or Follow mode (Follow mode can still select normal keyframes for preview)
    if (CurrentState != ERobotState::StateEditAnimation &&
        CurrentState != ERobotState::StateFollow)
    {
        UE_LOG(LogTemp, Warning, TEXT("Cannot set target frame outside Edit or Follow mode"));
        return;
    }

    CurrentTargetFrame = Target;

    // Auto-begin interpolation if in Edit mode only
    if (CurrentState == ERobotState::StateEditAnimation)
    {
        FVector CurrentPos = GetEndEffectorWorldPosition();
        FVector TargetPos = Target->GetActorLocation();

        Interp.BeginInterpolation(CurrentPos, TargetPos, MovementVelocity);
    }

    // Notify simulation of keyframe selection
    NotifySimulationEvent(EGameEvent::ESelectKeyframe, Target);
}

void ARobotActor::ClearCurrentTargetFrame()
{
    CurrentTargetFrame = nullptr;
}

// End Effector Helpers
void ARobotActor::MoveEndEffectorTo(const FVector& WorldPosition)
{
    FVector LocalPos = GetTransform().InverseTransformPosition(WorldPosition);

    ControlRigComponent->SetControlPosition(
        FName("IK_Effector"),
        LocalPos,
        EControlRigComponentSpace::ActorSpace
    );


}

FVector ARobotActor::GetEndEffectorWorldPosition() const
{
    return ControlRigComponent->GetControlPosition(
        FName("IK_Effector"),
        EControlRigComponentSpace::WorldSpace
    );
}

FRotator ARobotActor::GetEndEffectorWorldRotation() const {
    return ControlRigComponent->GetControlRotator(
        FName("IK_Effector"),
        EControlRigComponentSpace::WorldSpace
    );
}




//debug

ERobotState ARobotActor::PlayerApiEnterNextMode(){
    TArray<ERobotState> allStates = {
        ERobotState::StateIdle,
        ERobotState::StatePlayingAnimation,
        ERobotState::StateEditAnimation,
        ERobotState::StateFollow
    };

    int index = -1;
    for (int i = 0; i < allStates.Num(); i++){
        if(allStates[i] == CurrentState){
            index = i;
            break;
        }
    }
    index = (index + 1) % allStates.Num();


    SetState(allStates[index]);
    SetLineState(allStates[index]);
    return allStates[index];
}

ERobotState ARobotActor::PlayerApiEnterNextMode(AKeyFrame *frame){
    //DebugHelper::showScreenMessage("ARobotActor::PlayerApiEnterNextMode::", true);
    OverrideFollowFrameWorldPositionOverride(frame);
    return PlayerApiEnterNextMode();
}

void ARobotActor::OverrideFollowFrameWorldPositionOverride(AKeyFrame *frame){
    if(frame){
        FollowFrameWorldOverride = frame->GetActorLocation();
        bHasFollowFrameWorldOverride = true;
        if(FollowFrame){
            FollowFrame->SetActorLocation(FollowFrameWorldOverride);
        }
        //DebugHelper::showScreenMessage("ARobotActor::PlayerApiEnterNextMode::OVERRIDE LOCATION", true);
    }
}


//keyframe range check
bool ARobotActor::IsInRange(AKeyFrame *frame){
    if(frame){
        
        if(zone){
            bool result = zone->IsInRange(frame);

            //update visibility of safe zone on next tick.
            if(!result){
                onTickSafeZoneUpdate = true;
                outOfRangeframe = frame;
            }
            return result;
        }

        // --- deprecated! ---
    
        //erstmal per distanz
        float distMax = 450.0f;
        distMax = distMax * distMax;

        float dist = FVector::DistSquared(frame->GetActorLocation(), GetActorLocation());
        dist = std::max(0.0f, dist);
        //DebugHelper::showScreenMessage("RobotActor target Dist ", std::sqrt(dist));

        //flag safe zone enabled
        if(dist >= distMax){
            onTickSafeZoneUpdate = true;
            outOfRangeframe = frame;
        }

        return dist <= distMax;
    }
    return false;
}







void ARobotActor::FindSafeZoneOnBeginPlay(){
    AActor *zonefound = FindChildActorByName("SafeZoneManager");
    if(zonefound){
        if(ASafeZoneManager *casted = Cast<ASafeZoneManager>(zonefound)){
            //handeld internally
            zone = casted;
            SetZoneVisible(false);
        }
    }else{

        zone = ASafeZoneManager::MakeInstance(GetWorld());
        if(zone){

            zone->SetActorLocation(GetActorLocation());

            // Einen anderen Actor an die Root des Managers anhängen
            zone->AttachToComponent(
                GetRootComponent(),
                FAttachmentTransformRules::KeepRelativeTransform
            );
            
        }
    }
}

void ARobotActor::SetSafeZoneScaleOnBeginPlay(){
    if(safeZoneScaleIsSetup){
        return;
    }
    if(zone){
        if(zone->ScaleTo(500)){ //450
            //is checked.
            safeZoneScaleIsSetup = true;
            //DebugHelper::showScreenMessage("ARobotActor::Scale Zone", true);
        }
    }



    //DOES NOT WORK AS EXPECTED
    /*
    if(safeZoneScaleIsSetup){
        return;
    }
    if(zone && handActor){
       
        
        if(!safeZoneScaleIsSetupTickMade){
            safeZoneScaleIsSetupTickMade = true;
            defaultWorldPosSaved = handActor->GetActorLocation();
            FVector random = GetActorLocation() + FVector(2000, 0, 0);
            MoveEndEffectorTo(random);

            return; //tick once for end effector.
        }

        
        FVector max = handActor->GetActorLocation() - GetActorLocation();
        if(max.Size() > 1.0f && safeZoneScaleIsSetupTickMade){

            DebugHelper::logMessage("ARobotActor::SetSafeZoneScaleOnBeginPlay: scale: ", max.Size());
            zone->ScaleTo(max.Size());
            safeZoneScaleIsSetup = true;
            MoveEndEffectorTo(defaultWorldPosSaved);
        }
    }*/
}

    




void ARobotActor::SetZoneVisible(bool bflag){
    if(zone){
        //zone->SetVisible(bflag);
        zone->UpdateVisibility(outOfRangeframe, this, bflag);
    }
}

void ARobotActor::UpdateSafeZoneVisibility(){
    //hier noch unklar wie es aktualisiert werden soll.
    //Es ist nicht hinreichend dass die keyframes sich selber
    //auf bounds checken und ein und ausschlaten

    //immer dann wenn mindestens einer draussen ist, muss die safe zone anbleiben
    //immer dann wenn alle drin sind muss die sae zone ausbleiben

    SetZoneVisible(onTickSafeZoneUpdate);

    //flag was processed
    onTickSafeZoneUpdate = false;
    outOfRangeframe = nullptr;


    if (zone) {
        zone->SetActorLocation(GetActorLocation());
    }
}






//// ---- hand related ----
void ARobotActor::CreateHandOnBeginPlay(){
    handActor = ARobotHandActor::MakeInstance(GetWorld());
    AttachActorToSkeletalEndPlate(handActor);
}



void ARobotActor::AttachActorToSkeletalEndPlate(AActor *inAttach){
    if(inAttach){
        AttachActorToSkeletalEndPlate(inAttach->GetRootComponent());
    }
}

//new
void ARobotActor::AttachActorToSkeletalEndPlate(USceneComponent *inAttach){
    if(!inAttach){
        return;
    }
    if(USceneComponent *raw = FindSceneComponentByName("SkeletalMesh")){
        if(USkeletalMeshComponent *casted = Cast<USkeletalMeshComponent>(raw)){
            // An den Bone anhängen
            FName boneName(TEXT("Axis_6"));
            inAttach->AttachToComponent(
                casted,
                FAttachmentTransformRules::SnapToTargetNotIncludingScale, // Transform-Regeln
                boneName
            );

            DebugHelper::logMessage("ROBOT ATTACHED HAND TO SKELETAL MESH!");
        }
    }


}



/// ----- HAND DISPATCH AND EXECUTING FLAG -----
bool ARobotActor::HandStillExecutingCommand(){
    if(handActor){
        return !handActor->MovementAllowed();
    }
    return true;
}

void ARobotActor::DispatchKeyFrameCommandToHand(AKeyFrame *frame){

    //IS TESTED.

    //DebugHelper::showScreenMessage("ARobotActor::TryDispatchRobotHandAction");
    if (frame)
    {
        //get command of frame, dispatch. If still executing, its ignored.(?)
        //DebugHelper::showScreenMessage("ARobotActor::TryDispatchRobotHandAction", int(frame->command));
        if(handActor){
            handActor->ExecuteCommand(frame->command);
        }
    }
}


void ARobotActor::LockHandBasedOnState(){
    if(CurrentState == ERobotState::StatePlayingAnimation){
        LockHand(false);
        return;
    }
    LockHand(true);
}

void ARobotActor::LockHand(bool lock){
    if(handActor){
        handActor->LockInteraction(lock);
        //DebugHelper::logMessage("ARobotActor::Lock State Changed Hand");
    }
}


bool ARobotActor::HandHasItem(FVector &outposition){
    if(handActor){
        if(handActor->HasItem()){
            outposition = GetEndEffectorWorldPosition();
            return true;
        }
    }
    return false;
}

//clears the current animation and returns a new follow frame IF in EDIT MODE
AKeyFrame *ARobotActor::ClearCurrentAnimation(){
    bool bFlag = CurrentState == ERobotState::StateEditAnimation;
    if(bFlag){
        return ClearCurrentAnimationForce();
    }
    return nullptr;
}

AKeyFrame *ARobotActor::ClearCurrentAnimationForce(){
    KeyFrameAnimation* animation = AnimManager.GetActiveAnimation();
    if(animation){
        return animation->ClearAnimation();
    }
    return nullptr;
}

/// ---- LINE ACTOR ----
void ARobotActor::CreateLineActorOnBeginPlay(){
    if(!lineActor){
        lineActor = ARobotLineActor::MakeInstance(GetWorld());
        lineActor->SetVisible(false);
        lineActor->SetActorLocation(GetActorLocation());
    }
}


void ARobotActor::SetLineState(ERobotState state){
    if(lineActor){
        if(state == ERobotState::StatePlayingAnimation){
            KeyFrameAnimation* Anim = AnimManager.GetActiveAnimation();
            lineActor->UpdateMesh(Anim);
            lineActor->SetVisible(true);
        }else{
            lineActor->SetVisible(false);
        }
    }
}

void ARobotActor::UpdateLinePosition(){
    FVector target = GetEndEffectorWorldPosition();
    UpdateLinePosition(target);
}

void ARobotActor::UpdateLinePosition(FVector &position){
    if(lineActor){
        float dist = DistanceToTarget(position);
        //lineActor->SetEndEffectorPosition(position);
        lineActor->SetEndEffectorPosition(position, dist); //testing needed!
    }
}

float ARobotActor::DistanceToTarget(FVector &endeffectorPos){
    if(CurrentState == ERobotState::StatePlayingAnimation){
        if(KeyFrameAnimation* Anim = AnimManager.GetActiveAnimation()){
            if(AKeyFrame* TargetFrame = Anim->GetCurrentTargetFrame()){
                return FVector::Dist(endeffectorPos, TargetFrame->GetActorLocation());
            }
        }
    }
    if(CurrentState == ERobotState::StateFollow){
        if(FollowFrame){
            return FVector::Dist(endeffectorPos, FollowFrame->GetActorLocation());
        }
    }
    return 0.0f;
}



//testing needed!
void ARobotActor::ClearRobotOnLevelSwitch() {
    ClearCurrentAnimationForce();
    ClearRobotHand();
    ClearLineActor();

    SetState(ERobotState::StateIdle);
    ClearFollowFrameVisibility();

    DebugHelper::logMessage("ARobotActor::RESET DIGGA");
}

void ARobotActor::ClearRobotHand(){
    if(handActor){
        handActor->ExecuteCommand(ERobotHandCommand::EDropItem, true); //force
    }
}

void ARobotActor::ClearLineActor(){
    if(lineActor){
        lineActor->ClearMesh();
        lineActor->SetVisible(false);
    }
}

void ARobotActor::ClearFollowFrameVisibility(){
    if(FollowFrame){
        FollowFrame->SetVisible(false);
        FollowFrame->SetNumberShouldBeVisible(false);
        SetFollowFrameDefaultLocation();
        DebugHelper::logMessage("ARobotActor::SetFollowFrameInvisible");
    }
}