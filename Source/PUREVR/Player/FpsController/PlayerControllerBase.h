#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Camera/CameraComponent.h"
#include "Components/WidgetInteractionComponent.h"
#include "PUREVR/KeyFrames/frame/KeyFrame.h"

#include "PUREVR/RobotActor/RobotActor.h"
#include "PUREVR/Ui/PlayerUI/UIPlayerWidgetActor.h"
#include "PUREVR/Simulation/ESimulationLevel.h"
#include "PUREVR/Simulation/EGameEvent.h"
#include "PUREVR/Ui/PlayerUI/EPlayerUiEvent.h"

#include "PlayerControllerBase.generated.h"

UCLASS()
class PUREVR_API APlayerControllerBase : public ACharacter
{
	GENERATED_BODY()

public:
	static APlayerControllerBase *playerControllerBaseInstance;

	// Sets default values for this character's properties
	APlayerControllerBase();
	
	void showCursor(bool show);
	
	static APlayerControllerBase *GetInstanceBase(){
		return playerControllerBaseInstance;
	}

	// Initialize method called after all actors are registered
	UFUNCTION(BlueprintCallable, Category = "Initialization")
	void Initialize();

	//called when Initialize is called
	virtual void OnInitialize();
	void GetSimulationActorRef();
    void GetRobotActorRef();
    void GetCameraRef();
    void SetupTransitionMaterialOnInitialize();


	//shared with vr pawn

	// Material instance for the transition effect
	UPROPERTY()
	UMaterialInstanceDynamic* TransitionMaterial;

	// Getter for the transition material
	UFUNCTION(BlueprintCallable, Category = "Materials")
	UMaterialInstanceDynamic* GetTransitionMaterial() const;

	// Method to create and set up the transition material
	UFUNCTION(BlueprintCallable, Category = "Materials")
	void SetupTransitionMaterial( UMaterialInterface* BaseMaterial);

	// We only keep the CameraRef declaration since it's used in SetupTransitionMaterial
	// Other components are defined in blueprint to avoid C++ creation conflicts
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "VR Components", meta = (AllowPrivateAccess = "true"))
	UCameraComponent* CameraRef;

	// References to SimulationActor and RobotActor for blueprint usage
	UPROPERTY(BlueprintReadOnly, Category = "Actor References")
	class ASimulationActor* SimulationActorRef;

	UPROPERTY(BlueprintReadOnly, Category = "Actor References")
	class ARobotActor* RobotActorRef;


	// Called when the actor is being removed from the game or destroyed
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	//show player widget if level is changed
	void ShowPlayerWidget(ESimulationLevel level);
	void UpdatePlayerWidgetVisibility();
	void PauseGame();
	//dispatch game event changes to hud (show interactions)
	void UpdatePlayerWidget(EPlayerUiEvent event);

	void ClearPlayerOnLevelSwitch();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	void CreatePlayerWidgetOnBeginPlay();

	UPROPERTY()
	AUIPlayerWidgetActor *playerUiActor = nullptr;

	bool isVrPawn = false;

	//Todo hier: 
	//interaktion für grab keyframe, mitnehmen!

	bool IsRobotHandle(AKeyFrame *frame);

public:	

	UPROPERTY(EditAnywhere, Category="Player|TraceRange")
	float distanceMax = 1000.0f;

	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;
	
	//UPROPERTY(VisibleAnywhere)
	//UCameraComponent* CameraComponent;

	FRotator cameraRotation();
	FVector playerLookDir();


	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="UI")
	UWidgetInteractionComponent* WidgetInteractor;

	// Getter-Funktion für Blueprint
    UFUNCTION(BlueprintCallable, Category="UI")
    UWidgetInteractionComponent* GetInteractor() const{
		return WidgetInteractor;
	}

protected:
	float placeKeyFrameCoolDown = 1.0f;
	bool KeyFramePlacingAllowed();
	void TickKeyFramePlacingAllowedCoolDown(float deltatime);

	bool aiming;
	bool holding;
	bool sprinting;
	void sprint();
	static const int BASE_SPEED = 600;
	static const int SPRINT_SPEED = 700;
    float playerMaxMovementVelocity = 500.0f;

	bool isWalking;

    AActor *performRaycast();
	bool performRaycastWidgetHit(FHitResult &hit);

	void TurnAtRate(float Rate);
    void LookUpAtRate(float Rate);
	
	virtual void MoveForward(float Value);
    virtual void MoveRight(float Value);
    
	virtual void interact();
	virtual void drop();

	virtual void SwitchRobotState();
	virtual void ClearAnimationKeys();

	void TryGrabKeyframe();
	void DropKeyFrame();
	
	AActor *MakeSpehereCast(float radius);

	//wenn ich einen keyframe bewege dann sollte der alte zurück hinkommen wo er vorher war, 
	//on place von einem neuen!
	FVector latestPickupLocationToOverrideAfterPlacingKeyframe;
	AKeyFrame *grabbedFrame = nullptr;
	void Pickup(AKeyFrame *frame);

	void Jump() override;


    void leftMouseUp();
    void leftMouseDown();

    

    UPROPERTY(EditAnywhere)
    float TurnRateGamepad;

    UPROPERTY(EditAnywhere)
    float LookUpRateGamepad;





	ARobotActor *robotInstance(){
		if(RobotActorRef){
			return RobotActorRef;
		}
		return ARobotActor::GetInstance();
	}

private:
	bool LevelBasedKeyboardInteractionAllowed();

	void UpdateCursorVisibilityBasedOnPause();
	bool cursorVisibleFlag = false;
};
