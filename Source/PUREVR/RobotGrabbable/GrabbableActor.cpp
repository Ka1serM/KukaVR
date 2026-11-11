#include "GrabbableActor.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"


AGrabbableActor::AGrabbableActor()
{
	PrimaryActorTick.bCanEverTick = false;

	// Create static mesh component
	StaticMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("StaticMeshComponent"));
	RootComponent = StaticMeshComponent;

	// Create grab component
	GrabComponent = CreateDefaultSubobject<UGrabComponent>(TEXT("GrabComponent"));
	GrabComponent->SetupAttachment(StaticMeshComponent);

	// Default mesh
	UStaticMesh* SphereMesh = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Sphere"), nullptr, LOAD_None, nullptr);
	if (SphereMesh)
	{
		StaticMeshComponent->SetStaticMesh(SphereMesh);
	}

	// Collision
	StaticMeshComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	StaticMeshComponent->SetCollisionResponseToAllChannels(ECR_Block);
}

void AGrabbableActor::BeginPlay()
{
	Super::BeginPlay();
}

void AGrabbableActor::OnGrabbed_Implementation()
{
	UE_LOG(LogTemp, Warning, TEXT("AGrabbableActor grabbed!"));
}

void AGrabbableActor::OnReleased_Implementation()
{
	UE_LOG(LogTemp, Warning, TEXT("AGrabbableActor released!"));
}
