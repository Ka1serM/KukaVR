#include "PlayerWidget.h"
#include "PUREVR/RobotActor/RobotActor.h"
#include "DebugPlugin/DebugHelper.h"


void UPlayerWidget::SetSelectedIndex(int index){
    index = VerifyIndex(index);
    currentIndex = index;
    DebugHelper::showScreenMessage("UPlayerWidget::SHOW INDEX ", index);
    for (int i = 0; i < modes; i++)
    {
        UCheckBox *box = WidgetAsCheckBoxAtIndex(i);
        bool enable = i == index;
        if(box){
            box->SetIsChecked(enable);
        }
    }
}

void UPlayerWidget::SwitchToNextMode(){
    SetSelectedIndex(currentIndex + 1);
}

int UPlayerWidget::VerifyIndex(int index){
    int modded = std::max(index, 0);
    modded = modded % modes;
    return modded;
}




UCheckBox *UPlayerWidget::WidgetAsCheckBoxAtIndex(int index){
    if(UWidget *found = WidgetAtIndex(index)){
        return Cast<UCheckBox>(found);
    }
    return nullptr;
}

UWidget *UPlayerWidget::WidgetAtIndex(int index){
    switch(index){
        case 0: return GetWidgetRobotIdleMode();
        case 1: return GetWidgetRobotAnimationPlayMode();
        case 2: return GetWidgetRobotAnimationEditMode();
        case 3: return GetWidgetRobotFollowMode();
    }
       
    
    return nullptr;
}



void UPlayerWidget::SetSelectedIndexByState(ERobotState state){
    ShowControlsByRobotState(state);
    switch (state)
    {
        case ERobotState::StateIdle:
            SetSelectedIndex(0);
            return;
        case ERobotState::StatePlayingAnimation:
            SetSelectedIndex(1);
            return;
        case ERobotState::StateEditAnimation: 
            SetSelectedIndex(2);
            return;
        case ERobotState::StateFollow: 
            SetSelectedIndex(3);
            return;
    }

}



void UPlayerWidget::ShowControls(EPlayerUiEvent event){

    if(event == EPlayerUiEvent::ENone){
        ShowWidget(GetWidgetControlQ(), false);
        ShowWidget(GetWidgetControlR(), true);
        ShowWidget(GetWidgetControlE(), true);
        //ShowWidget(GetWidgetControlC(), false);
        ShowWidget(GetWidgetControlMouse(), false);
    }
    if(event == EPlayerUiEvent::EPickedUpFrame){
        ShowWidget(GetWidgetControlQ(), true);
        ShowWidget(GetWidgetControlR(), true);
        ShowWidget(GetWidgetControlE(), false);
        //ShowWidget(GetWidgetControlC(), false);
        ShowWidget(GetWidgetControlMouse(), true);
    }
    if(event == EPlayerUiEvent::EDroppedFrame){
        ShowWidget(GetWidgetControlQ(), false);
        ShowWidget(GetWidgetControlR(), true);
        ShowWidget(GetWidgetControlE(), true);
        //ShowWidget(GetWidgetControlC(), false);
        ShowWidget(GetWidgetControlMouse(), false);
    }
}


void UPlayerWidget::ShowControlsByRobotState(ERobotState event){
    ShowWidget(GetWidgetControlC(), false);
    if(event == ERobotState::StateEditAnimation){
        ShowWidget(GetWidgetControlC(), true);
    }
}


void UPlayerWidget::ShowWidget(UWidget *widget, bool flag){
    if(widget){
        if(flag){
            widget->SetVisibility(ESlateVisibility::Visible);
        }else{
            widget->SetVisibility(ESlateVisibility::Collapsed);
        }
    }
}


