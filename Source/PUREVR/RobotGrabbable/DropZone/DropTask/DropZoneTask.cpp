#include "DropZoneTask.h"

DropZoneTask::DropZoneTask(){

}

DropZoneTask::~DropZoneTask(){

}

DropZoneTask::DropZoneTask(const DropZoneTask &other){
    if(this != &other){
        *this = other;
    }
}
DropZoneTask &DropZoneTask::operator=(const DropZoneTask &other){
    if(this != &other){
        interpolator = other.interpolator;
        attachedActor = other.attachedActor;
    }
    return *this;
}

void DropZoneTask::Setup(ABaseActor *moveActor, FVector targetLocation, float speed){
    if(moveActor){
        attachedActor = moveActor;

        FVector start = moveActor->GetActorLocation();
        interpolator.BeginInterpolation(
            start,
            targetLocation,
            speed
        );
    }
}

bool DropZoneTask::Tick(float deltatime){
    bool completeFlag = false;
    FVector position = interpolator.Tick(deltatime, completeFlag);
    if(attachedActor){
        attachedActor->SetActorLocation(position);
        if(completeFlag){
            attachedActor->SetVisible(false);
        }
        return completeFlag;
    }
    return true; //invalid task
}
