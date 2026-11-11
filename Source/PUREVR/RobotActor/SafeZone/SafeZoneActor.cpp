#include "SafeZoneActor.h"
#include "DebugPlugin/DebugHelper.h"


ASafeZoneActor::ASafeZoneActor(){
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.bStartWithTickEnabled = true;
    SetActorTickEnabled(true);
}

void ASafeZoneActor::BeginPlay(){
    Super::BeginPlay();
    DebugHelper::logMessage("ASafeZoneActor::BeginPlay");

    CreateMaterial();
    InitMaterialRayParametersSelf();
}

void ASafeZoneActor::Tick(float deltatime){
    Super::Tick(deltatime);

    if(!visibilityInterpolator.isFinished()){
        float velocity = 1.0f;
        float t = visibilityInterpolator.Tick(deltatime, velocity);
        UpdateVisibilty(t);
    }

    //DebugHelper::showScreenMessage("ASafeZoneActor::Tick", FColor::Red);
}

//new based on dot product
void ASafeZoneActor::SetVisibleIfInSameDirection(
    AActor *keyframe,
    AActor *robot,
    bool flag
){
    //wenn ausschalten, dann immer.
    if(!flag){
        SetVisible(false);
        return;
    }

    if(!keyframe || !robot){
        SetVisible(flag);
        return;
    }

    if(keyframe && robot){
        FVector robotLocation = robot->GetActorLocation();
        FVector keyframeLocation = keyframe->GetActorLocation();
        FVector AB = GetActorLocation() - robotLocation; // AB = B - A
        FVector dirFrame = keyframeLocation - robotLocation; //kann ausgelagert werden

        AB.Z = 0.0f;
        dirFrame.Z = 0.0f;
        AB = AB.GetSafeNormal();
        dirFrame = dirFrame.GetSafeNormal();
        float dot = FVector::DotProduct(AB, dirFrame);
        bool visible = dot > 0.0f; //mindestens 90 grad dann visible.
        //DebugHelper::showScreenMessage("Dot Product ", dot);



        //looks just correct
        if(visible){
            DebugHelper::showLineBetween(GetWorld(), robotLocation, GetActorLocation(), FColor::Green, 1.0f);
        }else{
            DebugHelper::showLineBetween(GetWorld(), robotLocation, GetActorLocation(), FColor::Red, 1.0f);
        }

        UpdateMaterialRayParameters(keyframe, robot); // not tested
        SetVisible(visible);
    }
}





//Ok
void ASafeZoneActor::SetVisible(bool flag){
    if (savedVisibility == flag)
    {
        return;
    }
    savedVisibility = flag;


    DebugHelper::showLineBetween(GetWorld(), GetActorLocation(), GetActorLocation() + FVector(10,0,100), FColor::Blue, 1.0f);


    //DebugHelper::showScreenMessage("ASafeZoneActor Visibility Triggered", FColor::Red);

    if(false){
        float s = flag ? 1.0f : 0.0f;
        UpdateVisibilty(s);
        SetActorHiddenInGame(!flag); //debug
        return;
    }

    //debug
    FString message = flag ? TEXT("SafeZone Visible") : TEXT("SafeZone Invisible");
    FString messageOut = FString::Printf(TEXT("ARobotActor::ASafeZoneActor::%s"), *message);
    //DebugHelper::logMessage(message);

    //SetActorHiddenInGame(!flag);

    //DEBUG UNCOMMENT
    /*if(!visibilityInterpolator.isFinished()){
        return;
    }*/

    float t0 = flag ? 0.0f : 1.0f;
    float t1 = flag ? 1.0f : 0.0f;
    visibilityInterpolator.Setup(t0, t1);
}

void ASafeZoneActor::CreateMaterial(){

    //MID->SetScalarParameterValue("MyParam", 1.0f);

    ///Users/walterk/Documents/Uni/Semester5_WS25/pure3d/gitProject/PUREVR/Content/Robot/KeyFrame/red.uasset
    UMaterial* Mat = LoadObject<UMaterial>(
        nullptr,
        TEXT("/Game/Robot/safeZone/safeZoneMaterialNew.safeZoneMaterialNew")
        //TEXT("/Game/Robot/safeZone/safeZoneMaterial.safeZoneMaterial")
    );
    if(Mat){
        UStaticMeshComponent *mesh = FindMesh();
        if(mesh){
            DebugHelper::logMessage("ASafeZoneActorFoundMesh");

            materialInstance = UMaterialInstanceDynamic::Create(Mat, this);
            if(materialInstance){
                mesh->SetMaterial(0, materialInstance);
                DebugHelper::logMessage("ASafeZoneActor::CreatedMaterial");
            }
        }
    }
}

