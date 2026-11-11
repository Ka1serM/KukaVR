#pragma once
#include "CoreMinimal.h"

/**
 * Stateful interpolator with constant speed.
 * Fully dynamic - recalculates based on current distance each tick.
 * Perfect for live editing where targets can move.
 */
class Interpolator
{
public:
    virtual ~Interpolator() = default;

    /**
     * Begin interpolation from Start to End at given Speed
     */
    void BeginInterpolation(const FVector& Start, const FVector& End, float Speed)
    {
        StartPos = Start;
        EndPos = End;
        MovementSpeed = Speed;
        CurrentAlpha = 0.f;
    }

    /**
     * Tick the interpolation with dynamic target updates.
     * Always moves at constant Speed toward EndPos.
     */
    FVector Tick(float DeltaTime, bool& OutCompleted)
    {
        FVector CurrentPos = Evaluate(CurrentAlpha);
        FVector Dir = EndPos - CurrentPos;
        float Dist = Dir.Size();

        if (Dist > KINDA_SMALL_NUMBER)
        {
            FVector Step = Dir.GetSafeNormal() * MovementSpeed * DeltaTime;
            if (Step.Size() > Dist) Step = Dir;
            CurrentPos += Step;
        }

        float TotalDist = FVector::Dist(StartPos, EndPos);
        CurrentAlpha = (TotalDist > 0.f) ? FVector::Dist(StartPos, CurrentPos) / TotalDist : 1.f;

        OutCompleted = Dist < 0.1f;
        return CurrentPos;
    }

    FVector Tick(float DeltaTime)
    {
        bool OutCompleted = false;
        FVector returnVal = Tick(DeltaTime,OutCompleted);
        return returnVal;
    }

    /**
     * Update the end position dynamically (for live editing).
     * Just update the target - next tick will adjust automatically.
     */
    void UpdateEndPosition(const FVector& NewEnd)
    {
        EndPos = NewEnd;
    }

    /**
     * Update the start position (useful when target moves significantly)
     */
    void UpdateStartPosition(const FVector& NewStart)
    {
        StartPos = NewStart;
    }

    /**
     * Get current interpolated position without ticking
     */
    FVector GetCurrentPosition() const
    {
        return Evaluate(FMath::Clamp(CurrentAlpha, 0.f, 1.f));
    }

    bool IsComplete() const { return CurrentAlpha >= 1.f; }
    float GetProgress() const { return CurrentAlpha; }

protected:
    /**
     * Override this to provide different easing curves
     */
    virtual FVector Evaluate(float Alpha) const
    {
        return FMath::Lerp(StartPos, EndPos, Alpha);
    }

    FVector StartPos = FVector::ZeroVector;
    FVector EndPos = FVector::ZeroVector;
    float CurrentAlpha = 0.f;
    float MovementSpeed = 100.f;
};

/** Linear interpolation */
class LinearInterpolator : public Interpolator
{
protected:
    virtual FVector Evaluate(float Alpha) const override
    {
        return FMath::Lerp(StartPos, EndPos, Alpha);
    }
};

/** Cubic ease in/out interpolation */
class CubicEaseInOutInterpolator : public Interpolator
{
protected:
    virtual FVector Evaluate(float Alpha) const override
    {
        float EasedAlpha = (Alpha < 0.5f) 
            ? 4.f * Alpha * Alpha * Alpha
            : 1.f - FMath::Pow(-2.f * Alpha + 2.f, 3) / 2.f;
        
        return FMath::Lerp(StartPos, EndPos, EasedAlpha);
    }
};