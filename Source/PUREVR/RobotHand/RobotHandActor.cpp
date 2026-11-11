#include "RobotHandActor.h"

#include "PUREVR/RobotGrabbable/PickupCompatible.h"

#include "PUREVR/Simulation/SimulationActor.h"

#include "DebugPlugin/DebugHelper.h"

ARobotHandActor *ARobotHandActor::MakeInstance(UWorld *World){
    if(World){

        ///Content/Robot/hand/RobotHandBP.uasset
        FString BlueprintPath = FString::Printf(
            TEXT("/Game/Robot/hand/RobotHandBP.RobotHandBP_C")
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
            if (ARobotHandActor *hand = Cast<ARobotHandActor>(spawned)){
                return hand;
            }
        }
    }
    return nullptr;
}



ARobotHandActor::ARobotHandActor() : Super() {
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.bStartWithTickEnabled = true;
    SetActorTickEnabled(true);
}

/// --- BEGIN PLAY SETUP SECTION ---

void ARobotHandActor::BeginPlay(){
    Super::BeginPlay();
    FindComponentsOnBeginPlay();
}

void ARobotHandActor::FindComponentsOnBeginPlay(){
    handleMain = FindSceneComponentByName("Hold");


    innerLeft = FindSceneComponentByName("innerLeft");
    innerRight = FindSceneComponentByName("innerRight");
    outerLeft = FindSceneComponentByName("outerLeft");
    outerRight = FindSceneComponentByName("outerRight");
    fingerLeft = FindSceneComponentByName("fingerLeft");
    fingerRight = FindSceneComponentByName("fingerRight");

    fingerRightPivot = FindSceneComponentByName("fingerRightPivot");
    fingerLeftPivot = FindSceneComponentByName("fingerLeftPivot");

    itemSocketPivot = FindSceneComponentByName("itemSocket");

    //works all.
    if(handleMain){
        DebugHelper::logMessage("ARobotHandActor::Found handleMain");
    }
    if(innerLeft){
        DebugHelper::logMessage("ARobotHandActor::Found innerLeft");
    }
    if(innerRight){
        DebugHelper::logMessage("ARobotHandActor::Found innerRight");
    }
    if(outerLeft){
        DebugHelper::logMessage("ARobotHandActor::Found outerLeft");
    }
    if(outerRight){
        DebugHelper::logMessage("ARobotHandActor::Found outerRight");
    }
    if(fingerLeft){
        DebugHelper::logMessage("ARobotHandActor::Found fingerLeft");
    }
    if(fingerRight){
        DebugHelper::logMessage("ARobotHandActor::Found fingerRight");
    }
    if(fingerRightPivot){
        DebugHelper::logMessage("ARobotHandActor::Found fingerRightPivot");
    }
    if(fingerLeftPivot){
        DebugHelper::logMessage("ARobotHandActor::Found fingerLeftPivot");
    }
    if(itemSocketPivot){
        DebugHelper::logMessage("ARobotHandActor::Found itemSocketPivot");
    }
}


/// --- BEGIN PLAY SETUP SECTION END---



/// --- TICK SECTION ---
void ARobotHandActor::Tick(float deltatime){
    Super::Tick(deltatime);
    TickUpdatePickuppedItem();
    
    
    
    TickDebugPickup(); //DEBUG DEFFAULT PICKUP ALL THE TIME !
    if(bDebugLoopGrabItemCommand){
        ExecuteCommand(ERobotHandCommand::EPickupItem); //default pickup, debug too. Auto pickup tick
    }
    

    TickGripOpenState(deltatime);
    TickCommand(deltatime);
}

void ARobotHandActor::UpdateTransform(FVector &location, FRotator &rotation){
    SetActorLocation(location);
    SetActorRotation(rotation);
}

/// --- TICK SECTION END ---



void ARobotHandActor::SetGripDistance(float distance){

    distance /= 2.0f; //half size g
    DebugHelper::showScreenMessage("RobotHand::Distance -> ", distance);

    float h = std::abs(innerLength);
    float g = std::abs(distance);

    if(h < 0.00000000001f){
        h = 0.00000000001f;
    }
    float frac = g / h;
    float angleRad = FMath::Asin(frac);
    float angleDeg = FMath::RadiansToDegrees(angleRad);
    angleDeg *= -1.0f;
    angleDeg += 37.0f; // SYMPTOM FIX! ----->
    // MESH IS ROTATED IN IT SELF; MUST BE FIXED WHERE MESH IS ALIGNED TO LOOK FORWARD!

    FRotator rLeft;
    FRotator rRight;

    rLeft.Pitch = 0.0f;
    rRight.Pitch = 0.0f;
    rLeft.Roll = 0.0f;
    rRight.Roll = 0.0f;

    //OK
    rLeft.Yaw = angleDeg;
    SetRotationLeftGrip(rLeft);

    rRight.Yaw = -angleDeg;
    SetRotationRightGrip(rRight);
    
}

