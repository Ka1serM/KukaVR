#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "FTaskInfo.h"

#include "PUREVR/Simulation/LevelSpawned/FLevelSpawnedActor.h"

#include "LevelDataAsset.generated.h"

/**
 * Data asset that represents a level containing a list of tasks
 */
UCLASS(BlueprintType)
class PUREVR_API ULevelDataAsset : public UDataAsset
{
	GENERATED_BODY()

public:
	/** Enum of the level */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Level")
	ESimulationLevel LevelEnum;
	
	/** Name of the level */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Level")
	FText LevelName;

	/** Description of the level */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Level")
	FText LevelDescription;

	/** Array of tasks for this level */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Level")
	TArray<FTaskInfo> Tasks;

	/** If Level is Completed */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Level")
	bool bIsCompleted;
	
	/**
	 * Get the total number of tasks in this level
	 * @return Number of tasks in the level
	 */
	UFUNCTION(BlueprintPure, Category = "Level")
	int32 GetTaskCount() const;
	
	/**
	 * Check if the level has any tasks
	 * @return True if there are tasks, false otherwise
	 */
	UFUNCTION(BlueprintPure, Category = "Level")
	bool HasTasks() const;


	/// --- spawning actors on level load ---
	UFUNCTION(BlueprintPure, Category = "Level")
	bool HasLevelSpawnedActors() const;

	/// @brief array of actors to be spawned in this level
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Level")
	TArray<FLevelSpawnedActor> LevelSpawnedActors;
};