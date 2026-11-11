#include "KeyFrame.h"
#include "PUREVR/RobotActor/RobotActor.h"
#include "Engine/World.h"
#include "DebugPlugin/DebugHelper.h"
#include "PUREVR/Simulation/SimulationActor.h"
#include "PUREVR/RobotGrabbable/PickupCompatible.h"
#include "PUREVR/RobotGrabbable/DropZone/DropZoneActor.h"

AKeyFrame* AKeyFrame::SpawnKeyFrame(UWorld* World, const FVector& Location)
{
    return SpawnKeyFrame(World, Location, false);
}

AKeyFrame* AKeyFrame::SpawnKeyFrame(UWorld* World, const FVector& Location, bool bVisible)
{
    if (!World)
    {
        UE_LOG(LogTemp, Warning, TEXT("AKeyFrame::SpawnKeyFrame - Invalid World"));
        return nullptr;
    }

    UClass* SpawnClass = AKeyFrame::StaticClass();
    FActorSpawnParameters SpawnParams;
    SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

    AKeyFrame* KeyFrame = World->SpawnActor<AKeyFrame>(
        SpawnClass,
        Location,
        FRotator::ZeroRotator,
        SpawnParams
    );

    if (!KeyFrame)
    {
        UE_LOG(LogTemp, Error, TEXT("AKeyFrame::SpawnKeyFrame - Spawned actor is not a KeyFrame"));
        return nullptr;
    }
    return KeyFrame;
}

AKeyFrame::AKeyFrame() : Super()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.bStartWithTickEnabled = true;

    static ConstructorHelpers::FObjectFinder<UMaterialInterface> DefaultMaterialObj(TEXT("/Game/Robot/KeyFrame/red.red"));
    if (DefaultMaterialObj.Succeeded())
    {
        DefaultMaterial = DefaultMaterialObj.Object;
    }
}

void AKeyFrame::BeginPlay()
{
    Super::BeginPlay();

    if (StaticMeshComponent)
    {
        StaticMeshComponent->SetWorldScale3D(FVector(0.333f, 0.333f, 0.333f));
    }
    

    CreateMaterial();
    SetRaycastChannel();
    CreateNumberWidget();
    CheckForPickupableInRange();
    CheckForDropZoneInRange();

    DebugHelper::showScreenMessage("AKeyFrame::BeginPlay");
}


bool AKeyFrame::IsGrabbedFlag(){
    return isGrabbed;
}

void AKeyFrame::OnGrabbed_Implementation()
{
    if (ARobotActor* RobotActor = ARobotActor::GetInstance())
    {
        RobotActor->SetCurrentTargetFrame(this);
    }

    if (ASimulationActor* SimActor = ASimulationActor::GetInstance())
    {
        SimActor->OnGameEvent(EGameEvent::EMoveKeyframe, this);
    }
    isGrabbed = true;
    UpdateNumberVisibility();
}

void AKeyFrame::OnReleased_Implementation()
{
    if (ARobotActor* RobotActor = ARobotActor::GetInstance())
    {
        RobotActor->ClearCurrentTargetFrame();
    }
    isGrabbed = false;
    UpdateNumberVisibility();

    CheckForPickupableInRange();
    CheckForDropZoneInRange();
    NotifyRobotFollowHandleLocationOnReleased();
}

void AKeyFrame::NotifyRobotFollowHandleLocationOnReleased(){
    if(ARobotActor *actor = ARobotActor::GetInstance()){
        actor->OverrideFollowFrameWorldPositionOverride(this);
    }
}




void AKeyFrame::SetVisible(bool bVisible)
{
    SetActorHiddenInGame(!bVisible);
    isSetVisible = bVisible;
    UpdateNumberVisibility();
}

void AKeyFrame::CreateMaterial()
{
    UMaterialInterface* Mat = DefaultMaterial;

    if (!Mat)
    {
        Mat = LoadObject<UMaterialInterface>(nullptr, TEXT("/Game/Robot/KeyFrame/red.red"));
    }

    if (Mat && StaticMeshComponent)
    {
        materialInstance = UMaterialInstanceDynamic::Create(Mat, this);
        if (materialInstance)
        {
            StaticMeshComponent->SetMaterial(0, materialInstance);
            SetKeyFrameState(EKeyFrameState::EInvalid);
        }
    }
}

UStaticMeshComponent* AKeyFrame::FindMesh()
{
    return StaticMeshComponent;
}

