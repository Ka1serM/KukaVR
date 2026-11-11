// Fill out your copyright notice in the Description page of Project Settings.

#include "SimulationActor.h"
#include "Kismet/GameplayStatics.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "TimerManager.h"
#include "PUREVR/Simulation/EGameEvent.h"
#include "PUREVR/RobotActor/RobotActor.h"
#include "PUREVR/Ui/TaskWidget.h"
#include "PUREVR/RobotActor/ERobotState.h"
#include "PUREVR/Ui/UIActor.h"
#include "PUREVR/Player/VRPawn.h"

#include "DebugPlugin/DebugHelper.h"

ASimulationActor* ASimulationActor::SimulationActorInstance = nullptr;

// Sets default values
ASimulationActor::ASimulationActor()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	// Initialize transition variables
	CurrentTransitionProgress = 0.0f;
	TargetProgress = 0.0f;
	bIsTransitioning = false;
	bIsFirstHalfTransition = false;
	MatrixTransitionMaterial = nullptr; // Initialize to nullptr

	// Set the default transition material path
	static ConstructorHelpers::FObjectFinder<UMaterialInterface> TransitionMaterialObj(TEXT("/Game/Materials/PostProcess/MatrixTransition_Inst.MatrixTransition_Inst"));
	if (TransitionMaterialObj.Succeeded())
	{
		BaseTransitionMaterial = TransitionMaterialObj.Object;
		UE_LOG(LogTemp, Warning, TEXT("Transition material found and set in constructor."));
	}

	// Set as singleton instance in constructor of the actual spawned actor
	SimulationActorInstance = this;
}

ASimulationActor* ASimulationActor::GetInstance()
{
	return SimulationActorInstance;
}

// Called when the game starts or when spawned
void ASimulationActor::BeginPlay()
{
	Super::BeginPlay();

	// Set the PlayerReference using the VRPawn singleton
	PlayerReference = AVRPawn::GetInstance();
	RobotReference = ARobotActor::GetInstance();

	// Manage streaming level visibility at startup:
	// - Show Intro and Persistent levels
	// - Hide Simple level initially
	if (UWorld* World = GetWorld())
	{
		// Set Intro level to visible
		UGameplayStatics::LoadStreamLevel(World, TEXT("Intro"), true, true, FLatentActionInfo());
		if (ULevelStreaming* IntroLevel = UGameplayStatics::GetStreamingLevel(World, TEXT("Intro")))
		{
			IntroLevel->SetShouldBeVisible(true);
			IntroLevel->SetShouldBeLoaded(true);
		}

		// Set Simple level to hidden initially
		UGameplayStatics::LoadStreamLevel(World, TEXT("Simple"), true, true, FLatentActionInfo());
		if (ULevelStreaming* SimpleLevel = UGameplayStatics::GetStreamingLevel(World, TEXT("Simple")))
		{
			SimpleLevel->SetShouldBeVisible(false);
			SimpleLevel->SetShouldBeLoaded(true);
		}
	}

	if (GetWorld())
	{
		// Spawn main menu actor at the specified spawn position
		FActorSpawnParameters SpawnParams;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		if (!MainMenuActor)
		{
			MainMenuActor = GetWorld()->SpawnActor<AUIActor>(AUIActor::StaticClass(), MainMenuSpawnPosition, FRotator::ZeroRotator, SpawnParams);
			MainMenuActor->SetWidgetClass(MenuClass);
			MainMenuActor->bShowCursor = true;
			if (UMenuWidget* MenuWidget = MainMenuActor->GetWidget<UMenuWidget>())
			{
				MenuWidget->LoadLevels(Levels);
			}
		}

		// Spawn task menu actor and hide it by default
		if (!TaskUIActor)
		{
			TaskUIActor = GetWorld()->SpawnActor<AUIActor>(AUIActor::StaticClass(), TasksUISpawnPosition, FRotator::ZeroRotator, SpawnParams);
			if (TaskUIActor)
			{
				TaskUIActor->SetWidgetClass(TaskClass);
				TaskUIActor->SetActorHiddenInGame(true);
			}
		}

		// Spawn pause menu actor and hide it by default
		if (!PauseMenuActor)
		{
			PauseMenuActor = GetWorld()->SpawnActor<AUIActor>(AUIActor::StaticClass(), FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);
			if (PauseMenuActor)
			{
				PauseMenuActor->SetWidgetClass(PauseMenuClass);
				PauseMenuActor->SetActorHiddenInGame(true);
				// Configure pause menu to follow player
				PauseMenuActor->bFollowPlayer = false;
				PauseMenuActor->FollowDistance = 300.0f; // 3 meters in front
				PauseMenuActor->bAlwaysFacePlayer = true;
				PauseMenuActor->bShowCursor = true;
			}
		}
		
		if (!TaskFinishedUIActor)
		{
			TaskFinishedUIActor = GetWorld()->SpawnActor<AUIActor>(AUIActor::StaticClass(), TaskFinishedUISpawnPosition, FRotator::ZeroRotator, SpawnParams);
			if (TaskFinishedUIActor)
			{
				TaskFinishedUIActor->SetWidgetClass(TaskFinishedClass);
				TaskFinishedUIActor->SetActorHiddenInGame(true);
				TaskFinishedUIActor->bAlwaysFacePlayer = true;
				TaskFinishedUIActor->bShowCursor = true;
			}
		}
	}

	// DON'T create and setup the transition material here - VRPawn may not be initialized yet
	// Instead, the material will be created when RegisterPlayer is called from VRPawn::BeginPlay

	// Set up the initial transition parameters for startup
	TargetProgress = 1.0f;
	InitialTransitionDelayRemaining = InitialTransitionDelay;
	bWaitingForInitialDelay = true;
	// The actual transition will be started when RegisterPlayer is called and material is available
}

