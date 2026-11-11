#pragma once 

#include "CoreMinimal.h"

#include "BaseActor.generated.h"

/// @brief  ------ BASE ACTOR FOR SHARED FUNCTIONALITY: FIND COMPONENTS AND CHILD ACTORS! -------
/// MARCELL NICHT LÖSCHEN DAS IST GUT SO!!
UCLASS()
class PUREVR_API ABaseActor : public AActor {
    GENERATED_BODY()
protected:
    bool isSetVisible = true;

public:
    //doesnt have to 
    void EndPlay(const EEndPlayReason::Type EndPlayReason) override {
        // stuff.

        Super::EndPlay(EndPlayReason);
    }

    virtual void SetVisible(bool flag){
        SetActorHiddenInGame(!flag);
        isSetVisible = flag;
    }

    AActor *MakeSpehereCastByChannel(float radius, ECollisionChannel collisionChannel);
    AActor *MakeSpehereCastByChannel(
        float radius,
        ECollisionChannel collisionChannel,
        FVector &scaledDirection
    );

    USceneComponent *FindSceneComponentByName(FString ChildName);
    USceneComponent *FindSceneComponentByName(AActor *ParentActor, FString ChildName);
    AActor *FindChildActorByName(FString ChildName);
    AActor *FindChildActorByName(AActor *ParentActor, FString ChildName);

    ///Gets all Children of type T recursive across all of this and child actors.
    /// @brief T must be some sort of USceneComponent or derived!
    /// @tparam T 
    /// @param parent 
    /// @param collected 
    template <typename T>
    void GetAllRecursive(AActor *parent, TArray<T*> &collected){
        if(parent){
            TArray<USceneComponent*> components;
            parent->GetComponents<USceneComponent>(components);
            for (int i = 0; i < components.Num(); i++){
                //cast to T or ChildActor
                USceneComponent *current = components[i];
                if(current){
                    if (T *casted = Cast<T>(current)){
                        collected.Add(casted);
                    }

                    if(UChildActorComponent *childActorComponent = Cast<UChildActorComponent>(current)){
                        AActor *asActor = childActorComponent->GetChildActor();
                        if(asActor){
                            GetAllRecursive<T>(asActor, collected); //recursive call on same template type.
                        }
                    }
                }
            }
        }
    }
};