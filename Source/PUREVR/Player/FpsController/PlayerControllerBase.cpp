// Fill out your copyright notice in the Description page of Project Settings.


#include "PlayerControllerBase.h"

#include "DebugPlugin/DebugHelper.h"
#include "Animation/AnimSequence.h"

#include "GameFramework/Character.h" // Falls noch nicht inkludiert
#include "GameFramework/CharacterMovementComponent.h"

#include "Components/CapsuleComponent.h" // Include for UCapsuleComponent
#include "Camera/CameraComponent.h" // Include for UCameraComponent

#include "Components/WidgetComponent.h"  // <- hier ist UWidgetComponent definiert

#include "PUREVR/Simulation/SimulationActor.h"

APlayerControllerBase *APlayerControllerBase::playerControllerBaseInstance = nullptr;

#define WidgetCast ECC_GameTraceChannel1

// Sets default values
APlayerControllerBase::APlayerControllerBase()
{
    // Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
    PrimaryActorTick.bCanEverTick = true;
    
    aiming = false;

    // Set capsule size
    float radius = 45.0f;
    float halfHeight = 85.0f; //100cm leg, 70 arms / torso = 170 -> 170 /2 = 85.0f
    GetCapsuleComponent()->InitCapsuleSize(radius, halfHeight);

    CameraRef = CreateDefaultSubobject<UCameraComponent>(TEXT("FirstPersonCamera"));
    CameraRef->SetupAttachment(GetCapsuleComponent());
    CameraRef->SetRelativeLocation(FVector(0.0f, 0.0f, halfHeight)); // Position the camera
    CameraRef->bUsePawnControlRotation = true;

    DebugHelper::logMessage("debugsetup player capsule");

    TurnRateGamepad = 90.f;
    LookUpRateGamepad = 90.f;

    // Don't rotate the entire pawn, only the camera should rotate for pitch
    bUseControllerRotationYaw = true;    // Allow yaw rotation (left/right)
    bUseControllerRotationPitch = false; // DON'T rotate pawn for pitch (up/down)
    bUseControllerRotationRoll = false;  // No roll rotation
    
    CameraRef->bUsePawnControlRotation = true; // Camera follows controller rotation

    sprinting = false;

    //Construct widget
    WidgetInteractor = CreateDefaultSubobject<UWidgetInteractionComponent>(TEXT("WidgetInteractor"));
    WidgetInteractor->SetupAttachment(CameraRef);

    // Für 3D-Widgets wichtig
    WidgetInteractor->bEnableHitTesting = true;
    WidgetInteractor->InteractionDistance = 10000.0f;
    WidgetInteractor->TraceChannel = WidgetCast;
}

// Called when the game starts or when spawned
void APlayerControllerBase::BeginPlay()
{
	Super::BeginPlay();
    playerControllerBaseInstance = this;

    if(WidgetInteractor){
        WidgetInteractor->SetRelativeLocation(FVector(50, 50, 0));
    }
    
    // Register this pawn with the simulation actor
	ASimulationActor* SimActor = ASimulationActor::GetInstance();
	if (SimActor)
	{
		SimActor->RegisterPlayer(this);
	}

    CreatePlayerWidgetOnBeginPlay();
    UpdatePlayerWidget(EPlayerUiEvent::ENone);
}

