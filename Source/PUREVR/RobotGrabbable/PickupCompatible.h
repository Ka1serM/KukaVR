#pragma once 

#include "CoreMinimal.h"
#include "PUREVR/BaseActor/BaseActor.h"
#include "PickupCompatible.generated.h"

//sets up default robot-hand-grab channel on this actor
// -- ROBOT PICKUP BY HAND --
UCLASS()
class PUREVR_API APickupCompatible : public ABaseActor {
    GENERATED_BODY()

public:
    APickupCompatible();


    //add meshes to be ignored on begin play here.
    virtual void BeginPlay() override;
    void SetTraceChannel(UPrimitiveComponent *primitive);

    /// @brief tracks all children in pickup channel.
    /// @param name 
    void TrackAllPrimitvesContainingName(FString name);
    void TrackAllPrimitves();

    FVector Bound();

    void NotifyRobotHandForDrop();
    void UpdateRobotHandReference(AActor *actor);

    bool WasDropped();

    //clear / abort
    void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
    

private:
    //raw ptr, casted later
    AActor *robotHandActor = nullptr;

    bool wasDropped = false;
};