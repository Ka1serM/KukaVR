#include "GrabComponent.h"
#include "Components/PrimitiveComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "Components/SceneComponent.h"

UGrabComponent::UGrabComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
    bWantsInitializeComponent = true;
    bAutoActivate = true;
}

void UGrabComponent::BeginPlay()
{
    Super::BeginPlay();

    SetShouldSimulateOnDrop();

    USceneComponent* ComponentAttachParent = this->GetAttachParent();
    if (UPrimitiveComponent* PrimAttachParent = Cast<UPrimitiveComponent>(ComponentAttachParent))
    {
        PrimAttachParent->SetCollisionProfileName(TEXT("PhysicsActor"));
    }
}

void UGrabComponent::SetShouldSimulateOnDrop()
{
    USceneComponent* ComponentAttachParent = this->GetAttachParent();
    if (UPrimitiveComponent* PrimAttachParent = Cast<UPrimitiveComponent>(ComponentAttachParent))
    {
        bSimulateOnDrop = PrimAttachParent->IsAnySimulatingPhysics();
    }
}

bool UGrabComponent::TryGrab(UMotionControllerComponent* MotionController)
{
    if (!MotionController || bIsHeld)
    {
        return false;
    }

    MotionControllerRef = MotionController;

    AActor* OwnerActor = GetOwner();
    if (!OwnerActor)
    {
        return false;
    }

    // Handle physics based on grab type
    switch (GrabType)
    {
    case EGrabType::Free:
    case EGrabType::Snap:
        if (UPrimitiveComponent* PrimComp = Cast<UPrimitiveComponent>(OwnerActor->GetRootComponent()))
        {
            SetPrimitiveCompPhysics(PrimComp, false);
        }
        break;
    case EGrabType::Custom:
    case EGrabType::None:
    default:
        break;
    }

    // Attach to motion controller
    USceneComponent* RootComponent = OwnerActor->GetRootComponent();
    if (RootComponent)
    {
        FAttachmentTransformRules AttachmentRules = FAttachmentTransformRules::KeepWorldTransform;
        RootComponent->AttachToComponent(MotionController, AttachmentRules);
    }

    bIsHeld = true;

    // Call the interface method directly on the owner
    if (OwnerActor->Implements<UGrabbableInterface>())
    {
        IGrabbableInterface::Execute_OnGrabbed(OwnerActor);
    }

    // Play haptic feedback
    if (OnGrabHapticEffect)
    {
        if (APlayerController* PC = Cast<APlayerController>(UGameplayStatics::GetPlayerController(GetWorld(), 0)))
        {
            PC->PlayHapticEffect(OnGrabHapticEffect, GetHeldByHand());
        }
    }

    return true;
}

bool UGrabComponent::TryRelease()
{
    if (!bIsHeld)
        return false;

    AActor* OwnerActor = GetOwner();
    if (!OwnerActor)
        return false;

    // Handle physics based on grab type
    if ((GrabType == EGrabType::Free || GrabType == EGrabType::Snap) && bSimulateOnDrop)
    {
        if (UPrimitiveComponent* PrimComp = Cast<UPrimitiveComponent>(OwnerActor->GetRootComponent()))
        {
            SetPrimitiveCompPhysics(PrimComp, true);
        }
    }

    // Detach from motion controller
    if (USceneComponent* RootComponent = OwnerActor->GetRootComponent())
    {
        if (RootComponent->GetAttachParent() == MotionControllerRef)
        {
            RootComponent->DetachFromComponent(FDetachmentTransformRules::KeepWorldTransform);
        }
    }

    // Call interface method correctly
    if (OwnerActor->Implements<UGrabbableInterface>())
    {
        IGrabbableInterface::Execute_OnReleased(OwnerActor);
    }

    bIsHeld = false;
    MotionControllerRef = nullptr;

    return true;
}


void UGrabComponent::SetPrimitiveCompPhysics(UPrimitiveComponent* PrimComp, bool bSimulate)
{
    if (!PrimComp)
        return;

    PrimComp->SetSimulatePhysics(bSimulate);
    PrimComp->SetEnableGravity(bSimulate);
    PrimComp->SetCollisionEnabled(bSimulate ? ECollisionEnabled::PhysicsOnly : ECollisionEnabled::QueryAndPhysics);
}

EControllerHand UGrabComponent::GetHeldByHand() const
{
    if (MotionControllerRef)
    {
        FName MotionSourceType = MotionControllerRef->MotionSource;

        if (MotionSourceType == FName("Left"))
        {
            return EControllerHand::Left;
        }
        else if (MotionSourceType == FName("Right"))
        {
            return EControllerHand::Right;
        }
        else
        {
            FString ComponentName = MotionControllerRef->GetName();
            if (ComponentName.Contains("Left", ESearchCase::IgnoreCase) ||
                ComponentName.Contains("L_", ESearchCase::IgnoreCase))
            {
                return EControllerHand::Left;
            }
            else
            {
                return EControllerHand::Right;
            }
        }
    }

    return EControllerHand::Right;
}