void ASimulationActor::CreateAndSetupTransitionMaterial()
{
	// Find the VRPawn (prioritizing the singleton)
	AVRPawn* VRPlayerPawn = AVRPawn::GetInstance();
	if (!VRPlayerPawn && PlayerReference)
	{
		VRPlayerPawn = Cast<AVRPawn>(PlayerReference);
	}

	if (VRPlayerPawn && BaseTransitionMaterial)
	{
		// Set up the transition material in the VRPawn first
		VRPlayerPawn->SetupTransitionMaterial(BaseTransitionMaterial);

		// Get the transition material from VRPawn
		MatrixTransitionMaterial = VRPlayerPawn->GetTransitionMaterial();
	}

	// Set initial progress to 0.5 as required if material was found
	if (MatrixTransitionMaterial)
	{
		MatrixTransitionMaterial->SetScalarParameterValue("Progress", 0.5f);
		CurrentTransitionProgress = 0.5f;
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("CreateAndSetupTransitionMaterial: Failed to get transition material from VRPawn"));
	}
}

void ASimulationActor::RegisterPlayer(APawn* Player)
{
	if (Player)
	{
		PlayerReference = Player;

		// Create and setup the transition material now that player is available
		CreateAndSetupTransitionMaterial();

		// If we're waiting for initial transition and now have the material, start it
		if (bWaitingForInitialDelay && MatrixTransitionMaterial)
		{
			StartInitialTransition();
		}

		TryInitialize();
	}
}

void ASimulationActor::RegisterRobot(ARobotActor* Robot)
{
	if (Robot)
	{
		RobotReference = Robot;

		// Create and setup the transition material if player is also available but material wasn't created yet
		if (PlayerReference && !MatrixTransitionMaterial)
		{
			CreateAndSetupTransitionMaterial();

			// If we're waiting for initial transition and now have the material, start it
			if (bWaitingForInitialDelay && MatrixTransitionMaterial)
			{
				StartInitialTransition();
			}
		}

		TryInitialize();
	}
}

void ASimulationActor::TryInitialize()
{
	if (!bInitialized && PlayerReference && RobotReference)
	{
		bInitialized = true;
		InitializeAll();
	}
}

void ASimulationActor::InitializeAll()
{
	// Initialize all registered actors
	if (PlayerReference)
	{
		// Cast to VRPawn to call Initialize if it exists
		APlayerControllerBase* VRPawn = Cast<APlayerControllerBase>(PlayerReference);
		if (VRPawn)
		{
			VRPawn->Initialize();
		}
	}

	if (RobotReference)
	{
		RobotReference->Initialize();
	}
}

