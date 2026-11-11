#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "NumberWidget.generated.h"

UCLASS()
class PUREVR_API UNumberWidget : public UUserWidget
{
	GENERATED_BODY()

public:
    UFUNCTION(BlueprintImplementableEvent, Category = "NumberSetting")
    UWidget *GetText();

    void SetNumber(int32 num);
};