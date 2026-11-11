#pragma once

#include "CoreMinimal.h"
#include "PUREVR/Ui/UIActor.h"
#include "PlayerWidget.h"
#include "PUREVR/RobotActor/ERobotState.h"
#include "PUREVR/Ui/PlayerUI/EPlayerUiEvent.h"

#include "UIPlayerWidgetActor.generated.h"

UCLASS()
class PUREVR_API AUIPlayerWidgetActor : public AUIActor {
    GENERATED_BODY()

public:
    static AUIPlayerWidgetActor *MakeInstance(UWorld *world, AActor *attachTo);
    static AUIPlayerWidgetActor *MakeInstance(UWorld *world, USceneComponent *attachTo);

    virtual void BeginPlay() override;
    virtual void Tick(float Deltatime) override;
    void SetVisible(bool bFlag);

    //simple widget api, update by robot
    void UpdateIndex();

    //widget api
    void SetSelectedIndex(int index);
    void SwitchToNextMode();
    void SetSelectedIndexByState(ERobotState state);
    void ShowControls(EPlayerUiEvent event);

    UPROPERTY()
    float widgetScaleSet = 0.05f;

private:
    void ApplyScale(float widgetScale);
    bool isSetVisible = true;

    UPlayerWidget *GetPlayerWidget();

    bool wasInited = false;
    void SetModeZeroInit();
};