void ASimulationActor::StartInitialTransition()
{
	// Start the delay before the initial transition
	if (MatrixTransitionMaterial)
	{
		// Set the material's Progress parameter to 0.5 initially (but don't start transitioning yet)
		MatrixTransitionMaterial->SetScalarParameterValue("Progress", 0.5f);
		CurrentTransitionProgress = 0.5f;
		TargetProgress = 1.0f;

		// Start the delay
		InitialTransitionDelayRemaining = InitialTransitionDelay;
		bWaitingForInitialDelay = true;
		bIsTransitioning = false; // Don't start transition yet
		bIsFirstHalfTransition = false; // Initial startup is the "second half" transition (0.5 to 1.0)
	}
	else
	{
		// If no material available, just skip the transition
		bWaitingForInitialDelay = false;
		bIsTransitioning = false;
		bIsFirstHalfTransition = false;
		// Still proceed with normal gameplay
		UE_LOG(LogTemp, Warning, TEXT("StartInitialTransition: MatrixTransitionMaterial is null, skipping initial transition"));
	}
}

// Called every frame
void ASimulationActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// Handle the initial delay countdown if waiting
	if (bWaitingForInitialDelay && MatrixTransitionMaterial)
	{
		InitialTransitionDelayRemaining -= DeltaTime;
		if (InitialTransitionDelayRemaining <= 0.0f)
		{
			// Delay is over, start the transition
			bWaitingForInitialDelay = false;

			// Only start the transition if MatrixTransitionMaterial is available
			if (MatrixTransitionMaterial)
			{
				bIsTransitioning = true;
			}
		}
	}

	// Handle level loading transition if one is in progress
	if (bIsTransitioning)
	{
		LoadLevelTick(DeltaTime);
	}
}

// Dedicated method for handling level loading transitions
void ASimulationActor::LoadLevelTick(float DeltaTime)
{
	// Early return if material is not available - CRITICAL FIX: Always check material validity
	if (!MatrixTransitionMaterial)
	{
		UE_LOG(LogTemp, Warning, TEXT("LoadLevelTick: MatrixTransitionMaterial is null, stopping transition!"));
		// Stop transitioning if the material is not available
		bIsTransitioning = false;
		// Reset pending level to prevent stuck state
		PendingLevelEnum = ESimulationLevel::ENone;
		return;
	}

	// Calculate the interpolation speed based on duration
	float Speed = 1.0f / TransitionDuration; // 1 unit per second to complete in TransitionDuration
	float Increment = Speed * DeltaTime;

	// Move towards target
	if (CurrentTransitionProgress < TargetProgress)
	{
		CurrentTransitionProgress = FMath::Min(CurrentTransitionProgress + Increment, TargetProgress);
	}
	else
	{
		CurrentTransitionProgress = FMath::Max(CurrentTransitionProgress - Increment, TargetProgress);
	}

	// Update the material parameter - double-check material is still valid
	if (MatrixTransitionMaterial)
	{
		MatrixTransitionMaterial->SetScalarParameterValue("Progress", CurrentTransitionProgress);
	}

	if (FMath::IsNearlyEqual(CurrentTransitionProgress, TargetProgress, 0.001f))
	{
		// Reached target, stop transitioning
		CurrentTransitionProgress = TargetProgress;

		// Double-check material is still valid before setting final value
		if (MatrixTransitionMaterial)
		{
			MatrixTransitionMaterial->SetScalarParameterValue("Progress", CurrentTransitionProgress);
		}

		// Handle transition phase completion
		bIsTransitioning = false;

		if (bIsFirstHalfTransition)
		{
			// We've completed the first half (0.0 to 0.5)
			// Now perform the actual loading, but only if there's a valid level to load
			// and required objects are available
			if (PendingLevelEnum != ESimulationLevel::ENone)
			{
				// Only proceed if critical objects are available
				if (PlayerReference != nullptr)
				{
					Load(PendingLevelEnum);

					// Verify material still exists before starting second half
					if (MatrixTransitionMaterial)
					{
						// Start second half transition (0.5 to 1.0)
						TargetProgress = 1.0f;
						bIsTransitioning = true;
						bIsFirstHalfTransition = false;
					}
					else
					{
						// Material became invalid during loading, complete immediately
						UE_LOG(LogTemp, Warning, TEXT("LoadLevelTick: Material became invalid after first half, completing immediately"));
						EndLoad();
					}
				}
				else
				{
					// Player reference is null, complete the loading without transition
					EndLoad();
				}
			}
		}
		else
		{
			// We've completed the second half (0.5 to 1.0)
			// Check if this is the initial startup transition or a level loading transition
			if (PendingLevelEnum == ESimulationLevel::ENone)
			{
				// This is the initial startup transition (0.5 to 1.0)
				// Just finish the transition without calling EndLoad
				// The initial loading screen is now complete
			}
			else
			{
				// This is a level loading transition, finish the loading process
				// Only proceed if critical objects are available
				if (PlayerReference != nullptr)
				{
					EndLoad();
				}
			}
		}
	}
}