// Called to bind functionality to input
void APlayerControllerBase::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

    // Bind input actions
    PlayerInputComponent->BindAxis("MoveForward", this, &APlayerControllerBase::MoveForward);
    PlayerInputComponent->BindAxis("MoveRight", this, &APlayerControllerBase::MoveRight);
    
    //ok?
    //PlayerInputComponent->BindAxis("Turn", this, &APawn::AddControllerYawInput);
    //PlayerInputComponent->BindAxis("LookUp", this, &APawn::AddControllerPitchInput);
    PlayerInputComponent->BindAxis("TurnRate", this, &APawn::AddControllerYawInput);
    PlayerInputComponent->BindAxis("LookUpRate", this, &APawn::AddControllerPitchInput);


    //bricked
    PlayerInputComponent->BindAxis("TurnRate", this, &APlayerControllerBase::TurnAtRate);
    PlayerInputComponent->BindAxis("LookUpRate", this, &APlayerControllerBase::LookUpAtRate);

    

    PlayerInputComponent->BindAction("InteractKey", IE_Pressed, this, &APlayerControllerBase::interact);
	PlayerInputComponent->BindAction("DropKey", IE_Pressed, this, &APlayerControllerBase::drop);
    PlayerInputComponent->BindAction("JumpKey", IE_Pressed, this, &APlayerControllerBase::Jump);

    PlayerInputComponent->BindAction("sprintKey", IE_Pressed, this, &APlayerControllerBase::sprint);
    PlayerInputComponent->BindAction("sprintKey", IE_Released, this, &APlayerControllerBase::sprint);

    // Bind the key down action, here, not needed
	// PlayerInputComponent->BindAction("InteractKey", IE_Pressed, this, &AplayerScript::OnInteractKeyDown);
	// Bind the key up action
    PlayerInputComponent->BindAction("leftMouse", IE_Pressed, this, &APlayerControllerBase::leftMouseDown);
	PlayerInputComponent->BindAction("leftMouse", IE_Released, this, &APlayerControllerBase::leftMouseUp);


    PlayerInputComponent->BindAction("RKey", IE_Released, this, &APlayerControllerBase::SwitchRobotState);
    PlayerInputComponent->BindAction("CKey", IE_Released, this, &APlayerControllerBase::ClearAnimationKeys);

    PlayerInputComponent->BindAction("PauseKey", IE_Pressed, this, &APlayerControllerBase::PauseGame);
}

// Called every frame
void APlayerControllerBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
    TickKeyFramePlacingAllowedCoolDown(DeltaTime);

    if(!isVrPawn){
        return;
    }

    if(CameraRef && WidgetInteractor){
        FVector Start = CameraRef->GetComponentLocation();
        FRotator Rot = CameraRef->GetComponentRotation();
        WidgetInteractor->SetWorldLocationAndRotation(Start, Rot); //sollte so richtig sein.
    }
    UpdatePlayerWidgetVisibility();
}

//Movement
/**
 * moves the player forward
 */
void APlayerControllerBase::MoveForward(float Value)
{

    if ((Controller != nullptr) && (Value != 0.0f))
    {
        GetCharacterMovement()->MaxWalkSpeed = BASE_SPEED;
        GetCharacterMovement()->MinAnalogWalkSpeed = BASE_SPEED - 1;
        if (sprinting) {
            GetCharacterMovement()->MaxWalkSpeed = SPRINT_SPEED;
            GetCharacterMovement()->MinAnalogWalkSpeed = SPRINT_SPEED - 1;
        }
        

        const FRotator Rotation = Controller->GetControlRotation();
        const FRotator YawRotation(0, Rotation.Yaw, 0);

        const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);

       
        AddMovementInput(Direction, Value);
        
    
        isWalking = true;

        //DebugHelper::showScreenMessage("PlayerControllerBase: Forward", FColor::Green);
    }
}

void APlayerControllerBase::MoveRight(float Value)
{
    
    if ( (Controller != nullptr) && (Value != 0.0f) )
    {
        const FRotator Rotation = Controller->GetControlRotation();
        const FRotator YawRotation(0, Rotation.Yaw, 0);

        const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);
        AddMovementInput(Direction, Value);

    }
}

void APlayerControllerBase::TurnAtRate(float Rate)
{
    float yawRate = Rate * TurnRateGamepad * GetWorld()->GetDeltaSeconds();
    AddControllerYawInput(yawRate);
}

void APlayerControllerBase::LookUpAtRate(float Rate)
{
    


    float DeltaPitch = Rate * LookUpRateGamepad * GetWorld()->GetDeltaSeconds();
    FRotator currentRotaion = Controller->GetControlRotation();
    float newPichAbs = std::abs(currentRotaion.Pitch + DeltaPitch);
    if(newPichAbs < 89.0f){
        AddControllerPitchInput(DeltaPitch);
    }    
    
    if(std::abs(Rate) > 0.0f){
        //DebugHelper::showScreenMessage("PlayerBase PitchUp", DeltaPitch);
    }
    
	//AddControllerPitchInput(Rate * LookUpRateGamepad * GetWorld()->GetDeltaSeconds());
	//UE_LOG(LogTemp, Warning, TEXT("Turning at rate: %f"), Rate);
}



