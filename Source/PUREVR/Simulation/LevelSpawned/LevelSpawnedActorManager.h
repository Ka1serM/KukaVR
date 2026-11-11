#pragma once

#include "CoreMinimal.h"
#include "PUREVR/Simulation/LevelDataAsset.h"
#include "PUREVR/BaseActor/BaseActor.h"
#include "PUREVR/Simulation/LevelSpawned/FLevelSpawnedActor.h"



//Will Spawn and despawn actors on level load / unload, reset
class PUREVR_API LevelSpawnedActorManager{
public:
    LevelSpawnedActorManager() {};
    ~LevelSpawnedActorManager() {
        //Clear();
    };

    void StartLevel(ULevelDataAsset *asset, FVector robotLocation, UWorld *world);

protected:
    void Clear();
    TArray<ABaseActor *> SpawnedActors;

    
    
    
    void Spawn(
        TArray<FLevelSpawnedActor> &dataArray,
        FVector &robotLocation,
        UWorld *world
    );

    //added to spawned actors on sucess
    void Spawn(
        FLevelSpawnedActor &data,
        FVector &robotLocation,
        UWorld *world
    );

    ABaseActor *SpawnActor(
        TSubclassOf<ABaseActor> actorClass,
        FVector &worldLocation,
        UWorld *world
    );
};