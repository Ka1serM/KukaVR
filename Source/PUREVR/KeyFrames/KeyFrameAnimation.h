#pragma once

#include "CoreMinimal.h"

class AKeyFrame;
class UWorld;

class KeyFrameAnimation
{
public:
    KeyFrameAnimation(UWorld* InWorld);
    ~KeyFrameAnimation();

    void AddFrameAtPosition(const FVector& Position);

    //returns the newly added frame to be hold by player!!
    //new frame appended right after frame grabbed.
    AKeyFrame* PlayerApiAddFrameAtPosition(const FVector &Position, AKeyFrame *frameGrabbed);

    /** Get the current target frame for interpolation */
    AKeyFrame* GetCurrentTargetFrame() const;

    /** Advance to the next frame */
    void AdvanceSegment();

    void Reset();

    void SetLooping(bool bLoop) { bLoopEnabled = bLoop; }
    bool IsLooping() const { return bLoopEnabled; }

    void SetVisible(bool bVisible);
    bool IsVisible() const { return bVisible; }

    int32 GetFrameCount() const { return KeyFrames.Num(); }
    int32 GetCurrentFrameIndex() const { return CurrentFrameIndex; }

    void UpdateFrameNumbers();

    /// @brief clears the animation but keeps the first frame!
    /// @return 
    AKeyFrame *ClearAnimation();

    TArray<FVector> CopyAnimation();

private:
    void DebugShowLocation(FColor color);
    AKeyFrame *MakeKeyFrameWithNumberVisible(const FVector &Position);

    bool CanSpawnKeyFrameAt(const FVector &pos);

    TArray<AKeyFrame*> KeyFrames;
    UWorld* World = nullptr;

    int32 CurrentFrameIndex = 0;
    bool bLoopEnabled = false;
    bool bVisible = true;
};
