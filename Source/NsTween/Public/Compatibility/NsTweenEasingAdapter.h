// Copyright (C) 2024 mykaa. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "TweenEnums.h"

UE_DEPRECATED(5.3, "Use ENovaEasingPreset instead.")
static inline ENovaEasingPreset ConvertLegacyEasing(uint8 LegacyValue)
{
    switch (LegacyValue)
    {
    default:
    case 0: return ENovaEasingPreset::Linear;
    case 1: return ENovaEasingPreset::EaseInSine;
    case 2: return ENovaEasingPreset::EaseOutSine;
    case 3: return ENovaEasingPreset::EaseInOutSine;
    case 4: return ENovaEasingPreset::EaseInQuad;
    case 5: return ENovaEasingPreset::EaseOutQuad;
    case 6: return ENovaEasingPreset::EaseInOutQuad;
    case 7: return ENovaEasingPreset::EaseInCubic;
    case 8: return ENovaEasingPreset::EaseOutCubic;
    case 9: return ENovaEasingPreset::EaseInOutCubic;
    case 10: return ENovaEasingPreset::EaseInExpo;
    case 11: return ENovaEasingPreset::EaseOutExpo;
    case 12: return ENovaEasingPreset::EaseInOutExpo;
    }
}

