#include "KeyFrameAnimation.h"
#include "PUREVR/KeyFrames/frame/KeyFrame.h"
#include "DebugPlugin/DebugHelper.h"
#include "Engine/World.h"

KeyFrameAnimation::KeyFrameAnimation(UWorld* InWorld)
    : World(InWorld)
{
}

KeyFrameAnimation::~KeyFrameAnimation()
{
    // KeyFrame actors are spawned in the world and will be destroyed automatically
    // when the world is unloaded. We only need to clear our references.
    KeyFrames.Empty();
}

void KeyFrameAnimation::AddFrameAtPosition(const FVector& Position)
{
    if(!CanSpawnKeyFrameAt(Position)){
        return;
    }

    if (!World) return;

    AKeyFrame* NewFrame = MakeKeyFrameWithNumberVisible(Position); //visible by default!
    if (NewFrame)
    {
        KeyFrames.Add(NewFrame);
        UpdateFrameNumbers();

        NewFrame->DebugShowLocation(FColor::Orange);
        DebugHelper::showLineBetween(World, Position, Position + FVector(0, 0, 100000), FColor::Orange, 1.0f);
        DebugHelper::logMessage("KeyFrame added to animation");

        DebugHelper::showScreenMessage("KeyFrame added to animation!", FColor::Yellow);
    }
}

AKeyFrame* KeyFrameAnimation::MakeKeyFrameWithNumberVisible(const FVector &Position){
    if(!World){
        return nullptr;
    }

    AKeyFrame* NewFrame = AKeyFrame::SpawnKeyFrame(World, Position, true); //visible by default!
    if (NewFrame)
    {   
        NewFrame->SetActorLocation(Position);
        NewFrame->SetVisible(true);
        NewFrame->SetNumberShouldBeVisible(true);
        return NewFrame;
    }
    return nullptr;
}


/*
AKeyFrame* KeyFrameAnimation::PlayerApiAddFrameAtPosition(const FVector &Position){

}*/



AKeyFrame* KeyFrameAnimation::PlayerApiAddFrameAtPosition(const FVector &Position, AKeyFrame *frameGrabbed){
    if(!CanSpawnKeyFrameAt(Position)){
        return frameGrabbed;
    }

    if(frameGrabbed){
        int32 Index = KeyFrames.Find(frameGrabbed);

        //new: correct ---> testing needed!
        AKeyFrame *NewFrame = MakeKeyFrameWithNumberVisible(Position);
        if (NewFrame)
        {
            if (Index != INDEX_NONE) //found, insert
            {
                // gefunden
                AKeyFrame *frame = KeyFrames[Index];
                if(Index + 1 < KeyFrames.Num()){
                    if (NewFrame)
                    {
                        KeyFrames.Insert(NewFrame, Index); //right after
                        NewFrame->DebugShowLocation(FColor::Orange);
                    }
                }else{
                    KeyFrames.Add(NewFrame);
                    NewFrame->DebugShowLocation(FColor::Cyan);
                }
                UpdateFrameNumbers();
                return NewFrame;
            }else{
                //append new frame if none found
                KeyFrames.Add(NewFrame);
                NewFrame->DebugShowLocation(FColor::Green);
                UpdateFrameNumbers();
                return NewFrame;
            }
        }
    }
    return frameGrabbed;
}

void KeyFrameAnimation::UpdateFrameNumbers(){
    int32 count = 0;
    for (AKeyFrame *frame : KeyFrames)
    {
        count++; //start at 1
        if (frame)
        {
            frame->SetNumber(count);
            //frame->SetNumberShouldBeVisible(true); //hier nötig. Robot drop muss wieder anmachen
        }
    }
}

bool KeyFrameAnimation::CanSpawnKeyFrameAt(const FVector &pos){
    if(KeyFrames.Num() > 0){
        float minDistance = 50.0f;
        for (int i = 0; i < KeyFrames.Num(); i++){
            if(AKeyFrame *current = KeyFrames[i]){
                if(!current->IsGrabbedFlag()){
                    if(FVector::Dist(pos, current->GetActorLocation()) <= minDistance){
                        return false;
                    }
                }
            }
        }
    }
    
    return true;
}

AKeyFrame *KeyFrameAnimation::ClearAnimation(){
    if(KeyFrames.Num() > 0){
        AKeyFrame *first = KeyFrames[0];
        CurrentFrameIndex = 0;
        for (int i = 1; i < KeyFrames.Num(); i++){
            if(AKeyFrame *current = KeyFrames[i]){
                current->SetVisible(false);
                current->Destroy();
            }
        }
        KeyFrames.Empty();
        KeyFrames.Add(first);
        return first;
    }
    return nullptr;
}

AKeyFrame* KeyFrameAnimation::GetCurrentTargetFrame() const
{
    if (KeyFrames.IsValidIndex(CurrentFrameIndex))
        return KeyFrames[CurrentFrameIndex];
    
    return nullptr;
}

void KeyFrameAnimation::AdvanceSegment()
{
    if (KeyFrames.Num() == 0)
        return;

    CurrentFrameIndex++;

    if (CurrentFrameIndex >= KeyFrames.Num())
    {
        if (bLoopEnabled && KeyFrames.Num() > 1)
            CurrentFrameIndex = 0;
        else
            CurrentFrameIndex = KeyFrames.Num() - 1; // clamp to last
    }
}

void KeyFrameAnimation::Reset()
{
    CurrentFrameIndex = 0;
}

void KeyFrameAnimation::SetVisible(bool bVisibleIn)
{
    bVisible = bVisibleIn;
    
    for (AKeyFrame* Frame : KeyFrames){
        if (Frame){
            Frame->SetVisible(bVisible);
            Frame->SetNumberShouldBeVisible(true);
        }
    }
        
}



TArray<FVector> KeyFrameAnimation::CopyAnimation(){
    TArray<FVector> outAnim;
    for (AKeyFrame *Frame : KeyFrames)
    {
        if (Frame){
            outAnim.Add(Frame->GetActorLocation());
        }
    }
    return outAnim;
}