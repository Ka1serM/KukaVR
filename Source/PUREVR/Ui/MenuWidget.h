#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "PUREVR/Simulation/LevelDataAsset.h"
#include "MenuWidget.generated.h"

UCLASS()
class PUREVR_API UMenuWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	/**
	 * Load levels from the given level array.
	 */
	UFUNCTION(BlueprintImplementableEvent, Category="Tasks")
	void LoadLevels(const TArray<ULevelDataAsset*>& LevelArray);
};