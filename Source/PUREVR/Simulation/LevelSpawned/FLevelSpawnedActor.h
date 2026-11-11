#pragma once

#include "CoreMinimal.h"
#include "PUREVR/Simulation/ESimulationLevel.h"
#include "PUREVR/BaseActor/BaseActor.h"
#include "FLevelSpawnedActor.generated.h"

//class ASimulationActor; // Forward declaration

USTRUCT(BlueprintType)
struct FLevelSpawnedActor
{
	GENERATED_BODY()

    //Actor reference to be spawned
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ToSpawn")
	TSubclassOf<ABaseActor> actorClass;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ToSpawn")
    FVector relativeLocationToRobot;


};