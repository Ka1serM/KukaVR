#include "FollowHandle.h"

#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "PUREVR/Simulation/SimulationActor.h"

AFollowHandle::AFollowHandle()
{
}

void AFollowHandle::BeginPlay()
{
    Super::BeginPlay();
}

void AFollowHandle::OnGrabbed_Implementation()
{
    if (ASimulationActor* SimActor = ASimulationActor::GetInstance())
    {
        SimActor->OnGameEvent(EGameEvent::EMoveHandle, this);
    }
}

void AFollowHandle::OnReleased_Implementation()
{
    NotifyRobotFollowHandleLocationOnReleased();
}