void ARobotHandActor::SetRotationLeftGrip(FRotator rotation){
    if(innerLeft){
        innerLeft->SetRelativeRotation(rotation);
    }
    if(outerLeft){
        outerLeft->SetRelativeRotation(rotation);
    }

    //removeRotationFromFinger
    if(fingerLeftPivot){
        rotation.Yaw *= -1;
        fingerLeftPivot->SetRelativeRotation(rotation);
    }

    
}
void ARobotHandActor::SetRotationRightGrip(FRotator rotation){
    if(innerRight){
        innerRight->SetRelativeRotation(rotation);
    }
    if(outerRight){
        outerRight->SetRelativeRotation(rotation);
    }

    if(fingerRightPivot){
        rotation.Yaw *= -1;
        fingerRightPivot->SetRelativeRotation(rotation);
    }
}






void ARobotHandActor::TickGripOpenState(float deltatime){

    if(true){
        return;
    }

    //DEBUG ONLY!
    debugDistance += deltatime * 3.0f * debugSign; //5cm s
    if(debugDistance > 8.0f || debugDistance <= 0.0f){
        debugSign *= -1;
    }
    SetGripDistance(debugDistance);
}


void ARobotHandActor::TickDebugPickup(){
   
    // Only attempt pickup if not currently executing a command to prevent feedback loops
    // This helps prevent the hand from immediately grabbing newly created keyframes
    if (!commandExecuting && !lockedHand) {
        PickupItem();
        //DebugHelper::showScreenMessage("ARobotHandActor::Hand is unlocked", FColor::Green);
    }
    if(lockedHand){
        //DebugHelper::showScreenMessage("ARobotHandActor::Hand is LOCKED", FColor::Red);
    }
}

AActor *ARobotHandActor::MakeSpehereCast(){
    return MakeSpehereCast(SpehereCastRadius);
}

/// Sphere Cast
#define RobotHandCast ECC_GameTraceChannel2
AActor *ARobotHandActor::MakeSpehereCast(float radius){
    radius = std::max(std::abs(radius), 1.0f);

    FHitResult HitResult;
    FVector Start = GetActorLocation();
    FVector End = Start + GetActorForwardVector() * 10.0f; //10cm z.b.
    
    FCollisionShape Sphere = FCollisionShape::MakeSphere(radius);

    // Query-Parameter für Actors, die ignoriert werden sollen
    FCollisionQueryParams QueryParams;
    QueryParams.AddIgnoredActor(this); // optional, dich selbst ignorieren

    bool bHit = GetWorld()->SweepSingleByChannel(
        HitResult,
        Start,
        End,
        FQuat::Identity,
        RobotHandCast, //pickupable components are tracked in this channel.
        Sphere,
        QueryParams
    );

    if (bHit)
    {
        //DebugHelper::showScreenMessage("ARobotHandActor::HitSphere OK!");
        return HitResult.GetActor();
    }

    return nullptr;

}


APickupCompatible *ARobotHandActor::MakeSpehereCastPickupable(){
    AActor *actor = MakeSpehereCast(SpehereCastRadius);
    if(actor){
        if(APickupCompatible *casted = Cast<APickupCompatible>(actor)){
            if(casted != pickedUpItem){
                if(!casted->WasDropped()){
                    return casted;
                }
            }
        }
    }
    return nullptr;
}

void ARobotHandActor::TickUpdatePickuppedItem(){
    //block if command executing.
    //not tested
    if(commandExecuting){
        return;
    }

    if(pickedUpItem){
        //update transform
        FVector location = GetActorLocation();
        if(handleMain){
            location = CenterBetweenGrips();
        }

        pickedUpItem->SetActorLocation(location);
        pickedUpItem->SetActorRotation(GetActorRotation());
    }
}

bool ARobotHandActor::HasItem(){
    return pickedUpItem != nullptr;
}

