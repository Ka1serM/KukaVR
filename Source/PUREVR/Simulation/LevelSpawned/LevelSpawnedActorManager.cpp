#include "LevelSpawnedActorManager.h"



void LevelSpawnedActorManager::Clear(){
    for (int i = 0; i < SpawnedActors.Num(); i++){
        if(ABaseActor *a = SpawnedActors[i]){
            a->SetVisible(false);

            //UNKLAR, muss ggf EndPlay in Box Override und DropZone
            //das alle referenzen gecleared werden!
            a->Destroy(); //clean up on endplay!
        }
    }
    SpawnedActors.Empty();
}



void LevelSpawnedActorManager::StartLevel(
    ULevelDataAsset *asset, 
    FVector robotLocation,
    UWorld *world
){
    Clear();

    if(asset && world){
        if(asset->HasLevelSpawnedActors()){
            Spawn(
                asset->LevelSpawnedActors,
                robotLocation,
                world
            );
        }
    }
}



void LevelSpawnedActorManager::Spawn(
    TArray<FLevelSpawnedActor> &dataArray,
    FVector &robotLocation, 
    UWorld *world
){
    for (int i = 0; i < dataArray.Num(); i++){
        Spawn(
            dataArray[i],
            robotLocation,
            world
        );
    }
}


void LevelSpawnedActorManager::Spawn(
    FLevelSpawnedActor &data,
    FVector &robotLocation, 
    UWorld *world
){
    FVector worldLocation = robotLocation + data.relativeLocationToRobot;

    if (ABaseActor *actorSpawned = SpawnActor(data.actorClass, worldLocation, world)){
        SpawnedActors.Add(actorSpawned);
    }
}



ABaseActor *LevelSpawnedActorManager::SpawnActor(
    TSubclassOf<ABaseActor> actorClass,
    FVector &worldLocation, 
    UWorld *world
){
    if(world){
        FActorSpawnParameters Params;
        ABaseActor *spawned = world->SpawnActor<ABaseActor>(
            actorClass, FVector::ZeroVector, FRotator::ZeroRotator, Params
        );
        if(spawned){
            spawned->SetActorLocation(worldLocation);
            return spawned;
        }
    }
    return nullptr;
}