void APlayerControllerBase::Jump(){

    if (CanJump()){
        ACharacter::Jump(); // Calls the base class jump function
    }
}



void APlayerControllerBase::sprint(){
    sprinting = !sprinting;
}

// Called when the actor is being removed from the game or destroyed
void APlayerControllerBase::EndPlay(const EEndPlayReason::Type EndPlayReason){
    if(this == playerControllerBaseInstance){
        playerControllerBaseInstance = nullptr;
    }

    Super::EndPlay(EndPlayReason);
}

AActor* APlayerControllerBase::performRaycast()
{
    if (!CameraRef)
    {
        return nullptr;
    }

    FVector Start = CameraRef->GetComponentLocation();
    FVector End   = Start + (CameraRef->GetComponentRotation().Vector() * 500.0f);

    FHitResult HitResult;
    FCollisionQueryParams Params;
    Params.AddIgnoredActor(this);

    // Radius of the "thick line" (in cm)
    float Radius = 20.0f; // 20 cm thick ray

    FCollisionShape CollisionShape = FCollisionShape::MakeSphere(Radius);

    bool bHit = GetWorld()->SweepSingleByChannel(
        HitResult,
        Start,
        End,
        FQuat::Identity,
        ECC_Visibility,
        CollisionShape,
        Params
    );

    if (bHit)
    {
        AActor* Actor = HitResult.GetActor();
        DebugHelper::showLineBetween(GetWorld(), Start, HitResult.ImpactPoint);
        return Actor;
    }

    return nullptr;
}



bool APlayerControllerBase::performRaycastWidgetHit(FHitResult &hit){
    if(!CameraRef){
        return false;
    }

// Get the camera location and rotation
    FVector CameraLocation = CameraRef->GetComponentLocation();
    FRotator CameraRotation = CameraRef->GetComponentRotation();

    // Define the start and end vectors for the raycast
    FVector Start = CameraLocation;

    float dist = std::max(std::abs(distanceMax), 500.0f);

    //100 = 1m
    //500m = 
    FVector End = Start + (CameraRotation.Vector() * dist); // 7 meters

    // Perform the raycast
    FHitResult HitResult;
    FCollisionQueryParams Params;
    Params.AddIgnoredActor(this); // Ignore the character itself

    bool bHit = GetWorld()->LineTraceSingleByChannel(
        HitResult, 
        Start, 
        End, 
        WidgetCast, //ECC_Visibility, 
        Params
    );



    // If the raycast hit something, log the hit actor's name
    if (bHit)
    {
        if (UPrimitiveComponent *primitve = HitResult.GetComponent())
        {
            DebugHelper::showLineBetween(GetWorld(), HitResult.ImpactPoint, CameraLocation, FColor::Green);
            DebugHelper::showScreenMessage("ANY HIT");
            if (UWidgetComponent* WidgetComp = Cast<UWidgetComponent>(primitve))
            {
                // Wir haben ein 3D Widget getroffen
                DebugHelper::showScreenMessage("ANY HIT 2");
                return true;
            }
        }
    }
    return false;
}








void APlayerControllerBase::drop(){
    DebugHelper::showScreenMessage("PlayerBase DROP", FColor::Orange);
    DropKeyFrame();
    UpdatePlayerWidget(EPlayerUiEvent::EDroppedFrame);
}

/**
 * allows the player to interact (for example picking up a weapon by pressing "E")
 */
void APlayerControllerBase::interact(){
    if(LevelBasedKeyboardInteractionAllowed()){
        DebugHelper::showScreenMessage("PlayerBase INTERACT", FColor::Cyan);
        TryGrabKeyframe();
    }
	
}

