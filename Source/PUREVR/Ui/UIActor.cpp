#include "UIActor.h" 
#include "PUREVR/Player/VRPawn.h" 
#include "Kismet/KismetMathLibrary.h"
#include "Components/StaticMeshComponent.h"
#include "UObject/ConstructorHelpers.h"
#include "Materials/MaterialInstanceDynamic.h"
 
AUIActor::AUIActor() 
{ 
    // Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it. 
    PrimaryActorTick.bCanEverTick = true; 
    PrimaryActorTick.bTickEvenWhenPaused = true; 
 
    // Create the widget component as root
    Widget = CreateDefaultSubobject<UWidgetComponent>(TEXT("Widget")); 
    RootComponent = Widget;
    
    // Set scale to 0.5 on all axes 
    Widget->SetWorldScale3D(FVector(0.5f, 0.5f, 0.5f)); 
 
    Widget->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics); 
    Widget->SetCollisionResponseToAllChannels(ECR_Ignore); 
    Widget->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block); 
    Widget->SetCollisionResponseToChannel(ECC_Camera, ECR_Block); 
    Widget->SetCollisionResponseToChannel(ECC_GameTraceChannel1, ECR_Block); 
 
    Widget->SetTwoSided(true); 
    Widget->SetBlendMode(EWidgetBlendMode::Transparent); 
     
    Widget->SetDrawAtDesiredSize(true); 
    Widget->SetGeometryMode(EWidgetGeometryMode::Cylinder); 
    Widget->SetCylinderArcAngle(30.0f);

    // Create cursor mesh component
    CursorMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("CursorMesh"));
    CursorMesh->SetupAttachment(Widget);
    
    // Load default sphere mesh
    static ConstructorHelpers::FObjectFinder<UStaticMesh> SphereMesh(TEXT("/Engine/BasicShapes/Sphere"));
    if (SphereMesh.Succeeded())
    {
        CursorMesh->SetStaticMesh(SphereMesh.Object);
    }
    
    // Load custom cursor material
    static ConstructorHelpers::FObjectFinder<UMaterial> CursorMat(TEXT("/Game/UI/CursorMaterial"));
    if (CursorMat.Succeeded())
    {
        CursorMesh->SetMaterial(0, CursorMat.Object);
    }
    
    // Configure cursor appearance
    CursorMesh->SetWorldScale3D(FVector(0.25f, 0.25f, 0.25f)); // Small cursor
    CursorMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    CursorMesh->SetCastShadow(false);
} 
 
// Called when the game starts or when spawned 
void AUIActor::BeginPlay() 
{ 
    Super::BeginPlay(); 
} 
 
// Called every frame 
void AUIActor::Tick(float DeltaTime) 
{ 
    Super::Tick(DeltaTime); 
 
    APlayerControllerBase* PlayerPawn = APlayerControllerBase::GetInstanceBase(); 
    if (!PlayerPawn) 
    { 
       return; 
    } 
 
    UCameraComponent* Camera = PlayerPawn->CameraRef; 
    if (!Camera) 
    { 
       return; 
    } 
 
    FVector CameraLocation = Camera->GetComponentLocation();
    FVector NewLocation = GetActorLocation();
    
    /* ---------------- POSITION ---------------- */ 
    if (bFollowPlayer) 
    { 
       FVector CameraForward = Camera->GetForwardVector(); 
       NewLocation = CameraLocation + CameraForward * FollowDistance; 
       NewLocation.Z = CameraLocation.Z;
        SetActorLocation(NewLocation);
        
    } 
 
    /* ---------------- ROTATION ---------------- */ 
    if (bAlwaysFacePlayer) 
    {
       // Use FindLookAtRotation for safe calculation
       FRotator TargetRotation = UKismetMathLibrary::FindLookAtRotation(NewLocation, CameraLocation);
        SetActorRotation(TargetRotation, ETeleportType::TeleportPhysics);
    }

    /* ---------------- CURSOR UPDATE ---------------- */
    if (bShowCursor) 
    {
        UpdateCursor(Camera);
    }
    else
    {
        // Hide cursor when bShowCursor is false
        if (CursorMesh)
        {
            CursorMesh->SetVisibility(false);
        }
    }
} 

void AUIActor::UpdateCursor(UCameraComponent* Camera)
{
    if (!Camera || !CursorMesh)
    {
        return;
    }

    FVector CameraLocation = Camera->GetComponentLocation();
    FVector CameraForward = Camera->GetForwardVector();

    // Perform line trace from camera
    FHitResult HitResult;
    FVector TraceStart = CameraLocation;
    FVector TraceEnd = CameraLocation + CameraForward * 10000.0f; // 100m trace

    FCollisionQueryParams QueryParams;
    QueryParams.AddIgnoredActor(this);

    // Trace against the widget component
    if (Widget->LineTraceComponent(HitResult, TraceStart, TraceEnd, QueryParams))
    {
        // Convert world space hit location to widget local space
        FVector LocalHitLocation = Widget->GetComponentTransform().InverseTransformPosition(HitResult.Location);
        
        // Position cursor at hit location (slightly offset forward to avoid z-fighting)
        CursorMesh->SetRelativeLocation(LocalHitLocation + FVector(0.5f, 0, 0));
        CursorMesh->SetVisibility(true);
    }
    else
    {
        // Hide cursor if not pointing at widget
        CursorMesh->SetVisibility(false);
    }
}
 
void AUIActor::SetWidgetClass(TSubclassOf<UUserWidget> NewWidgetClass) 
{ 
    if (NewWidgetClass) 
    { 
       CurrentWidgetClass = NewWidgetClass; 
       Widget->SetWidgetClass(NewWidgetClass); 
    } 
} 
 
TSubclassOf<UUserWidget> AUIActor::GetWidgetClass() const 
{ 
    return CurrentWidgetClass; 
}