void ARobotHandActor::PickupItem(){
    if(APickupCompatible *newItem = MakeSpehereCastPickupable()){
        DropItem();
        pickedUpItem = newItem;
        
        FVector size = newItem->Bound();
        newItem->UpdateRobotHandReference(this);
        // instant set for now, no interpolation!
        //DebugHelper::logMessage("ARobotHandActor::PickupLocalSize ", size.Size()); //34
        SetGripDistance(size.Size());
        NotifyEvent(EGameEvent::ERobotPickupBox);
    }

    //deprecated!
    /*
    AActor *newItem = MakeSpehereCast();
    if(newItem != pickedUpItem){
        

        if(APickupCompatible *casted = Cast<APickupCompatible>(newItem)){
            DropItem();
            pickedUpItem = newItem;
            
            FVector size = casted->Bound();
            casted->UpdateRobotHandReference(this);
            // instant set for now, no interpolation!
            DebugHelper::logMessage("ARobotHandActor::PickupLocalSize ", size.Size()); //34
            SetGripDistance(size.Size());
        }
    }*/
}

void ARobotHandActor::DropItem(){
    pickedUpItem = nullptr;

    //hier physics auf dem item? 
    //wie fällt es runter?
}


FVector ARobotHandActor::CenterBetweenGrips(){
    if(itemSocketPivot){
        return itemSocketPivot->GetComponentLocation();
    }
    return GetActorLocation();
}

float ARobotHandActor::CurrentGripDistance(){
    if(fingerRightPivot && fingerLeftPivot){
        FVector connect = fingerRightPivot->GetComponentLocation() - fingerLeftPivot->GetComponentLocation();
        return connect.Size();
    }
    return 20.0f;
}

void ARobotHandActor::ExecuteCommand(ERobotHandCommand commandIn){
    ExecuteCommand(commandIn, false);
}

void ARobotHandActor::ExecuteCommand(ERobotHandCommand commandIn, bool force){
    if(commandIn == ERobotHandCommand::ENone){
        return;
    }
    if(!force){
        if(!lockedHand){
            DebugHelper::showScreenMessage("ARobotHandActor::Hand is LOCKED", FColor::Red);
            return;
        }else{
            //DebugHelper::showScreenMessage("ARobotHandActor::Hand is unlocked", FColor::Green);
        }
    }
    

    //DebugHelper::showScreenMessage("ARobotHandActor::Try Command A", FColor::Green);
    if(!commandExecuting || force){
        //DebugHelper::showScreenMessage("ARobotHandActor::Try Command B", FColor::Cyan);
        
        
        //open hand for pickup
        if(commandIn == ERobotHandCommand::EPickupItem){

            if(APickupCompatible *newItem = MakeSpehereCastPickupable()){
                DropItem();
                pickedUpItem = newItem; //wont be updated because of flag: commandExecuting

                
                FVector size = newItem->Bound();
                handInterpolator.Setup(CurrentGripDistance(), size.Size());
                DebugHelper::showScreenMessage("ARobotHandActor::OPEN HAND", FColor::Red);

                commandExecuting = true;
                command = commandIn;

                //paste refernce to this hand for eternal drop command by drop zone
                newItem->UpdateRobotHandReference(this);
                

                NotifyEvent(EGameEvent::ERobotPickupBox);


            }

            //deprecated!
            /*
            //erstmal einfach so einsammeln.
            AActor *newItem = MakeSpehereCast();
            if(newItem){
                DebugHelper::showScreenMessage("ARobotHandActor::Try Command C", FColor::Red);
                if(newItem != pickedUpItem){
                    DropItem();
                    pickedUpItem = newItem; //wont be updated because of flag: commandExecuting

                    if(APickupCompatible *casted = Cast<APickupCompatible>(newItem)){
                        FVector size = casted->Bound();
                        handInterpolator.Setup(CurrentGripDistance(), size.Size());
                        DebugHelper::showScreenMessage("ARobotHandActor::OPEN HAND", FColor::Red);

                        commandExecuting = true;
                        command = commandIn;

                        //paste refernce to this hand for eternal drop command by drop zone
                        casted->UpdateRobotHandReference(this);
                    }
                }
            }*/
        }
        if(commandIn == ERobotHandCommand::EDropItem){
            if(pickedUpItem){
                DropItem();
                float distA = CurrentGripDistance();
                handInterpolator.Setup(distA, distA + 10);

                commandExecuting = true;
                command = commandIn;

                NotifyEvent(EGameEvent::ERobotDropBox);
            }
        }
        
    }
}

void ARobotHandActor::TickCommand(float deltatime){
    if(commandExecuting){
        float velocity = 10.0f;
        float distance = handInterpolator.Tick(deltatime, velocity);
        if(handInterpolator.isFinished()){
            commandExecuting = false;
            command = ERobotHandCommand::ENone;
        }
        SetGripDistance(distance); //update.
    }
}



void ARobotHandActor::NotifyEvent(EGameEvent event){
    if(ASimulationActor *sim = ASimulationActor::GetInstance()){
        sim->OnGameEvent(event);
        //DebugHelper::showScreenMessage("FireEvent::ERobotPickupBox!", true);
    }
}