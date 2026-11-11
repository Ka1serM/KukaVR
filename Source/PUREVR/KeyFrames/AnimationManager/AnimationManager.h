#pragma once

#include "CoreMinimal.h"
#include "PUREVR/KeyFrames/KeyFrameAnimation.h"

class UWorld;

class AnimationManager
{
public:
    KeyFrameAnimation* CreateAnimation(UWorld* World);

    void RemoveAnimation(int32 Index);

    void SetActiveAnimation(KeyFrameAnimation* Animation);

    void SetActiveAnimationByIndex(int32 Index);

    KeyFrameAnimation* GetActiveAnimation();

    TArray<KeyFrameAnimation>& GetAnimations() { return Animations; }

    int32 GetAnimationCount() const { return Animations.Num(); }

private:
    TArray<KeyFrameAnimation> Animations;
    int32 ActiveIndex = INDEX_NONE;
};