void AKeyFrame::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    UpdateStateByDistance();

    if (changedState && !interpolator.isFinished())
    {
        float velocity = 1.5f;
        float scalar = interpolator.Tick(DeltaTime, velocity);
        UpdateMaterialInterpolation(scalar);

        if (interpolator.isFinished())
        {
            changedState = false;
        }
    }
}

void AKeyFrame::UpdateMaterialInterpolation(float scalar)
{
    if (materialInstance)
    {
        materialInstance->SetScalarParameterValue("interpolation", scalar);
    }
}

void AKeyFrame::SetKeyFrameState(EKeyFrameState stateIn)
{
    if (stateIn == stateCurrent)
        return;

    stateCurrent = stateIn;
    changedState = true;

    if (stateIn == EKeyFrameState::EDefault)
    {
        interpolator.Setup(1.0f, 0.0f);
    }
    else
    {
        interpolator.Setup(0.0f, 1.0f);
    }
}

void AKeyFrame::UpdateStateByDistance()
{
    if (!isSetVisible) return;

    if (ARobotActor* actor = ARobotActor::GetInstance())
    {
        bool inRange = actor->IsInRange(this);
        EKeyFrameState state = inRange ? EKeyFrameState::EDefault : EKeyFrameState::EInvalid;
        SetKeyFrameState(state);
    }
}

#define KeyframeChannel ECC_GameTraceChannel3
void AKeyFrame::SetRaycastChannel()
{
    if (StaticMeshComponent)
    {
        StaticMeshComponent->SetCollisionResponseToChannel(KeyframeChannel, ECR_Block);
    }
}





//widget
void AKeyFrame::SetNumber(int32 number){
    if(numberWidgetActor){
        numberWidgetActor->SetNumber(number);
    }
}

void AKeyFrame::SetNumberVisible(bool flag){
    if(numberWidgetActor){
        numberWidgetActor->SetVisible(flag);
    }
}

void AKeyFrame::CreateNumberWidget(){
    if(!numberWidgetActor){
        numberWidgetActor = AUINumberWidgetActor::MakeInstance(GetWorld(), this);
        SetNumberVisible(false);
    }
}

void AKeyFrame::SetNumberShouldBeVisible(bool flag){
    
    if(flag){
        numberState = ENumberState::EDisabledWhileGrabbed;
    }else{
        numberState = ENumberState::EDisabled;
    }
    UpdateNumberVisibility();

    if(flag){
        DebugHelper::logMessage("AKeyFrame::setNumberShouldVisible");
    }
}

void AKeyFrame::UpdateNumberVisibility(){
    if(!numberWidgetActor){
        return;
    }
   
    if(isSetVisible){
        if(numberState == ENumberState::EDisabledWhileGrabbed){
            if(!isGrabbed){
                SetNumberVisible(true);
                return;
            }
        }
    }
    SetNumberVisible(false);

    
}




//debug
void AKeyFrame::DebugShowLocation(FColor color){
    FVector Position = GetActorLocation();
    DebugHelper::showLineBetween(GetWorld(), Position, Position + FVector(0, 0, 100), color, 1.0f);
    DebugHelper::showLineBetween(GetWorld(), Position, Position + FVector(0, 100, 0), color, 1.0f);
    DebugHelper::showLineBetween(GetWorld(), Position, Position + FVector(100, 0, 0), color, 1.0f);
}





#define RobotHandCast ECC_GameTraceChannel2 


//change state based on pickupables in reach
void AKeyFrame::CheckForPickupableInRange(){
    command = ERobotHandCommand::ENone;

    if(AActor *actor = MakeSpehereCastByChannel(sphereCastRange, ECC_WorldDynamic)){
        if(APickupCompatible *casted = Cast<APickupCompatible>(actor)){
            command = ERobotHandCommand::EPickupItem;

            //apply location to help robot for pickup
            SetActorLocation(casted->GetActorLocation());
            //DebugHelper::showScreenMessage("AKeyFrame::Released To Pickup Box!", true);
        }
    }
}

void AKeyFrame::CheckForDropZoneInRange(){
    if(AActor *actor = MakeSpehereCastByChannel(sphereCastRange, ECC_WorldDynamic)){
        if(ADropZoneActor *casted = Cast<ADropZoneActor>(actor)){
            //command = ERobotHandCommand::EDropItem;

            //apply location to help robot for pickup
            FVector target = casted->GetActorLocation() + FVector(0, 0, 30); //little bit up
            SetActorLocation(target);

            //DebugHelper::showScreenMessage("AKeyFrame::Released To Drop Zone!", true);
        }
    }
}

