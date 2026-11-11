#pragma once

#include "CoreMinimal.h"
#include "PUREVR/BaseActor/BaseActor.h"
#include "PUREVR/RobotGrabbable/DropZone/DropTask/DropZoneTask.h"

#include "DropZoneActor.generated.h"

/// @brief will listen for pickup compatible items (drop by robot hand)
UCLASS()
class PUREVR_API ADropZoneActor : public ABaseActor
{
	GENERATED_BODY()

public:
	ADropZoneActor();

    virtual void BeginPlay() override;
    virtual void Tick(float deltatime) override;

    UPROPERTY(EditAnywhere, Category="DropZone")
    float dropSpeed = 70.0f;

    //clear tasks / abort
    void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

protected:
    void UpdateSpehereCastByDistance();
    void PerformSphereCast();

    

    void UpdateZoneColor(float deltatime);

    void FindMeshAndCreateMaterial();
    UStaticMeshComponent *FindMesh();

    float traceRadius = 100.0f;

    UPROPERTY()
    UMaterialInstanceDynamic *materialInstance = nullptr;

    //collected item tasks to tick interpolation
    TArray<DropZoneTask> dropTasks;
    void AddTask(ABaseActor *actor);
    void TickDropTasks(float deltatime);

    //create animation pairs to tick!
};
