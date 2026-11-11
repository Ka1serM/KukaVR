#pragma once

#include "CoreMinimal.h"
#include "PUREVR/KeyFrames/interpolator/Interpolator.h"
#include "PUREVR/BaseActor/BaseActor.h"

class PUREVR_API DropZoneTask {

public:
    DropZoneTask();
    ~DropZoneTask();

    DropZoneTask(const DropZoneTask &other);
    DropZoneTask &operator=(const DropZoneTask &other);

    void Setup(ABaseActor *moveActor, FVector targetLocation, float speed);
    bool Tick(float deltatime);

private:
    LinearInterpolator interpolator;

    ABaseActor *attachedActor = nullptr;
};