UStaticMeshComponent *ASafeZoneActor::FindMesh(){
    return FindComponentByClass<UStaticMeshComponent>();
}



void ASafeZoneActor::UpdateVisibilty(float scalar){
    if(materialInstance){
        materialInstance->SetScalarParameterValue("visibility", scalar);

        /*DebugHelper::showScreenMessage(
            FString::Printf(TEXT("ASafeZoneActor::changeVisibility %.2f"), scalar)
        );*/
    }
}


void ASafeZoneActor::SetVisibleImmidiate(bool flag){
    float val = flag ? 1.0f : 0.0f;
    UpdateVisibilty(val);
    savedVisibility = flag;
}

//hitted zone marked red?










//update dynamic material properties of radial draw node
void ASafeZoneActor::InitMaterialRayParametersSelf(){
    //UPROPERTY(EditAnywhere, meta = (RequiredInput = "true"))
    //FExpressionInput WorldPositionSelf;

    //UPROPERTY(EditAnywhere, meta = (RequiredInput = "true"))
    //FExpressionInput NormalPlane;

    //UPROPERTY(EditAnywhere, meta = (RequiredInput = "true"))
    //FExpressionInput radius;

    if(materialInstance){
        //radius of grid drawing
        materialInstance->SetScalarParameterValue("radius", 200.0f);


        //normal of plane
        if(UStaticMeshComponent *mesh = FindMesh()){
            FRotator r = mesh->GetComponentRotation();
            FVector normal(0, 0, 1); //default is up.
            normal = r.RotateVector(normal);
            materialInstance->SetVectorParameterValue("PlaneNormal", normal);
            //DebugHelper::logMessage("ASafeZoneActor::Setup::PlaneNormal ", normal); //looks good.

            //looks good.
            FVector location = mesh->GetComponentLocation();
            /*DebugHelper::showLineBetween(
                GetWorld(),
                location,
                location + normal * 100,
                FColor::Red,
                100.0f
            );
            DebugHelper::showLineBetween(
                GetWorld(),
                location,
                location + FVector(0,0,100),
                FColor::Blue,
                100.0f
            );*/
        }
        
        //worldPositionOfPlane (offset 250 might be needed)
        FVector pos = GetActorLocation();
        materialInstance->SetVectorParameterValue("PlaneOrigin", pos);
    }
}


void ASafeZoneActor::UpdateMaterialRayParameters(AActor *robot, AActor *keyframe){
    /*
    UPROPERTY(EditAnywhere, meta = (RequiredInput = "true"))
    FExpressionInput WorldPositionOrigin; //ray origin

    UPROPERTY(EditAnywhere, meta = (RequiredInput = "true"))
    FExpressionInput WorldPositionObject;
    */
    if(robot && keyframe){
        if(materialInstance){
            FVector worldKeyFrame = keyframe->GetActorLocation();
            FVector worldRayOrigin = robot->GetActorLocation();

            materialInstance->SetVectorParameterValue("RayOrigin", worldRayOrigin);
            materialInstance->SetVectorParameterValue("KeyFrameWorld", worldKeyFrame);

            //DebugHelper::showScreenMessage("ASafeZoneActor::UpdateRaycastParamsShader", FColor::Purple);


            //FVector location = mesh->GetComponentLocation();
            //fixes safe zone bug!
            FVector pos = GetActorLocation();
            DebugHelper::showLineBetween(GetWorld(), GetActorLocation(), GetActorLocation() + FVector(0, 0, 100), FColor::Orange);
            materialInstance->SetVectorParameterValue("PlaneOrigin", pos);
        }
    }
}





bool ASafeZoneActor::ScaleTo(int cmIn){
    if(UStaticMeshComponent *mesh = FindMesh()){
        //to scale, since plane is 1m 1m by default in bp
        float asScale = cmIn / 100.0f;

        mesh->SetWorldScale3D(FVector(asScale, asScale, asScale));
        return true;
    }
    return false;
}
