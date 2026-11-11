#pragma once


#include "CoreMinimal.h"

UENUM(Blueprintable)
enum class ERobotState : uint8
{
    StateIdle,
    StatePlayingAnimation,
    StateEditAnimation,
    StateFollow,
    StateNone
};