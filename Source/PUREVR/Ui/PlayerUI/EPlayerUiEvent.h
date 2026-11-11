#pragma once

/**
* Enum for all game events that can occur from the player 
 */
UENUM(Blueprintable)
enum class EPlayerUiEvent : uint8
{
    ENone,
    EPickedUpFrame,
    EDroppedFrame,
};