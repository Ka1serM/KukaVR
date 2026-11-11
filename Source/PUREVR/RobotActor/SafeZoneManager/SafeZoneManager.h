#pragma once

#include "CoreMinimal.h"
#include "PUREVR/RobotActor/SafeZone/SafeZoneActor.h"
#include "PUREVR/BaseActor/BaseActor.h"

#include "SafeZoneManager.generated.h"

UCLASS()
class PUREVR_API ASafeZoneManager : public ABaseActor {
    GENERATED_BODY()


public:
    static ASafeZoneManager *MakeInstance(UWorld *world);

    ASafeZoneManager();

    virtual void BeginPlay() override;
    virtual void Tick(float deltatime) override;

    void UpdateVisibility(
        AActor *keyframe,
        AActor *robot,
        bool flag
    );

    bool IsInRange(AActor* actor);
    bool IsInRange(FVector &worldLocation);

    bool ScaleTo(int cmIn);

private:
    void FindChildSafeZonesOnBeginPlay();

    UPROPERTY()
    TArray<ASafeZoneActor *> safeZones;

    float SafeZoneBound();
};