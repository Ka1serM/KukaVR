#pragma once


#include "CoreMinimal.h"
#include "PUREVR/KeyFrames/frame/scalarInterpolator/ScalarInterpolator.h"
#include "PUREVR/BaseActor/BaseActor.h"

#include "SafeZoneActor.generated.h"

UCLASS()
class PUREVR_API ASafeZoneActor : public ABaseActor {
    GENERATED_BODY()

public:
    ASafeZoneActor();

    virtual void BeginPlay() override;
    virtual void Tick(float deltatime) override;

    void SetVisibleIfInSameDirection(
        AActor *keyframe,
        AActor *robot,
        bool flag
    );

    virtual void SetVisible(bool flag) override;

    void SetVisibleImmidiate(bool flag);

    bool ScaleTo(int cmIn);

private:
    

    void UpdateVisibilty(float t);

    UStaticMeshComponent *FindMesh();

    void CreateMaterial();

    ScalarInterpolator visibilityInterpolator;
    bool savedVisibility = true;

    bool hiddenOnPlay = false;

    UPROPERTY()
    UMaterialInstanceDynamic *materialInstance = nullptr;


    //material special parameters for radius drawing
    void InitMaterialRayParametersSelf();
    void UpdateMaterialRayParameters(AActor *robot, AActor *keyframe);
};