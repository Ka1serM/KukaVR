#include "UIPlayerWidgetActor.h"
#include "PUREVR/Simulation/SimulationActor.h"



AUIPlayerWidgetActor *AUIPlayerWidgetActor::MakeInstance(UWorld *world, AActor *attachTo){
    if(world && attachTo){
        return MakeInstance(world, attachTo->GetRootComponent());
    }
    return nullptr;
    
}

AUIPlayerWidgetActor *AUIPlayerWidgetActor::MakeInstance(UWorld *world, USceneComponent *attachTo){
    if(!world || !attachTo){
        return nullptr;
    }

    UClass* SpawnClass = AUIPlayerWidgetActor::StaticClass();
    FActorSpawnParameters SpawnParams;
    SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

    FVector Location(0, 0, 0);
    AUIPlayerWidgetActor *spawned = world->SpawnActor<AUIPlayerWidgetActor>(
        SpawnClass,
        Location,
        FRotator::ZeroRotator,
        SpawnParams
    );

    if(ASimulationActor *actor = ASimulationActor::GetInstance()){
        // Set pause menu to hidden by default
        spawned->SetWidgetClass(actor->PlayerWidgetClass);
    }else{
        return nullptr;
    }

    if (!spawned)
    {
        UE_LOG(LogTemp, Error, TEXT("AUIPlayerWidgetActor::MakeINstance - Spawned actor is not a AUINumberWidgetActor"));
        return nullptr;
    }

    spawned->SetVisible(true);

    //attach
    spawned->AttachToComponent(attachTo, FAttachmentTransformRules::SnapToTargetNotIncludingScale);
    spawned->SetActorRelativeLocation(FVector(50, 0, -20)); //z: -25

    UE_LOG(LogTemp, Error, TEXT("AUIPlayerWidgetActor::MakeINstance - OK!"));

    return spawned;
}

void AUIPlayerWidgetActor::BeginPlay(){
    Super::BeginPlay();
    ApplyScale(widgetScaleSet);
    SetActorEnableCollision(false); //prevent interaction
    SetSelectedIndex(0);
}

void AUIPlayerWidgetActor::Tick(float deltatime){
    Super::Tick(deltatime);
    SetModeZeroInit();
}

void AUIPlayerWidgetActor::ApplyScale(float widgetScale){
    //SetActorScale3D(FVector(widgetScale));
    if(Widget){
        Widget->SetRelativeScale3D(FVector(widgetScale));
    }
}


void AUIPlayerWidgetActor::SetVisible(bool bFlag){
    if(isSetVisible != bFlag){
        isSetVisible = bFlag;
        SetActorHiddenInGame(!bFlag);
        if(RootComponent){
            RootComponent->SetVisibility(bFlag, true); //true: propagate to children
        }
    }    
}

void AUIPlayerWidgetActor::SetSelectedIndex(int index){
    if(UPlayerWidget *widget = GetPlayerWidget()){
        widget->SetSelectedIndex(index);
    }
}

void AUIPlayerWidgetActor::SwitchToNextMode(){
    if(UPlayerWidget *widget = GetPlayerWidget()){
        widget->SwitchToNextMode();
    }
}

    
void AUIPlayerWidgetActor::SetSelectedIndexByState(ERobotState state){
    if(UPlayerWidget *widget = GetPlayerWidget()){
        widget->SetSelectedIndexByState(state);
    }
}


UPlayerWidget *AUIPlayerWidgetActor::GetPlayerWidget(){
    return GetWidget<UPlayerWidget>();
}



//fix
void AUIPlayerWidgetActor::SetModeZeroInit(){
    if(!wasInited){
        if(UPlayerWidget *widget = GetPlayerWidget()){
            wasInited = true;
            widget->SetSelectedIndex(0);
        }
    }
}





void AUIPlayerWidgetActor::ShowControls(EPlayerUiEvent event){
    if(UPlayerWidget *widget = GetPlayerWidget()){
        widget->ShowControls(event);
    }
}