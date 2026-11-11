#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "PUREVR/Simulation/LevelDataAsset.h"  // We need this for LevelDataAsset
#include "TaskWidget.generated.h"

UCLASS()
class PUREVR_API UTaskWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	/**
	 * Load tasks from the given level data asset.
	 */
	UFUNCTION(BlueprintImplementableEvent, Category = "Tasks")
	void LoadTasks(ULevelDataAsset* LevelDataAsset);

	/**
	 * Set active task by index.
	 */
	UFUNCTION(BlueprintImplementableEvent, Category = "Tasks")
	void SetCompleted(int32 TaskIndex);
};