void APlayerControllerBase::leftMouseDown(){
    if(WidgetInteractor){
        WidgetInteractor->PressPointerKey(EKeys::LeftMouseButton); //SOLLTE JA SO STIMMEN
        
        /*return;



        //NOT NEEDED
        DebugHelper::showScreenMessage("PlayerBase LEFTMOUSE DOWN", FColor::Red); //print so the interactor must be true.
        FHitResult result;
        if(performRaycastWidgetHit(result)){

            //hilft nicht
            WidgetInteractor->SetCustomHitResult(result);
            DebugHelper::showScreenMessage("PlayerBase LEFTMOUSE DOWN ---> WIDGET UPDATE ", FColor::Green);


            //sollte ja ok sein.
            WidgetInteractor->PressPointerKey(EKeys::LeftMouseButton);
        }else{

            
            

        }*/
    }
}


void APlayerControllerBase::TickKeyFramePlacingAllowedCoolDown(float deltatime){
    if(placeKeyFrameCoolDown > 0.0f){
        placeKeyFrameCoolDown -= deltatime;
    }
}

bool APlayerControllerBase::KeyFramePlacingAllowed(){
    if(placeKeyFrameCoolDown > 0.0f){
        return false;
    }
    //reset timer
    placeKeyFrameCoolDown = 1.0f;
    return true;
}



#include "PUREVR/RobotActor/FollowHandle.h"
bool APlayerControllerBase::IsRobotHandle(AKeyFrame *frame){
    if(frame){
        if(AFollowHandle *casted = Cast<AFollowHandle>(frame)){
            return true;
        }
    }
    return false;
}

void APlayerControllerBase::leftMouseUp(){
    if(WidgetInteractor){
        //DebugHelper::showScreenMessage("PlayerBase LEFTMOUSE UP", FColor::Green);
        WidgetInteractor->ReleasePointerKey(EKeys::LeftMouseButton);


        //keyframe allowed?
        if(!KeyFramePlacingAllowed()){
            return;
        }

        //create new keyframe
        if(ARobotActor *actor = robotInstance()){
            if(grabbedFrame){
                FVector frameLocation = grabbedFrame->GetActorLocation();
                DebugHelper::showScreenMessage("PlayerBase LEFTMOUSE UP TRY ADD KEYFRAME", FColor::Green);
                AKeyFrame *newFrame = actor->PlayerApiTryInsertAddKeyframe(frameLocation, grabbedFrame);

                //unklar ob das hilft.
                if(!newFrame){
                    return;
                }
                if(IsRobotHandle(grabbedFrame)){
                    //do not drop, do not reset.
                    return;
                }

                //reset grabbed frame location!
                //and copy new latest position to reset to
                AKeyFrame *oldFrame = grabbedFrame;
                FVector copyPos = latestPickupLocationToOverrideAfterPlacingKeyframe;
                Pickup(newFrame); // neuer frame picked up to insert easy after.
                oldFrame->SetActorLocation(copyPos);
                latestPickupLocationToOverrideAfterPlacingKeyframe = newFrame->GetActorLocation(); 

               
            }
        }
    }
}

void APlayerControllerBase::SwitchRobotState(){
    if(!LevelBasedKeyboardInteractionAllowed()){
        return;
    }
    
    if (ARobotActor *actor = robotInstance())
    {
        //get new state, pass frame for follow frame location override 
        //(where frame dropped to return to same pos as dropped location!)
        ERobotState state = actor->PlayerApiEnterNextMode(grabbedFrame);

        if(playerUiActor){
            //not bugged any more
            playerUiActor->SetSelectedIndexByState(state); //update state in widget

            //playerUiActor->SwitchToNextMode(); //update state in widget
        }
    }
    //AFTER Robot state has been updated, and follow frame position
    //refreshed if a frame was indeed picked up: Drop Keyframe if one
    //was picked up.
    drop();
    
}

//dispatch game event changes to hud (show interactions)
void APlayerControllerBase::UpdatePlayerWidget(EPlayerUiEvent event){
    if(playerUiActor){
        playerUiActor->ShowControls(event); //update state in widget
    }
}


void APlayerControllerBase::ClearAnimationKeys(){
    if(!LevelBasedKeyboardInteractionAllowed()){
        return;
    }
    if (ARobotActor *actor = robotInstance())
    {
        //actor
        AKeyFrame *frame = actor->ClearCurrentAnimation();
        Pickup(frame);
    }
}

void APlayerControllerBase::ClearPlayerOnLevelSwitch(){
    ERobotState state = ERobotState::StateIdle;

    if (playerUiActor){
        playerUiActor->SetSelectedIndexByState(state); // update state in widget
    }
    drop();
}

