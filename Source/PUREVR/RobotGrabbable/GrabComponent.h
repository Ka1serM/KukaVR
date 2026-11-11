#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "MotionControllerComponent.h"
#include "GrabComponent.generated.h"

UENUM(BlueprintType)
enum class EGrabType : uint8
{
    None UMETA(DisplayName = "None"),
    Free UMETA(DisplayName = "Free"),
    Snap UMETA(DisplayName = "Snap"),
    Custom UMETA(DisplayName = "Custom")
};

// Simple interface for grab callbacks
UINTERFACE(MinimalAPI, Blueprintable)
class UGrabbableInterface : public UInterface
{
    GENERATED_BODY()
};

class IGrabbableInterface
{
    GENERATED_BODY()

public:
    // Called when object is grabbed
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Grab")
    void OnGrabbed();
    virtual void OnGrabbed_Implementation() {} // default C++ implementation

    // Called when object is released
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Grab")
    void OnReleased();
    virtual void OnReleased_Implementation() {} // default C++ implementation
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class PUREVR_API UGrabComponent : public USceneComponent
{
    GENERATED_BODY()

public:
    UGrabComponent();

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grab")
    EGrabType GrabType = EGrabType::Free;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grab")
    bool bSimulateOnDrop = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grab")
    UHapticFeedbackEffect_Base* OnGrabHapticEffect = nullptr;

    UFUNCTION(BlueprintCallable, Category = "Grab")
    bool TryGrab(UMotionControllerComponent* MotionController);

    UFUNCTION(BlueprintCallable, Category = "Grab")
    bool TryRelease();

    UFUNCTION(BlueprintPure, Category = "Grab")
    bool IsHeld() const { return bIsHeld; }

    UFUNCTION(BlueprintPure, Category = "Grab")
    EControllerHand GetHeldByHand() const;

protected:
    virtual void BeginPlay() override;

private:
    void SetShouldSimulateOnDrop();
    void SetPrimitiveCompPhysics(UPrimitiveComponent* PrimComp, bool bSimulate);

    UPROPERTY()
    UMotionControllerComponent* MotionControllerRef = nullptr;

    bool bIsHeld = false;
};