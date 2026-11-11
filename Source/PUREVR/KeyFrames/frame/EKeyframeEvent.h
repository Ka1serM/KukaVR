#pragma once


#include "CoreMinimal.h"

UENUM(Blueprintable)
enum class EKeyframeEvent : uint8
{
    KeyframeAdded,
    KeyframeMoved,
    KeyframeRemoved
};