#pragma once

#include "CoreMinimal.h"
#include "PUREVR/BaseActor/BaseActor.h"
#include "PUREVR/PCM/derived/ProceduralMeshComponentCustom.h"
#include "PUREVR/KeyFrames/KeyFrameAnimation.h"

#include "RobotLineActor.generated.h"

UCLASS()
class PUREVR_API ARobotLineActor : public ABaseActor {
    GENERATED_BODY()

public:
    static ARobotLineActor *MakeInstance(UWorld *World);
    ARobotLineActor();

    virtual void BeginPlay() override;
    virtual void Tick(float deltatime) override;

    void UpdateMesh(TArray<FVector> &anchors);
    void UpdateMesh(KeyFrameAnimation *animation);

    void ClearMesh();

    //sets location of the mesh marking, but not color
    void SetEndEffectorPosition(FVector &worldPos);

    //sets location of mesh colored marking and color based on distance
    void SetEndEffectorPosition(FVector &pos, float distanceTarget);

private:
    void CreateMaterial();

    UPROPERTY()
    UProceduralMeshComponentCustom *Mesh = nullptr;


};