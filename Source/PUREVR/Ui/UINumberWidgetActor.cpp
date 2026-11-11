#include "UINumberWidgetActor.h"
#include "NumberWidget.h"
#include "PUREVR/Simulation/SimulationActor.h"


AUINumberWidgetActor *AUINumberWidgetActor::MakeInstance(UWorld *world, AActor *attachTo){
    if(!world || !attachTo){
        return nullptr;
    }

    UClass* SpawnClass = AUINumberWidgetActor::StaticClass();
    FActorSpawnParameters SpawnParams;
    SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

    FVector Location(0, 0, 0);
    AUINumberWidgetActor *spawned = world->SpawnActor<AUINumberWidgetActor>(
        SpawnClass,
        Location,
        FRotator::ZeroRotator,
        SpawnParams
    );

    if(ASimulationActor *actor = ASimulationActor::GetInstance()){
        // Set pause menu to hidden by default
        spawned->SetWidgetClass(actor->NumberWidgetClass);
    }else{
        return nullptr;
    }

    if (!spawned)
    {
        UE_LOG(LogTemp, Error, TEXT("AUINumberWidgetActor::MakeINstance - Spawned actor is not a AUINumberWidgetActor"));
        return nullptr;
    }

    spawned->SetVisible(true);

    //attach
    spawned->AttachToComponent(attachTo->GetRootComponent(), FAttachmentTransformRules::SnapToTargetNotIncludingScale);
    spawned->SetActorRelativeLocation(FVector(0, 0, 50));

    //debug
    spawned->SetNumber(999);

    return spawned;
}

void AUINumberWidgetActor::BeginPlay(){
    Super::BeginPlay();


    

}


void AUINumberWidgetActor::SetNumber(int32 number){
    if(UNumberWidget *found = GetWidget<UNumberWidget>()){
        found->SetNumber(number);
    }
}

void AUINumberWidgetActor::SetVisible(bool bFlag){
    if(isSetVisible != bFlag){
        isSetVisible = bFlag;
        SetActorHiddenInGame(!bFlag);
        if(RootComponent){
            RootComponent->SetVisibility(bFlag, true);
        }
    }    
}