ULevelDataAsset* ASimulationActor::GetCurrentLevel() const
{
	return GetLevelByEnum(CurrentLevel);
}

ULevelDataAsset* ASimulationActor::GetLevelByEnum(ESimulationLevel LevelEnum) const
{
	for (ULevelDataAsset* Level : Levels)
	{
		if (Level && Level->LevelEnum == LevelEnum)
		{
			return Level;
		}
	}
	return nullptr;
}

void ASimulationActor::OnGameEvent(EGameEvent Event, void* EventData)
{
	//update hud
	/*if(PlayerReference){
		if(APlayerControllerBase *casted = Cast<APlayerControllerBase>(PlayerReference)){
			casted->UpdatePlayerWidget(Event);
		}
	}*/


	UEnum* EnumPtr = StaticEnum<EGameEvent>();
	FString EventName = EnumPtr ? EnumPtr->GetNameStringByValue(static_cast<int64>(Event)) : FString::Printf(TEXT("%d"), static_cast<int32>(Event));
	UE_LOG(LogTemp, Warning, TEXT("Game Event Received: %s"), *EventName);
	
	ULevelDataAsset* CurrentLevelAsset = GetCurrentLevel();
	if (!CurrentLevelAsset || CurrentTaskIndex < 0 || CurrentTaskIndex >= CurrentLevelAsset->Tasks.Num())
	{
		return;
	}

	FTaskInfo& CurrentTask = CurrentLevelAsset->Tasks[CurrentTaskIndex]; // Get reference to actual task

	if (CurrentTask.TaskEvent == Event)
	{
		// Mark the task as completed in the data asset
		CurrentTask.bIsCompleted = true;

		// Update UI to reflect completion
		if (TaskUIActor)
		{
			if (UTaskWidget* TaskWidget = TaskUIActor->GetWidget<UTaskWidget>())
			{
				TaskWidget->SetCompleted(CurrentTaskIndex);
			}
		}

		// Move to next task
		CurrentTaskIndex++;

		// Check if we've completed all tasks in this level
		if (CurrentTaskIndex >= CurrentLevelAsset->GetTaskCount())
		{
			// All tasks in level completed
			// Could trigger level completion or stay on last task
			CurrentTaskIndex = FMath::Max(0, CurrentLevelAsset->GetTaskCount() - 1); // Stay on last task, with minimum of 0
			CompleteCurrentLevel();
		}
	}
}

void ASimulationActor::CompleteCurrentLevel()
{
	// set current Level Asset as Completed
	if (ULevelDataAsset* CurrentLevelAsset = GetCurrentLevel())
	{
		CurrentLevelAsset->bIsCompleted = true;
	}
	
	// Show the task finished UI
	if (TaskFinishedUIActor && PlayerReference)
	{
		// Position it 0.5 meters (50 units) in front of the TaskUIActor
		if (TaskUIActor)
		{
			FVector TaskUILocation = TaskUIActor->GetActorLocation();
			FVector PlayerLocation = PlayerReference->GetActorLocation();
			
			// Calculate direction from TaskUI to Player
			FVector DirectionToPlayer = (PlayerLocation - TaskUILocation).GetSafeNormal();
			
			// Position TaskFinishedUI 0.5 meters closer to player than TaskUI
			FVector FinishedUILocation = TaskUILocation + (DirectionToPlayer * 300.0f); // 50 units = 0.5 meters
			
			TaskFinishedUIActor->SetActorLocation(FinishedUILocation);
		}
		
		// Make it visible
		TaskFinishedUIActor->SetActorHiddenInGame(false);
	}
	
	// Hide Tasks List
	if (TaskUIActor)
	{
		TaskUIActor->SetActorHiddenInGame(true);
	}
}

