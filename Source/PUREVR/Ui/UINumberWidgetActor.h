#pragma once

#include "UIActor.h"

#include "UINumberWidgetActor.generated.h"

UCLASS()
class PUREVR_API AUINumberWidgetActor : public AUIActor {
    GENERATED_BODY()

public:
    static AUINumberWidgetActor *MakeInstance(UWorld *world, AActor *attachTo);

    virtual void BeginPlay() override;

    void SetNumber(int32 number);

    void SetVisible(bool bFlag);

private:
    bool isSetVisible = true;
};