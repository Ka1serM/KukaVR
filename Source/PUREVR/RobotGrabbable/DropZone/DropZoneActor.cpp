#include "DropZoneActor.h"

#include "PUREVR/RobotGrabbable/PickupCompatible.h"
#include "DebugPlugin/DebugHelper.h"
#include "PUREVR/RobotActor/RobotActor.h"



#define RobotHandCast ECC_GameTraceChannel2 


ADropZoneActor::ADropZoneActor(): Super(){
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.bStartWithTickEnabled = true;
    SetActorTickEnabled(true);
}

void ADropZoneActor::PerformSphereCast(){
    //check for drop zone Pickup actor
    //external drop trigger!

    //colorized items?
    //only proper enum zone for drop
    FVector dir(0, 0, 200);
    AActor *found = MakeSpehereCastByChannel(traceRadius, RobotHandCast, dir);
    //DebugHelper::showScreenMessage("ADropZoneActor::SphereCast TRY", FColor::Yellow);
    if(found){
        if(APickupCompatible *casted = Cast<APickupCompatible>(found)){
            casted->NotifyRobotHandForDrop();
            //DebugHelper::showScreenMessage("ADropZoneActor::SphereCast Success, NOTIFY DROP", FColor::Orange);
            //hier item konsumieren!
            AddTask(casted);

            
        }
    }
}

UStaticMeshComponent *ADropZoneActor::FindMesh(){
    //find mesh for material here
    if(USceneComponent *scene = FindSceneComponentByName("ZoneMesh")){
        if(UStaticMeshComponent *casted = Cast<UStaticMeshComponent>(scene)){
            return casted;
        }
    }
    return nullptr; 
}

void ADropZoneActor::BeginPlay(){
    Super::BeginPlay();

    //find mesh
    FindMeshAndCreateMaterial();

    //apply material for drop zone

}

void ADropZoneActor::FindMeshAndCreateMaterial(){

    //MID->SetScalarParameterValue("MyParam", 1.0f);
    UMaterial* Mat = LoadObject<UMaterial>(
        nullptr,
        TEXT("/Game/Robot/dropZone/dropZoneMaterial.dropZoneMaterial")
    );
    DebugHelper::logMessage("ADropZoneActor::FoundMaterial");
    if(Mat){
        UStaticMeshComponent *mesh = FindMesh();
        if(mesh){
            DebugHelper::logMessage("ADropZoneActor::FoundMesh");

            materialInstance = UMaterialInstanceDynamic::Create(Mat, this);
            if(materialInstance){
                mesh->SetMaterial(0, materialInstance);
                DebugHelper::logMessage("ADropZoneActor::CreatedMaterial");
            }
        }
    }
}



void ADropZoneActor::Tick(float deltatime){
    Super::Tick(deltatime);

    //DebugHelper::showScreenMessage("ADropZoneActor::Tick");
    UpdateSpehereCastByDistance();
    TickDropTasks(deltatime);

    //DebugHelper::logMessage("ADropZoneActor::Tick!");
    //DebugHelper::showScreenMessage("ADropZoneActor::Tick!", true);
}

void ADropZoneActor::UpdateSpehereCastByDistance(){
    if(ARobotActor *robot = ARobotActor::GetInstance()){
        FVector location;
        if(robot->HandHasItem(location)){
            if(FVector::Dist(location, GetActorLocation()) <= traceRadius){
                PerformSphereCast();
                //DebugHelper::showScreenMessage("ADropZoneActor::Distance Ok");
            }
        }
    }
}


void ADropZoneActor::AddTask(ABaseActor *actor){
    if(actor){
        dropTasks.SetNum(dropTasks.Num() + 1);
        DropZoneTask &task = dropTasks.Last();
        task.Setup(
            actor, 
            GetActorLocation(),
            dropSpeed
        );
    }
}

void ADropZoneActor::TickDropTasks(float deltatime){
    int i = 0;
    while (i < dropTasks.Num()){
        if(i >= 0){
            DropZoneTask &current = dropTasks[i];
            if(current.Tick(deltatime)){
                //swap popback, i--
                dropTasks[i] = dropTasks.Last();
                dropTasks.Pop();
                if(i >= dropTasks.Num()){
                    return;
                }
                i--;
            }
        }
        i++;
    }

}



void ADropZoneActor::UpdateZoneColor(float deltatime){

}




void ADropZoneActor::EndPlay(const EEndPlayReason::Type EndPlayReason){
    // clean up
    dropTasks.Empty();

    Super::EndPlay(EndPlayReason);
}