void ASimulationActor::LoadLevel(ESimulationLevel LevelEnum)
{
	DebugHelper::logMessage("ASimulationActor::LoadLevel Trigger!");

	// Check if we're already transitioning, if so, don't start a new transition
	if (bIsTransitioning)
	{
		UE_LOG(LogTemp, Warning, TEXT("LoadLevel: Currently in transition, ignoring new load request"));
		return;
	}

	if (LevelEnum != ESimulationLevel::ENone && GetLevelByEnum(LevelEnum) != nullptr)
	{
		// Store the pending level
		PendingLevelEnum = LevelEnum;

		// Begin the loading process
		BeginLoad(LevelEnum);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("LoadLevel: Invalid level enum or level data not found"));
	}
}

void ASimulationActor::BeginLoad(ESimulationLevel LevelEnum)
{
	// Set the material's Progress parameter to 0 to start the transition
	if (MatrixTransitionMaterial)
	{
		MatrixTransitionMaterial->SetScalarParameterValue("Progress", 0.0f);
		CurrentTransitionProgress = 0.0f;
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("BeginLoad: MatrixTransitionMaterial is null!"));
		// If no material, skip transition and go directly to loading
		// Make sure transition state is reset
		bIsTransitioning = false;
		bIsFirstHalfTransition = false;
		CurrentTransitionProgress = 0.0f;
		TargetProgress = 0.0f;
		Load(PendingLevelEnum);
		return;
	}

	// Start smooth transition from 0.0 to 0.5
	TargetProgress = 0.5f;
	bIsTransitioning = true;
	bIsFirstHalfTransition = true;
}

void ASimulationActor::Load(ESimulationLevel LevelEnum)
{
	// Set the current level and reset to first task
	CurrentLevel = LevelEnum;
	CurrentTaskIndex = 0;

	// Get current level asset
	ULevelDataAsset* CurrentLevelAsset = GetCurrentLevel();
	if (!CurrentLevelAsset)
	{
		UE_LOG(LogTemp, Warning, TEXT("Load: Failed to get level data asset"));
		return;
	}

	// Reset ALL tasks to incomplete state (fresh start)
	for (FTaskInfo& Task : CurrentLevelAsset->Tasks)
	{
		Task.bIsCompleted = false;
	}

	// Clear all operations (robot state, player state, etc.) //very important, do not change.
	ClearOperationOnLevelChangeOrReload();

	// Switch streaming levels (hide Intro, show Simple)
	if (UWorld* World = GetWorld())
	{
		if (ULevelStreaming* SimpleLevel = UGameplayStatics::GetStreamingLevel(World, TEXT("Simple")))
		{
			SimpleLevel->SetShouldBeVisible(true);
		}

		if (ULevelStreaming* IntroLevel = UGameplayStatics::GetStreamingLevel(World, TEXT("Intro")))
		{
			IntroLevel->SetShouldBeVisible(false);
		}
	}

	// Reset and start level actors from the beginning
	StartLevelActorsFrom(CurrentLevelAsset);

	// Update and show Task UI
	if (TaskUIActor)
	{
		if (UTaskWidget* TaskWidget = TaskUIActor->GetWidget<UTaskWidget>())
		{
			TaskWidget->LoadTasks(CurrentLevelAsset);
		}
		TaskUIActor->SetActorHiddenInGame(false);
	}

	// Hide other UI elements
	if (TaskFinishedUIActor)
	{
		TaskFinishedUIActor->SetActorHiddenInGame(true);
	}

	if (MainMenuActor)
	{
		MainMenuActor->SetActorHiddenInGame(true);
	}

	// Teleport player to start position
	if (PlayerReference)
	{
		FVector CurrentLocation = PlayerReference->GetActorLocation();
		FVector NewPosition = FVector(PlayerStartPosition.X, PlayerStartPosition.Y, CurrentLocation.Z);
		PlayerReference->SetActorLocation(NewPosition);

		// Make player look at Tasks UI
		if (TaskUIActor)
		{
			FVector PlayerLocation = NewPosition;
			FVector TasksUILocation = TaskUIActor->GetActorLocation();
			FVector DirectionToTasksUI = (TasksUILocation - PlayerLocation).GetSafeNormal();
			
			FRotator LookAtRotation = DirectionToTasksUI.Rotation();
			FRotator CurrentRotation = PlayerReference->GetActorRotation();
			LookAtRotation.Pitch = CurrentRotation.Pitch;
			LookAtRotation.Roll = 0.0f;

			PlayerReference->SetActorRotation(LookAtRotation);
		}
	}

	// Notify of level change
	OnGameEvent(EGameEvent::EChangeLevel, nullptr);
	
	DebugHelper::logMessage("ASimulationActor LOAD LEVEL Complete!");
}

