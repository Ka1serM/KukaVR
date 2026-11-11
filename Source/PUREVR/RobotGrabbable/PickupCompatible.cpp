#include "PickupCompatible.h"
#include "PUREVR/RobotHand/RobotHandActor.h"
#include "DebugPlugin/DebugHelper.h"

APickupCompatible::APickupCompatible() :Super(){
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.bStartWithTickEnabled = true;
    SetActorTickEnabled(true);
}


void APickupCompatible::BeginPlay(){
    Super::BeginPlay();

    //erstmal so:
    TrackAllPrimitves();
}

#define RobotHandCast ECC_GameTraceChannel2 //in editor is second channel. 
void APickupCompatible::SetTraceChannel(UPrimitiveComponent *mesh){
    //RobotHandCast
    if(mesh){
        mesh->SetCollisionResponseToChannel(RobotHandCast, ECR_Block); // z.B. Blocken
    }
}

void APickupCompatible::TrackAllPrimitvesContainingName(FString name){
    TArray<UPrimitiveComponent *> collected;
    GetAllRecursive<UPrimitiveComponent>(this, collected);
    for (int i = 0; i < collected.Num(); i++){
        if(UPrimitiveComponent *current = collected[i]){
            if(current->GetName().Contains(name)){
                SetTraceChannel(current);
            }
        }
    }
}


void APickupCompatible::TrackAllPrimitves(){
    TArray<UPrimitiveComponent *> collected;
    GetAllRecursive<UPrimitiveComponent>(this, collected);
    for (int i = 0; i < collected.Num(); i++){
        if(UPrimitiveComponent *current = collected[i]){
            SetTraceChannel(current);
        }
    } 
}



FVector APickupCompatible::Bound(){

    USceneComponent *scene = FindSceneComponentByName("base");
    if(scene){
        if(UStaticMeshComponent *component = Cast<UStaticMeshComponent>(scene)){
            // Lokale Bounding Box
            FBox LocalBox = component->GetStaticMesh()->GetBoundingBox(); // immer im Mesh-Localspace

            // Optional: Zentrum und Extents
            FVector LocalCenter = LocalBox.GetCenter();
            FVector LocalExtents = LocalBox.GetExtent(); // halbe Größe in jeder Dimension
            return LocalExtents * 2.0f;
        
        }
    }
    
    return FVector(20, 20, 20);
}



void APickupCompatible::UpdateRobotHandReference(AActor *actor){
    if(actor){
        robotHandActor = actor;
    }
}

void APickupCompatible::NotifyRobotHandForDrop(){
    if(robotHandActor){
        if(ARobotHandActor *casted = Cast<ARobotHandActor>(robotHandActor)){
            DebugHelper::logMessage("Execute Drop!");
            casted->ExecuteCommand(ERobotHandCommand::EDropItem, true);
            wasDropped = true;
        }
    }
}

void APickupCompatible::EndPlay(const EEndPlayReason::Type EndPlayReason){
    NotifyRobotHandForDrop();
    robotHandActor = nullptr;
    Super::EndPlay(EndPlayReason);
}

bool APickupCompatible::WasDropped(){
    return wasDropped;
}