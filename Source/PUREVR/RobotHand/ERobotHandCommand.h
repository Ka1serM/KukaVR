#pragma once


#include "CoreMinimal.h"

UENUM(Blueprintable)
enum class ERobotHandCommand : uint8
{
    ENone,
    EPickupItem,
    EDropItem
};