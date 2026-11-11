#include "AnimationManager.h"
#include "DebugPlugin/DebugHelper.h"

KeyFrameAnimation* AnimationManager::CreateAnimation(UWorld* World)
{
    if (!World)
        return nullptr;

    int32 NewIndex = Animations.Emplace(World);

    //DebugHelper::showScreenMessage("AnimationManager", "Created new animation");

    // Set as active if none
    if (ActiveIndex == INDEX_NONE)
        ActiveIndex = NewIndex;

    return &Animations[NewIndex];
}

void AnimationManager::RemoveAnimation(int32 Index)
{
    if (!Animations.IsValidIndex(Index))
        return;

    Animations.RemoveAt(Index);

    // Adjust ActiveIndex
    if (ActiveIndex == Index)
        ActiveIndex = INDEX_NONE;
    else if (ActiveIndex > Index)
        ActiveIndex--;

    //DebugHelper::showScreenMessage("AnimationManager", "Removed animation");
}

void AnimationManager::SetActiveAnimation(KeyFrameAnimation* Animation)
{
    if (!Animation)
        return;

    // Find the animation
    for (int32 i = 0; i < Animations.Num(); i++)
        if (&Animations[i] == Animation)
        {
            SetActiveAnimationByIndex(i);
            return;
        }
}

void AnimationManager::SetActiveAnimationByIndex(int32 Index)
{
    if (!Animations.IsValidIndex(Index))
        return;

    // Hide previous animation
    if (ActiveIndex != INDEX_NONE && Animations.IsValidIndex(ActiveIndex))
        Animations[ActiveIndex].SetVisible(false);

    ActiveIndex = Index;
    Animations[ActiveIndex].Reset();
    Animations[ActiveIndex].SetVisible(true);
}

KeyFrameAnimation* AnimationManager::GetActiveAnimation()
{
    if (ActiveIndex != INDEX_NONE && Animations.IsValidIndex(ActiveIndex)){
        //DebugHelper::showScreenMessage("AnimationManager::Active AnimationIndex", ActiveIndex);
        return &Animations[ActiveIndex];
    }
        
        
    return nullptr;
}