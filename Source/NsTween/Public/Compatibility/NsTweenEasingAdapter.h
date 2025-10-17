// Copyright (C) 2024 mykaa. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "TweenEnums.h"

UE_DEPRECATED(5.3, "Use ENsTweenEase instead.")
static inline ENsTweenEase ConvertLegacyEasing(uint8 LegacyValue)
{
    switch (LegacyValue)
    {
    default:
    case 0: return ENsTweenEase::Linear;
    case 1: return ENsTweenEase::EaseInSine;
    case 2: return ENsTweenEase::EaseOutSine;
    case 3: return ENsTweenEase::EaseInOutSine;
    case 4: return ENsTweenEase::EaseInQuad;
    case 5: return ENsTweenEase::EaseOutQuad;
    case 6: return ENsTweenEase::EaseInOutQuad;
    case 7: return ENsTweenEase::EaseInCubic;
    case 8: return ENsTweenEase::EaseOutCubic;
    case 9: return ENsTweenEase::EaseInOutCubic;
    case 10: return ENsTweenEase::EaseInExpo;
    case 11: return ENsTweenEase::EaseOutExpo;
    case 12: return ENsTweenEase::EaseInOutExpo;
    }
}

