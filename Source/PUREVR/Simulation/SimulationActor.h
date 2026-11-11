#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "LevelDataAsset.h"
#include "ESimulationLevel.h"
#include "PUREVR/Simulation/EGameEvent.h"
#include "PUREVR/RobotActor/RobotActor.h"
#include "PUREVR/Ui/TaskWidget.h"
#include "PUREVR/Ui/UIActor.h"
#include "Engine/Engine.h"
#include "PUREVR/Simulation/LevelSpawned/LevelSpawnedActorManager.h"
#include "PUREVR/Ui/MenuWidget.h"

#include "SimulationActor.generated.h"

/**
 * Main simulation controller that manages the connection between player, robot, and tasks
 */
UCLASS()
class PUREVR_API ASimulationActor : public AActor
{
	GENERATED_BODY()

private:
	static ASimulationActor* SimulationActorInstance;

	// Private constructor for singleton pattern
	ASimulationActor();

public:
	UFUNCTION(BlueprintCallable, Category = "SimulationActor")
	static ASimulationActor* GetInstance();

	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	

	/*==================================
	 * REFERENCES
	 *===============================*/

	// Reference to the player pawn
	UPROPERTY(BlueprintReadOnly, Category = "References")
	APawn* PlayerReference = nullptr;

	// Reference to the robot actor
	UPROPERTY(BlueprintReadOnly, Category = "References")
	ARobotActor* RobotReference = nullptr;

	/*==================================
	 * REGISTRATION SYSTEM
	 *===============================*/

	// Register player pawn for initialization
	UFUNCTION(BlueprintCallable, Category = "Initialization")
	void RegisterPlayer(APawn* Player);

	// Register robot for initialization
	UFUNCTION(BlueprintCallable, Category = "Initialization")
	void RegisterRobot(ARobotActor* Robot);

	// Check if all required actors are registered and can initialize
	void TryInitialize();

	// Initialize all registered actors
	UFUNCTION(BlueprintCallable, Category = "Initialization")
	void InitializeAll();

	UPROPERTY()
	bool bInitialized = false;
	
	/*==================================
	 * UI
	 *===============================*/

	// Reference to the main menu UI actor
	UPROPERTY(BlueprintReadOnly, Category = "UI")
	AUIActor* MainMenuActor = nullptr;

	// Reference to the task UI actor
	UPROPERTY(BlueprintReadOnly, Category = "UI")
	AUIActor* TaskUIActor = nullptr;

	// Reference to the pause menu UI actor
	UPROPERTY(BlueprintReadOnly, Category = "UI")
	AUIActor* PauseMenuActor = nullptr;
	
	UPROPERTY(BlueprintReadOnly, Category = "UI")
	AUIActor* TaskFinishedUIActor = nullptr;
	
	// Class reference for menu widget
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI")
	TSubclassOf<UMenuWidget> MenuClass;

	// Class reference for task widget
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI")
	TSubclassOf<UTaskWidget> TaskClass;

	// Class reference for pause menu widget
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI")
	TSubclassOf<UUserWidget> PauseMenuClass;

	// Class reference for number widget
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI")
	TSubclassOf<UUserWidget> NumberWidgetClass;


	// Class reference for number widget
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI")
	TSubclassOf<UUserWidget> PlayerWidgetClass;
	
	// Class reference for task finished widget
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI")
	TSubclassOf<UUserWidget> TaskFinishedClass;
	
