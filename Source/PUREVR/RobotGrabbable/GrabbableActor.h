#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GrabComponent.h"
#include "PUREVR/BaseActor/BaseActor.h"
#include "GrabbableActor.generated.h"

UCLASS()
class PUREVR_API AGrabbableActor : public ABaseActor, public IGrabbableInterface
{
	GENERATED_BODY()

public:
	AGrabbableActor();

protected:
	virtual void BeginPlay() override;

public:
	// IGrabbableInterface implementations
	virtual void OnGrabbed_Implementation() override;
	virtual void OnReleased_Implementation() override;

public:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	UStaticMeshComponent* StaticMeshComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	UGrabComponent* GrabComponent;
};