FRotator APlayerControllerBase::cameraRotation(){
    FRotator rotator{};
    if(CameraRef) {
        return CameraRef->GetComponentRotation();
    }
    return rotator;
}

FVector APlayerControllerBase::playerLookDir(){
    if(CameraRef){
        return CameraRef->GetForwardVector();
    }
    return FVector(0, 0, 0);
}


void APlayerControllerBase::showCursor(bool show){
    // Assuming you're inside your ACharacter or subclass (e.g., AMyCharacter)
    APlayerController* PlayerController = Cast<APlayerController>(GetController());
    if (PlayerController)
    {
        // Toggle mouse cursor visibility
        PlayerController->bShowMouseCursor = show;

        //game only custom dispatch no ui - CUSTOM DIPATCH CLICKS
        FInputModeGameOnly InputMode;
        PlayerController->SetInputMode(InputMode);
    }
}



/// ---- interaction keyframes ----
void APlayerControllerBase::TryGrabKeyframe(){

    if(!CameraRef){
        return;
    }

    AActor *raycasted = performRaycast();
    if(raycasted == nullptr){
        float radius = 100.0f;
        raycasted = MakeSpehereCast(radius);
    }
    if(raycasted){
        if(AKeyFrame *casted = Cast<AKeyFrame>(raycasted)){
            Pickup(casted);
        }
    }

    /*
    void OnGrabbed();

    void SetVisible(bool bVisible);
    */
}

void APlayerControllerBase::Pickup(AKeyFrame *casted){
    if(casted){
        drop();

        DebugHelper::showScreenMessage("KEY FRAME TRY GRAB!");
        grabbedFrame = casted;


        grabbedFrame->OnGrabbed_Implementation(); //attaches to robot internally

        /*if (ARobotActor* RobotActor = robotInstance())
        {
            RobotActor->SetCurrentTargetFrame(grabbedFrame);
        }*/
       
        latestPickupLocationToOverrideAfterPlacingKeyframe = grabbedFrame->GetActorLocation();

        //attach
        grabbedFrame->AttachToComponent(CameraRef, FAttachmentTransformRules::SnapToTargetNotIncludingScale);
        
        grabbedFrame->SetActorRelativeLocation(FVector(150, 75, 0));

        UpdatePlayerWidget(EPlayerUiEvent::EPickedUpFrame);
    }
}

void APlayerControllerBase::DropKeyFrame(){
    if(grabbedFrame){
        grabbedFrame->DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);

        grabbedFrame->OnReleased_Implementation();
    }
    grabbedFrame = nullptr;
}


AActor *APlayerControllerBase::MakeSpehereCast(float radius){
    radius = std::max(std::abs(radius), 1.0f);

    FHitResult HitResult;
    FVector Start = GetActorLocation();
    FVector End = Start + GetActorForwardVector() * 200.0f; //10cm z.b.
    
    FCollisionShape Sphere = FCollisionShape::MakeSphere(radius);

    // Query-Parameter für Actors, die ignoriert werden sollen
    FCollisionQueryParams QueryParams;
    QueryParams.AddIgnoredActor(this); // optional, dich selbst ignorieren

    bool bHit = GetWorld()->SweepSingleByChannel(
        HitResult,
        Start,
        End,
        FQuat::Identity,
        ECC_WorldDynamic, //pickupable components are tracked in this channel.
        Sphere,
        QueryParams
    );

    if (bHit)
    {
        DebugHelper::showScreenMessage("ARobotHandActor::HitSphere OK!");
        return HitResult.GetActor();
    }

    return nullptr;

}





void APlayerControllerBase::Initialize(){
    OnInitialize();
}

void APlayerControllerBase::OnInitialize(){
    // Update actor references in case they weren't available during constructor
    GetSimulationActorRef();
    GetRobotActorRef();
    GetCameraRef();
    SetupTransitionMaterialOnInitialize();
}

void APlayerControllerBase::GetSimulationActorRef(){
    if (!SimulationActorRef) {
		SimulationActorRef = ASimulationActor::GetInstance();
	}
}

