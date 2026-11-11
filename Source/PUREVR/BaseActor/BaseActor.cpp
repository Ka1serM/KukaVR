#include "BaseActor.h"
#include "DebugPlugin/DebugHelper.h"


USceneComponent* ABaseActor::FindSceneComponentByName(FString ChildName){
    return FindSceneComponentByName(this, ChildName);
}

USceneComponent* ABaseActor::FindSceneComponentByName(AActor* ParentActor, FString ChildName){
    if (!ParentActor){
        return nullptr;
    }

    TArray<USceneComponent*> ChildComponents;
    ParentActor->GetComponents<USceneComponent>(ChildComponents);
    for (USceneComponent* Component : ChildComponents){
        if(Component){
            if(Component->GetName() == ChildName){
                return Component;
            }
        }
    }

    // 2. Rekursiv auf alle Child-Actors gehen
    TArray<AActor*> AttachedActors;
    ParentActor->GetAttachedActors(AttachedActors);
    for (AActor* Child : AttachedActors){
        if (Child){
            if (USceneComponent* Found = FindSceneComponentByName(Child, ChildName)){
                return Found;
            }
        }
    }
    return nullptr;

}




AActor* ABaseActor::FindChildActorByName(FString ChildName){
    return FindChildActorByName(this, ChildName);
}

AActor* ABaseActor::FindChildActorByName(AActor* ParentActor, FString ChildName){
    if (!ParentActor){
        return nullptr;
    }

    TArray<UChildActorComponent*> ChildComponents;
    ParentActor->GetComponents<UChildActorComponent>(ChildComponents);

    for (UChildActorComponent* Component : ChildComponents){
        if(Component){
            AActor *childActor = Component->GetChildActor();
            if(childActor){
                if(Component->GetName() == ChildName){
                    return childActor;
                }
                if(AActor *casted = Cast<AActor>(childActor)){
                    if(AActor *found = FindChildActorByName(casted, ChildName)){
                        return found;
                    }
                }
            }
        }
    }

    return nullptr;
}





AActor *ABaseActor::MakeSpehereCastByChannel(float radius, ECollisionChannel collisionChannel){
    FVector dir(0, 0, 100);
    return MakeSpehereCastByChannel(radius, collisionChannel, dir);
}

AActor *ABaseActor::MakeSpehereCastByChannel(
    float radius, 
    ECollisionChannel collisionChannel,
    FVector &scaledDirection
){
    radius = std::max(std::abs(radius), 1.0f);

    FHitResult HitResult;
    FVector Start = GetActorLocation();
    FVector End = Start + scaledDirection; 
    
    FCollisionShape Sphere = FCollisionShape::MakeSphere(radius);

    // Query-Parameter für Actors, die ignoriert werden sollen
    FCollisionQueryParams QueryParams;
    QueryParams.AddIgnoredActor(this); // optional, dich selbst ignorieren

    bool bHit = GetWorld()->SweepSingleByChannel(
        HitResult,
        Start,
        End,
        FQuat::Identity,
        collisionChannel, //pickupable components are tracked in this channel.
        Sphere,
        QueryParams
    );

    if (bHit)
    {
        return HitResult.GetActor();
    }

    return nullptr;

}