	// World position where the main menu widget should spawn
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "References")
	FVector MainMenuSpawnPosition = FVector(2495.860015f, 2420.872729f, 224.610259f);

	// World position where the player should spawn when entering a level
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "References")
	FVector PlayerStartPosition = FVector(2933.778422f, 2324.597239f, 28.277010f); // Robot position (2583.778422, 2124.597239, 28.277010) + offset (350, 200, 0)

	// World position for the player when returning to the Home/Intro menu (Safe Spot)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "References")
	FVector PlayerHomePosition = FVector(2188.009110f, 2060.686314f, 162.781108f);
	
	// World position where the Tasks UI should spawn when entering a level
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "References")
	FVector TasksUISpawnPosition = FVector(2583.778422f, 2874.597239f, 300.f); // Robot position (2583.778422, 2124.597239, 28.277010) + offset (0, 750, 300)
	
	// World position where the Task Finished UI should spawn when level completes
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "References")
	FVector TaskFinishedUISpawnPosition = FVector(2583.778422f, 2924.597239f, 300.f); // 0.5m (50cm) in front of TasksUI
	
	// Base material for the matrix transition effect (user selects this in editor)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Levels")
	UMaterialInterface* BaseTransitionMaterial;

	// Material instance for the matrix transition effect (created at runtime - private reference)
	UPROPERTY()
	UMaterialInstanceDynamic* MatrixTransitionMaterial;

	// Time for each half of the transition animation (0 to 0.5, then 0.5 to 1.0)
	// Total transition time will be 2x this value (15 seconds total = 7.5 seconds per half)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Levels")
	float TransitionDuration = 7.5f;

	// Time to wait before starting the initial transition at startup
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Levels")
	float InitialTransitionDelay = 5.0f;

	// Time remaining before starting the initial transition
	UPROPERTY()
	float InitialTransitionDelayRemaining = 0.0f;

	// Whether we're waiting for the initial delay to complete
	UPROPERTY()
	bool bWaitingForInitialDelay = false;

	/*==================================
	 * TRANSITION ANIMATION VARIABLES
	 *===============================*/

	// Current progress value for the transition
	UPROPERTY()
	float CurrentTransitionProgress = 0.0f;

	// Target progress value for the transition
	UPROPERTY()
	float TargetProgress = 0.0f;

	// Whether a transition is currently in progress
	UPROPERTY()
	bool bIsTransitioning = false;

	// Whether we're in the first or second half of the transition
	UPROPERTY()
	bool bIsFirstHalfTransition = false;

	// The level enum being loaded
	UPROPERTY()
	ESimulationLevel PendingLevelEnum = ESimulationLevel::ENone;

	/*==================================
	 * LEVEL AND TASK MANAGEMENT
	 *===============================*/

	// Array of levels in the simulation - now mapped to enum values
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Levels")
	TArray<ULevelDataAsset*> Levels;

	// Current level enum
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Levels")
	ESimulationLevel CurrentLevel = ESimulationLevel::ENone;

	// Current task index within the current level (index-based to prevent desync)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Levels")
	int32 CurrentTaskIndex = 0;

	// Getter methods to access current items safely
	UFUNCTION(BlueprintPure, Category = "Simulation")
	ULevelDataAsset* GetCurrentLevel() const;

	// Get level data asset by enum
	UFUNCTION(BlueprintPure, Category = "Simulation")
	ULevelDataAsset* GetLevelByEnum(ESimulationLevel LevelEnum) const;

	// Main event handler that gets called by robot and player
	void OnGameEvent(EGameEvent Event, void* EventData = nullptr);

	/*==================================
	 * TRANSITION MATERIAL
	 *===============================*/

	// Create and setup the transition material in player camera
	UFUNCTION(BlueprintCallable, Category = "Materials")
	void CreateAndSetupTransitionMaterial();

	// Getter for the transition material
	UFUNCTION(BlueprintPure, Category = "Materials")
	UMaterialInstanceDynamic* GetTransitionMaterial() const { return MatrixTransitionMaterial; }

	/*==================================
	 * PAUSE/RESUME SYSTEM
	 *===============================*/
	
	// Pause the game and show the pause menu
	UFUNCTION(BlueprintCallable, Category = "Pause")
	void TogglePause();

	// Check if the game is currently paused
	UFUNCTION(BlueprintPure, Category = "Pause")
	bool IsGamePaused() const { return bIsPaused; }
	
protected:
	/*==================================
	 * LEVEL LOADING
	 *===============================*/

	// Load a specific level by enum
	UFUNCTION(BlueprintCallable, Category = "Level Loading")
	void LoadLevel(ESimulationLevel LevelEnum);

	// Begin the level loading process - starts transition shader
	void BeginLoad(ESimulationLevel LevelEnum);

	// Perform the actual level loading - spawns UI, teleports player, etc.
	void Load(ESimulationLevel LevelEnum);

	// Complete the level loading process - finishes transition shader
	void EndLoad();

	UFUNCTION(BlueprintCallable, Category = "Game Flow")
	void StartNextLevel();

	UFUNCTION(BlueprintCallable, Category = "Game Flow")
	void RestartCurrentLevel();

	// Dedicated tick method for handling level loading transitions
	void LoadLevelTick(float DeltaTime);

	// Start the initial startup transition from 0.5 to 1.0
	void StartInitialTransition();

	// Complete the current level
	void CompleteCurrentLevel();
	
private:
	// Whether the game is currently paused
	UPROPERTY()
	bool bIsPaused = false;


	//clears player and robot on game level change / restart.
	void ClearOperationOnLevelChangeOrReload();

public:
	/*==================================
	 * GAME FLOW CONTROLS
	 *===============================*/

	/** * Returns the player to the startup/home menu 
	 * Resets level visibility, player state, and UI
	 */
	UFUNCTION(BlueprintCallable, Category = "Game Flow")
	void ReturnToHomeMenu();

private:
	/**
	 * Spawned extra actors from level assets - clears old actors
	 * on level change / reload, stores them inside.
	 */
	LevelSpawnedActorManager levelBasedActorManager;

	//load or reload.
	void StartLevelActorsFrom(ULevelDataAsset *asset);
};