void APlayerControllerBase::GetRobotActorRef(){
    if (!RobotActorRef) {
		RobotActorRef = ARobotActor::GetInstance();
	}
}

void APlayerControllerBase::GetCameraRef(){
    // Resolve camera once
	TArray<UCameraComponent*> Cameras;
	GetComponents(Cameras);
	if (Cameras.Num() > 0)
	{
		CameraRef = Cameras[0];
	}

}


void APlayerControllerBase::SetupTransitionMaterialOnInitialize(){
    // Set up transition material here after initialization is complete
	// But we should set it up on the VRPawn itself, not on the simulation actor
	if (SimulationActorRef && SimulationActorRef->BaseTransitionMaterial)
	{
		// Use the existing SetupTransitionMaterial method on VRPawn
		SetupTransitionMaterial(SimulationActorRef->BaseTransitionMaterial);
		TransitionMaterial = GetTransitionMaterial();
	}
}



	







// Getter for the transition material
UMaterialInstanceDynamic* APlayerControllerBase::GetTransitionMaterial() const
{
	return TransitionMaterial;
}



void APlayerControllerBase::SetupTransitionMaterial(UMaterialInterface* BaseMaterial)
{
	if (!BaseMaterial)
	{
		UE_LOG(LogTemp, Warning, TEXT("SetupTransitionMaterial: Base material is null"));
		return;
	}
	
	if (!CameraRef)
	{
		UE_LOG(LogTemp, Warning, TEXT("SetupTransitionMaterial: No camera found on VR Pawn"));
		return;
	}

	FWeightedBlendables& Blendables = CameraRef->PostProcessSettings.WeightedBlendables;
	UMaterialInterface* BaseMat = BaseMaterial->GetMaterial();

	// Look for existing material
	for (int32 i = 0; i < Blendables.Array.Num(); ++i)
	{
		FWeightedBlendable& Blendable = Blendables.Array[i];
		UMaterialInterface* Material = Cast<UMaterialInterface>(Blendable.Object);

		if (!Material || Material->GetMaterial() != BaseMat)
		{
			continue;
		}

		// Already dynamic?
		TransitionMaterial = Cast<UMaterialInstanceDynamic>(Material);
		if (!TransitionMaterial)
		{
			TransitionMaterial = UMaterialInstanceDynamic::Create(Material, this);
			if (TransitionMaterial)
			{
				Blendable.Object = TransitionMaterial;
			}
		}

		return;
	}

	// Not found → create and add
	TransitionMaterial = UMaterialInstanceDynamic::Create(BaseMaterial, this);
	if (TransitionMaterial)
	{
		Blendables.Array.Add(FWeightedBlendable(1.0f, TransitionMaterial));
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("SetupTransitionMaterial: Failed to create dynamic material instance"));
	}
}




void APlayerControllerBase::CreatePlayerWidgetOnBeginPlay(){
    if(!playerUiActor && CameraRef){
        DebugHelper::logMessage("APlayerControllerBase::AUIPlayerWidgetActor::Launch");
        playerUiActor = AUIPlayerWidgetActor::MakeInstance(GetWorld(), CameraRef);
        if(playerUiActor){
            DebugHelper::logMessage("APlayerControllerBase::AUIPlayerWidgetActor::LaunchOK"); //ok

            //issue: widget still invisible!
        }
    }
}


void APlayerControllerBase::ShowPlayerWidget(ESimulationLevel level){
    bool show = level != ESimulationLevel::ENone;
    if(playerUiActor){
        playerUiActor->SetVisible(show);
    }
}

void APlayerControllerBase::UpdatePlayerWidgetVisibility(){
    if(ASimulationActor *actor = ASimulationActor::GetInstance()){
        ShowPlayerWidget(actor->CurrentLevel);
    }
}

void APlayerControllerBase::PauseGame()
{
    if(ASimulationActor *actor = ASimulationActor::GetInstance()){
        actor->TogglePause();
    }
}


bool APlayerControllerBase::LevelBasedKeyboardInteractionAllowed(){
    if(ASimulationActor *actor = ASimulationActor::GetInstance()){
        bool allowed = actor->CurrentLevel != ESimulationLevel::ENone;
        return allowed;
    }
    return true;
}