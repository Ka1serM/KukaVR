#pragma once

/**
* Enum for all game events that can occur in the simulation
 */
UENUM(Blueprintable)
enum class EGameEvent : uint8
{
	ENone,
    
	// Keyframe events
	EAddKeyframe,
	EMoveKeyframe,
	ERemoveKeyframe,
	ESelectKeyframe,
    
	// Handle movement events
	EMoveHandle,
    
	// Robot state events
	ERobotStateChanged,
	ERobotPlayAnimation,
	ERobotStopAnimation,
	ERobotEditAnimation,

	// Level loading events
	EChangeLevel,
	EStartNewLevel,

	//Box drop and pickup
	ERobotPickupBox,
	ERobotDropBox
};