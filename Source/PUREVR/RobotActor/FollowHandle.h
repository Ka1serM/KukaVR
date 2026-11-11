#pragma once

#include "CoreMinimal.h"
#include "PUREVR/KeyFrames/frame/KeyFrame.h"
#include "FollowHandle.generated.h"

/**
 * Specialized KeyFrame for the robot's follow mode - uses a different mesh for visualization
 */
UCLASS()
class PUREVR_API AFollowHandle : public AKeyFrame
{
    GENERATED_BODY()

public:
    AFollowHandle();

    virtual void BeginPlay() override;

protected:
    virtual void OnGrabbed_Implementation() override;
    virtual void OnReleased_Implementation() override;
};