void ASimulationActor::EndLoad()
{
	// Notify that loading is complete
	OnGameEvent(EGameEvent::EStartNewLevel);

	// Reset the pending level enum
	PendingLevelEnum = ESimulationLevel::ENone;
}

// Simplified StartNextLevel - just advances to the next level enum
void ASimulationActor::StartNextLevel()
{
	// Get the next level enum value
	ESimulationLevel NextLevel = static_cast<ESimulationLevel>(static_cast<int32>(CurrentLevel) + 1);
	
	// Validate that the next level exists
	if (GetLevelByEnum(NextLevel) == nullptr)
	{
		UE_LOG(LogTemp, Warning, TEXT("StartNextLevel: No next level found after %d"), static_cast<int32>(CurrentLevel));
		return;
	}
	
	// Just load the next level - LoadLevel handles everything
	LoadLevel(NextLevel);
	
	UE_LOG(LogTemp, Log, TEXT("ReturnToHomeMenu: Player reset to safe spot %s"), *PlayerHomePosition.ToString());
}

void ASimulationActor::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	// Clean up the material instance to prevent potential crashes
	MatrixTransitionMaterial = nullptr;

	// Also stop any ongoing transitions to prevent infinite loops
	bIsTransitioning = false;
	bWaitingForInitialDelay = false;

	// Clear the singleton instance if this is the active instance
	if (SimulationActorInstance == this)
	{
		SimulationActorInstance = nullptr;
	}

	Super::EndPlay(EndPlayReason);
}

// Starts or restarts level from data asset
void ASimulationActor::StartLevelActorsFrom(ULevelDataAsset* Asset)
{
	if (Asset && RobotReference)
	{
		levelBasedActorManager.StartLevel(Asset, RobotReference->GetActorLocation(), GetWorld());
	}
}

// Dramatically simplified RestartCurrentLevel
void ASimulationActor::RestartCurrentLevel()
{
	// Just reload the current level - LoadLevel handles everything
	LoadLevel(CurrentLevel);
	
	UE_LOG(LogTemp, Log, TEXT("Restarted Level: %d"), static_cast<int32>(CurrentLevel));
}

void ASimulationActor::ClearOperationOnLevelChangeOrReload()
{
	if (RobotReference)
	{
		RobotReference->ClearRobotOnLevelSwitch();	
	}
	
	// Reset player
	if (PlayerReference)
	{
		if (APlayerControllerBase* Casted = Cast<APlayerControllerBase>(PlayerReference))
		{
			Casted->ClearPlayerOnLevelSwitch();
		}
	}
}

