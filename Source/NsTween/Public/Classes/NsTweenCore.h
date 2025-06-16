// Copyright (C) 2024 mykaa. All rights reserved.

#pragma once

#include "NsTweenEasing.h"
#include "NsTweenInstance.h"
#include "NsTweenManager.h"

NSTWEEN_API DECLARE_LOG_CATEGORY_EXTERN(LogNsTween, Log, All)

/**
 * Core Tween class
 */
class NSTWEEN_API NsTweenCore
{
// Functions
public:

    /** Initialize */
    static void Initialize();

    /** Deinitialize */
    static void Deinitialize();

    /** Ensure there are at least this many tweens in the recycle pool. Call this at game startup to increase your initial capacity for each type of tween, if you know you will be needing more and don't want to allocate memory during the game */
    static void EnsureCapacity(int NumFloatTweens, int NumVectorTweens, int NumVector2DTweens, int NumQuatTweens);

    /** Add more tweens to the recycle pool. Call this at game startup to increase your initial capacity if you know you will be needing more and don't want to allocate memory during the game */
    static void EnsureCapacity(int NumTweens);

    /** Update */
    static void Update(float UnscaledDeltaSeconds, float DilatedDeltaSeconds, bool bIsGamePaused);

    /** Clear active tweens */
    static void ClearActiveTweens();

    /** Compare the current reserved memory for tweens against the initial capacity, to tell the developer if initial capacity needs to be increased */
    static int CheckTweenCapacity();

    /** Convenience function for UNsTweenEasing::Ease() */
    static float Ease(float T, ENsTweenEase EaseType);

    /** Play Tween - Float */
    static NsTweenInstanceFloat* Play(float Start, float End, float DurationSecs, ENsTweenEase EaseType, TFunction<void(float)> OnUpdate);

    /** Play Tween - Vector */
    static NsTweenInstanceVector* Play(const FVector& Start, const FVector& End, float DurationSecs, ENsTweenEase EaseType, TFunction<void(FVector)> OnUpdate);

    /** Play Tween - Vector2D */
    static NsTweenInstanceVector2D* Play(FVector2D Start, FVector2D End, float DurationSecs, ENsTweenEase EaseType, TFunction<void(FVector2D)> OnUpdate);

    /** Play Tween - Quat */
    static NsTweenInstanceQuat* Play(const FQuat& Start, const FQuat& End, float DurationSecs, ENsTweenEase EaseType, TFunction<void(FQuat)> OnUpdate);

// Variables
private:

    /** Float tween manager */
    static NsTweenManager<NsTweenInstanceFloat>* FloatTweenManager;

    /** Vector tween manager */
    static NsTweenManager<NsTweenInstanceVector>* VectorTweenManager;

    /** Vector2D tween manager */
    static NsTweenManager<NsTweenInstanceVector2D>* Vector2DTweenManager;

    /** Quat tween manager */
    static NsTweenManager<NsTweenInstanceQuat>* QuatTweenManager;

    /** Reserved floats */
    static int NumReservedFloat;

    /** Reserved Vector */
    static int NumReservedVector;

    /** Reserved Vector2D */
    static int NumReservedVector2D;

    /** Reserved Quat */
    static int NumReservedQuat;
};
