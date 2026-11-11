#include "SafeZoneManager.h"
#include "DebugPlugin/DebugHelper.h"


ASafeZoneManager *ASafeZoneManager::MakeInstance(UWorld *World){
    if(World){


        ///Content/Robot/safeZone/SafeZoneManagerBp.uasset
        FString BlueprintPath = FString::Printf(
            TEXT("/Game/Robot/safeZone/SafeZoneManagerBp.SafeZoneManagerBp_C")
        );
        // Load the blueprint class
        UClass* bp = LoadObject<UClass>(nullptr, *BlueprintPath);
        if (!bp)
        {
            return nullptr;
        }

        // Spawn the actor
        FVector Location;
        FActorSpawnParameters SpawnParams;
        SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

        AActor *spawned = World->SpawnActor<AActor>(
            bp,
            Location,
            FRotator::ZeroRotator,
            SpawnParams
        );
        if(spawned){
            if (ASafeZoneManager *manager = Cast<ASafeZoneManager>(spawned)){
                return manager;
            }
        }
    }
    return nullptr;
}

ASafeZoneManager::ASafeZoneManager(){
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.bStartWithTickEnabled = true;
    SetActorTickEnabled(true);
}

void ASafeZoneManager::BeginPlay(){
    Super::BeginPlay();
    FindChildSafeZonesOnBeginPlay();
    for (int i = 0; i < safeZones.Num(); i++){
        if(ASafeZoneActor *zone = safeZones[i]){
            zone->SetVisibleImmidiate(false);
        }
    }

    DebugHelper::logMessage("ASafeZoneManager::FoundZones", safeZones.Num());
}

void ASafeZoneManager::Tick(float deltatime){
    Super::Tick(deltatime);

    //DebugHelper::showScreenMessage("Tick safezonemanager");
}

void ASafeZoneManager::UpdateVisibility(
    AActor *keyframe, 
    AActor *robot,
    bool flag
){
    //DebugHelper::showScreenMessage("UPDATE SAFE ZONE A");
    
    for (int i = 0; i < safeZones.Num(); i++){
        if(ASafeZoneActor *zone = safeZones[i]){

            bool bDynamicVisibility = true;
            if(bDynamicVisibility){
                zone->SetVisibleIfInSameDirection(keyframe, robot, flag);
            }else{
                zone->SetVisible(flag); //simple
            }

            //DebugHelper::showScreenMessage("UPDATE SAFE ZONE B");
            //zone->SetVisibleIfInSameDirection(keyframe, robot, flag);

            //DebugHelper::logMessage(
            //    FString::Printf(TEXT("ASafeZoneManager::updateVisibility %d / %d"), i, safeZones.Num() - 1)
            //);
            //zone->SetVisible(flag); //simple

            
        }
    }
    
}





void ASafeZoneManager::FindChildSafeZonesOnBeginPlay(){

    TArray<UChildActorComponent*> ChildComponents;
    GetComponents<UChildActorComponent>(ChildComponents);

    for (UChildActorComponent* Component : ChildComponents){
        if(Component){
            AActor *childActor = Component->GetChildActor();
            if(childActor){
                FString name = Component->GetName();
                if (name.Contains("SafeZone"))
                {
                    if(ASafeZoneActor *casted = Cast<ASafeZoneActor>(childActor)){
                        if(!safeZones.Contains(casted)){
                            safeZones.Add(casted);
                        }
                    }
                }
            }
        }
    }

}





bool ASafeZoneManager::IsInRange(AActor* actor){
    if(actor){
        FVector locationWorld = actor->GetActorLocation();
        return IsInRange(locationWorld);
    }
    return false;
}

bool ASafeZoneManager::IsInRange(FVector &worldLocation){
    
    float bound = SafeZoneBound();
    //FVector Origin;
    //GetActorBounds(true, bound, Extent); // returns 0,0,0 which is not true, child actors are not counted.

    //DebugHelper::showScreenMessage("ASafeZoneManager bound ", bound);

    FVector localLocation = worldLocation - GetActorLocation(); // AB = B - A
    if(localLocation.Size() < bound){
        return true;
    }

    return false;
}

float ASafeZoneManager::SafeZoneBound(){
    float bound = 0.0f;
    FVector location = GetActorLocation();
    for (int i = 0; i < safeZones.Num(); i++)
    {
        if(ASafeZoneActor *zoneCurrent = safeZones[i]){
            float dist = FVector::Dist(zoneCurrent->GetActorLocation(), location);
            if(dist > bound){
                bound = dist;
            }
        }
    }

    return bound;
}



//scale safe zone to some CM
bool ASafeZoneManager::ScaleTo(int cmIn){
    if(safeZones.Num() < 4){
        return false;
    }

    float halfSize = cmIn / 2.0f;

    //pull away from center
    FVector center = GetActorLocation();
    center.Z = 0.0f;
    for (int i = 0; i < safeZones.Num(); i++)
    {
        if(ASafeZoneActor *zone = safeZones[i]){
            FVector currentZoneLocation = zone->GetActorLocation();
            currentZoneLocation.Z = 0.0f;

            FVector dir = currentZoneLocation - center; // AB = B - A
            dir = dir.GetSafeNormal() * halfSize;

            FVector newLocation = center + dir;
            newLocation.Z += GetActorLocation().Z + halfSize; //move up 

            zone->SetActorLocation(newLocation);

            zone->ScaleTo(cmIn);
        }
    }
    return true;
}