void ASimulationActor::TogglePause()
{
	bIsPaused = !bIsPaused;

	if (bIsPaused)
	{
		// Show the pause menu if it exists
		if (PauseMenuActor)
		{
			if (PlayerReference)
			{
				// Teleport pause menu in front of player
				FVector PlayerLocation = PlayerReference->GetActorLocation();
				FVector PlayerForward = PlayerReference->GetActorForwardVector();
				
				// Place menu 300 units (3 meters) in front of player at eye level
				FVector MenuLocation = PlayerLocation + (PlayerForward * 300.0f);
				MenuLocation.Z = 175.0f;
				PauseMenuActor->SetActorLocation(MenuLocation);

				// Add Post Process Effects
				APlayerControllerBase* PlayerBase = Cast<APlayerControllerBase>(PlayerReference);
				if (PlayerBase && PlayerBase->CameraRef)
				{
					PlayerBase->CameraRef->PostProcessSettings.bOverride_VignetteIntensity = true;
					PlayerBase->CameraRef->PostProcessSettings.VignetteIntensity = 0.33f; // Darken edges
			
					PlayerBase->CameraRef->PostProcessSettings.bOverride_ColorSaturation = true;
					PlayerBase->CameraRef->PostProcessSettings.ColorSaturation = FVector4(0.66f, 0.66f, 0.66f, 1.0f); // Desaturate
				}
			}
			
			PauseMenuActor->SetActorHiddenInGame(false);
		}

		// Set global time dilation to 0 to pause gameplay
		if (UWorld* World = GetWorld())
		{
			World->GetWorldSettings()->SetTimeDilation(0.0f);
		}
	}
	else
	{
		// Resuming the game
		
		// Restore global time dilation to normal
		if (UWorld* World = GetWorld())
		{
			World->GetWorldSettings()->SetTimeDilation(1.0f);
		}

		// Hide the pause menu if it exists
		if (PauseMenuActor)
		{
			PauseMenuActor->SetActorHiddenInGame(true);
		}

		APlayerControllerBase* PlayerBase = Cast<APlayerControllerBase>(PlayerReference);
		if (PlayerBase && PlayerBase->CameraRef)
		{
			// Disable post process overrides
			PlayerBase->CameraRef->PostProcessSettings.bOverride_VignetteIntensity = false;
			PlayerBase->CameraRef->PostProcessSettings.bOverride_ColorSaturation = false;
		}
	}
}

void ASimulationActor::ReturnToHomeMenu()
{
	//remove pause when exit
	if(IsGamePaused()){
		TogglePause();
	}
	
	// 1. Hide all gameplay UI elements
	if (TaskUIActor) TaskUIActor->SetActorHiddenInGame(true);
	if (TaskFinishedUIActor) TaskFinishedUIActor->SetActorHiddenInGame(true);
	if (PauseMenuActor) PauseMenuActor->SetActorHiddenInGame(true);
		
	// 2. Show the main menu
	if (MainMenuActor)
	{
		MainMenuActor->SetActorHiddenInGame(false);
	}

	if (MainMenuActor)
	{
		MainMenuActor->SetActorHiddenInGame(false);
		if (UMenuWidget* MenuWidget = MainMenuActor->GetWidget<UMenuWidget>())
		{
			MenuWidget->LoadLevels(Levels);
		}
	}

	// 3. Switch levels (Hide Simple, Show Intro)
	if (UWorld* World = GetWorld())
	{
		// Hide Simple level
		if (ULevelStreaming* SimpleLevel = UGameplayStatics::GetStreamingLevel(World, TEXT("Simple")))
		{
			SimpleLevel->SetShouldBeVisible(false);
		}

		// Show Intro level
		if (ULevelStreaming* IntroLevel = UGameplayStatics::GetStreamingLevel(World, TEXT("Intro")))
		{
			IntroLevel->SetShouldBeVisible(true);
		}
	}

	// 4. Reset robot to idle state
	if (RobotReference)
	{
		RobotReference->SetState(ERobotState::StateIdle);
		// Optional: You might want to teleport the robot away or to a specific spot too
	}

	// 5. Teleport Player to the Safe Spot
	if (PlayerReference)
	{
		// Use the hardcoded safe spot coordinates
		PlayerReference->SetActorLocation(PlayerHomePosition);

		// Make player look at the Main Menu
		if (MainMenuActor)
		{
			FVector MenuLoc = MainMenuActor->GetActorLocation();
			
			// Calculate direction from the Safe Spot to the Menu
			FVector DirectionToMenu = (MenuLoc - PlayerHomePosition).GetSafeNormal();
			
			FRotator LookAtMenu = DirectionToMenu.Rotation();
			LookAtMenu.Pitch = 0.0f; // Keep head level
			LookAtMenu.Roll = 0.0f;
			
			PlayerReference->SetActorRotation(LookAtMenu);
		}
		else
		{
			// Fallback rotation if menu is missing (e.g., face Forward X)
			PlayerReference->SetActorRotation(FRotator(0, 0, 0));
		}
	}
	ClearOperationOnLevelChangeOrReload();

	// 6. Clear current level and task index
	CurrentLevel = ESimulationLevel::ENone;
	CurrentTaskIndex = 0;

	// 7. Trigger level change event
	OnGameEvent(EGameEvent::EChangeLevel);

	UE_LOG(LogTemp, Log, TEXT("PerformReturnToHomeInternal: Player reset to safe spot %s"), *PlayerHomePosition.ToString());
}