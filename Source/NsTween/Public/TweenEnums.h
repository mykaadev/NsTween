// Copyright (C) 2024 mykaa. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "TweenEnums.generated.h"

UENUM(BlueprintType)
enum class ENovaTweenDirection : uint8
{
    Forward,
    Backward
};

UENUM(BlueprintType)
enum class ENovaTweenWrapMode : uint8
{
    Once,
    Loop,
    PingPong
};

UENUM(BlueprintType)
enum class ENovaEasingPreset : uint8
{
    Linear,
    EaseInSine,
    EaseOutSine,
    EaseInOutSine,
    EaseInQuad,
    EaseOutQuad,
    EaseInOutQuad,
    EaseInCubic,
    EaseOutCubic,
    EaseInOutCubic,
    EaseInExpo,
    EaseOutExpo,
    EaseInOutExpo,
    CustomBezier,
    CurveAsset
};

