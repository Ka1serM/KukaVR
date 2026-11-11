#include "RobotLineActor.h"
#include "DebugPlugin/DebugHelper.h"


ARobotLineActor *ARobotLineActor::MakeInstance(UWorld *World){
    if(World){

        // Load the blueprint class
        UClass* bp = ARobotLineActor::StaticClass();
        if (!bp)
        {
            return nullptr;
        }

        // Spawn the actor
        FVector Location;
        FActorSpawnParameters SpawnParams;
        SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

        ARobotLineActor *spawned = World->SpawnActor<ARobotLineActor>(
            bp,
            Location,
            FRotator::ZeroRotator,
            SpawnParams
        );
        return spawned;
    }
    return nullptr;
}




// Sets default values
ARobotLineActor::ARobotLineActor()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

    RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("USceneRoot"));

    // Create the ProceduralMeshComponent
    Mesh = CreateDefaultSubobject<UProceduralMeshComponentCustom>(TEXT("GeneratedMesh"));
    Mesh->SetupAttachment(RootComponent);
   
}



void ARobotLineActor::BeginPlay(){
    Super::BeginPlay();
    CreateMaterial();


}


void ARobotLineActor::CreateMaterial(){

    //MID->SetScalarParameterValue("MyParam", 1.0f);

    ///Users/walterk/Documents/Uni/Semester5_WS25/pure3d/gitProject/PUREVR/Content/Robot/KeyFrame/red.uasset
    UMaterial *Mat = LoadObject<UMaterial>(
        nullptr,
        TEXT("/Game/Robot/line/lineMaterial.lineMaterial")
        // TEXT("/Game/Robot/safeZone/safeZoneMaterial.safeZoneMaterial")
    );
    if(Mat){
        if(Mesh){
            UMaterialInstanceDynamic *materialInstance = UMaterialInstanceDynamic::Create(Mat, this);
            Mesh->ApplyMaterial(materialInstance);
            DebugHelper::logMessage("ARobotLineActor::CreatedMaterial");
        }
    }
}


void ARobotLineActor::Tick(float deltatime){
    Super::Tick(deltatime);


}

void ARobotLineActor::SetEndEffectorPosition(FVector &pos){
    if(Mesh){
        //pos = GetActorTransform().InverseTransformPosition(pos);
        Mesh->SetEndEffectorPosition(pos);
    }
}


void ARobotLineActor::SetEndEffectorPosition(FVector &pos, float distanceTarget){
    if(Mesh){
        Mesh->SetEndEffectorPosition(pos, distanceTarget);
    }
}
    


void ARobotLineActor::UpdateMesh(TArray<FVector> &anchors){
    if(Mesh){
        for(FVector &current : anchors){
            current = GetActorTransform().InverseTransformPosition(current);
        }

        Mesh->UpdateMesh(anchors);
    }
}
    
void ARobotLineActor::UpdateMesh(KeyFrameAnimation *animation){
    if(animation){
        TArray<FVector> animFrames = animation->CopyAnimation();
        UpdateMesh(animFrames);
    }
}
   
void ARobotLineActor::ClearMesh(){
    if(Mesh){
        Mesh->ClearMesh();
    }
}
