#include "NumberWidget.h"
#include "Components/TextBlock.h"

void UNumberWidget::SetNumber(int32 num){
    if(UWidget *widget = GetText()){
        if(UTextBlock *casted = Cast<UTextBlock>(widget)){
            casted->SetText(FText::AsNumber(num));
        }
    }
}