#pragma once

#include "CoreMinimal.h"
#include "EGameEvent.h"
#include "Engine/DataAsset.h"
#include "FTaskInfo.generated.h"

class ASimulationActor; // Forward declaration

USTRUCT(BlueprintType)
struct FTaskInfo
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Task")
	FText TaskName;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Task")
	FText TaskDescription;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Task")
	EGameEvent TaskEvent = EGameEvent::ENone;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Task")
	bool bIsCompleted = false;
};