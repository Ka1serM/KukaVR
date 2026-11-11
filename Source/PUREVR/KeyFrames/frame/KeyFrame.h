#pragma once

#include "CoreMinimal.h"
#include "PUREVR/RobotGrabbable/GrabbableActor.h"
#include "EKeyFrameState.h"
#include "PUREVR/KeyFrames/frame/scalarInterpolator/ScalarInterpolator.h"
#include "PUREVR/RobotHand/ERobotHandCommand.h"

#include "ENumberState.h"

#include "PUREVR/Ui/UINumberWidgetActor.h"

#include "KeyFrame.generated.h"

UCLASS()
class PUREVR_API AKeyFrame : public AGrabbableActor {
    GENERATED_BODY()

public:
    // Spawn functions
    static AKeyFrame* SpawnKeyFrame(
        UWorld* World, 
        const FVector& Location
    );

    static AKeyFrame* SpawnKeyFrame(
        UWorld* World, 
        const FVector& Location,
        bool bVisible
    );

    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    virtual void SetVisible(bool bVisible) override;
    void SetKeyFrameState(EKeyFrameState stateIn);

    AKeyFrame();

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="KeyFrame|Material")
    UMaterialInterface* DefaultMaterial = nullptr;

    UPROPERTY(EditAnywhere, Category="KeyFrameCommand")
    ERobotHandCommand command = ERobotHandCommand::ENone;

    void SetNumber(int32 number);
    void SetNumberShouldBeVisible(bool flag);
    void ReEnableNumberVisible();

    void SetNumberVisible(bool flag);

    void DebugShowLocation(FColor color);

    

    bool IsGrabbedFlag();

protected:
    void NotifyRobotFollowHandleLocationOnReleased();

private:
    void CheckForPickupableInRange();
    void CheckForDropZoneInRange();
    float sphereCastRange = 120.0f;

    UFUNCTION()
    void CreateMaterial();

    void UpdateMaterialInterpolation(float scalar);
    void UpdateStateByDistance();
    UStaticMeshComponent* FindMesh();

    bool changedState = false;
    //bool isSetVisible = true;
    EKeyFrameState stateCurrent = EKeyFrameState::EDefault;
    ScalarInterpolator interpolator;
    UMaterialInstanceDynamic* materialInstance = nullptr;

    void SetRaycastChannel();

private:
    //text spawning
    UPROPERTY()
    AUINumberWidgetActor *numberWidgetActor = nullptr;

    ENumberState numberState = ENumberState::EDisabled;
    bool isGrabbed = false;


    void CreateNumberWidget();
    void UpdateNumberVisibility();

public:
    // Grab interface
    virtual void OnGrabbed_Implementation() override;
    virtual void OnReleased_Implementation() override;
};
