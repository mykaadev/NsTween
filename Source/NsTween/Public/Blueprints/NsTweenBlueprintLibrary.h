// Copyright (C) 2024 mykaa. All rights reserved.

#pragma once

#include "Classes/NsTweenEasing.h"
#include "NsTweenBlueprintLibrary.generated.h"

/**
 * Tween Blueprint Function Library
 */
UCLASS()
class NSTWEEN_API UNsTweenBlueprintLibrary : public UBlueprintFunctionLibrary
{
    GENERATED_BODY()

// Functions
public:

    /**
     * Ease with overriding parameters
     * @param t the percent complete, 0-1
     * @param EaseType The easing function to interpolate with
     */
    UFUNCTION(BlueprintPure, Category = "Tween")
    static float Ease(float t, ENsTweenEase EaseType);

    /**
     * Ease with overriding parameters that affect Elastic, Back, Stepped, and Smoothstep. 0 means no override
     * @param t the percent complete, 0-1
     * @param EaseType The easing function to interpolate with
     * @param Param1 Elastic: Amplitude (1.0) / Back: Overshoot (1.70158) / Stepped: Steps (10) / Smoothstep: x0 (0)
     * @param Param2 Elastic: Period (0.2) / Smoothstep: x1 (1)
     */
    UFUNCTION(BlueprintPure, Category = "Tween")
    static float EaseWithParams(float t, ENsTweenEase EaseType, float Param1 = 0, float Param2 = 0);

    /** Make sure there are at least these many of each type of tween available in memory. Call this only once, probably in a GameInstance blueprint, if you need more initial memory reserved on game startup. */
    UFUNCTION(BlueprintCallable, Category = "Tween|Utility")
    static void EnsureTweenCapacity(int NumFloatTweens = 75, int NumVectorTweens = 50, int NumVector2DTweens = 50, int NumQuatTweens = 10);
};
