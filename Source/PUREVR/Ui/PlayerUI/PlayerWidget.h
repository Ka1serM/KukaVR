#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/CheckBox.h"
#include "PUREVR/RobotActor/ERobotState.h"
#include "PUREVR/Ui/PlayerUI/EPlayerUiEvent.h"

#include "PlayerWidget.generated.h"

UCLASS()
class PUREVR_API UPlayerWidget : public UUserWidget {
    GENERATED_BODY()

public:

    //widget functions to get the check boxes, check selected Mode
    UFUNCTION(BlueprintImplementableEvent, Category = "WidgetSetting")
    UWidget *GetWidgetRobotIdleMode();

    UFUNCTION(BlueprintImplementableEvent, Category = "WidgetSetting")
    UWidget *GetWidgetRobotFollowMode();

    UFUNCTION(BlueprintImplementableEvent, Category = "WidgetSetting")
    UWidget *GetWidgetRobotAnimationEditMode();

    UFUNCTION(BlueprintImplementableEvent, Category = "WidgetSetting")
    UWidget *GetWidgetRobotAnimationPlayMode();

    //show hide stuff based on states of robot and player
    void ShowControls(EPlayerUiEvent event);
    void ShowControlsByRobotState(ERobotState event);


    //widget functions to get the check boxes, check selected Mode
    UFUNCTION(BlueprintImplementableEvent, Category = "WidgetSetting")
    UWidget *GetWidgetControlQ();

    UFUNCTION(BlueprintImplementableEvent, Category = "WidgetSetting")
    UWidget *GetWidgetControlR();

    UFUNCTION(BlueprintImplementableEvent, Category = "WidgetSetting")
    UWidget *GetWidgetControlE();

    UFUNCTION(BlueprintImplementableEvent, Category = "WidgetSetting")
    UWidget *GetWidgetControlC();

    UFUNCTION(BlueprintImplementableEvent, Category = "WidgetSetting")
    UWidget *GetWidgetControlMouse();



    //modes = 4
private:
    int modes = 4;
    int currentIndex = 0;
    int VerifyIndex(int index);

    void ShowWidget(UWidget *widget, bool flag);

public:
    

    //api for switch mode
    void SetSelectedIndex(int index);
    void SetSelectedIndexByState(ERobotState state);
    void SwitchToNextMode();

private:
    //helper for ERobotState 
    UCheckBox *WidgetAsCheckBoxAtIndex(int index);
    UWidget *WidgetAtIndex(int index);
};