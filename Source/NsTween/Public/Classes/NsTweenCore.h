// Copyright (C) 2025 mykaadev. All rights reserved.

#pragma once

#include "Library/NsTweenTypeLibrary.h"
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
    static void EnsureCapacity(const int32 NumFloatTweens, const int32 NumVectorTweens, const int32 NumVector2DTweens, const int32 NumQuatTweens);

    /** Add more tweens to the recycle pool. Call this at game startup to increase your initial capacity if you know you will be needing more and don't want to allocate memory during the game */
    static void EnsureCapacity(const int32 NumTweens);

    /** Update */
    static void Update(float UnscaledDeltaSeconds, float DilatedDeltaSeconds, bool bIsGamePaused);

    /** Clear active tweens */
    static void ClearActiveTweens();

    /** Compare the current reserved memory for tweens against the initial capacity, to tell the developer if initial capacity needs to be increased */
    static int CheckTweenCapacity();

    /** Convenience function for UNsTweenEasing::Ease() */
    static float Ease(float T, ENsTweenEase EaseType);

    /** Log debug information for all active tweens */
    static void LogActiveTweens();

    /** Draw debug information on screen near each tween's debug actor */
    static void DrawActiveTweens(UWorld* World);

    /** Play Tween - Float */
    static FNsTweenInstanceFloat& Play(float Start, float End, float DurationSecs, ENsTweenEase EaseType, TFunction<void(float)> OnUpdate);

    /** Play Tween - Vector */
    static FNsTweenInstanceVector& Play(const FVector& Start, const FVector& End, float DurationSecs, ENsTweenEase EaseType, TFunction<void(FVector)> OnUpdate);

    /** Play Tween - Vector2D */
    static FNsTweenInstanceVector2D& Play(FVector2D Start, FVector2D End, float DurationSecs, ENsTweenEase EaseType, TFunction<void(FVector2D)> OnUpdate);

    /** Play Tween - Quat */
    static FNsTweenInstanceQuat& Play(const FQuat& Start, const FQuat& End, float DurationSecs, ENsTweenEase EaseType, TFunction<void(FQuat)> OnUpdate);

    /** For Each Manager */
    template <typename Func>
    FORCEINLINE static void ForEachManager(Func&& Fn)
    {
        Fn(FloatTweenManager);
        Fn(VectorTweenManager);
        Fn(Vector2DTweenManager);
        Fn(QuatTweenManager);
    }

    /** Ensure Manager capacity */
    template <typename ManagerType>
    FORCEINLINE static void EnsureManagerCapacity(ManagerType* Manager, int32 Num)
    {
        Manager->EnsureCapacity(Num);
    }

    template <typename ManagerType>
    FORCEINLINE static void CheckManagerCapacity(const ManagerType* Manager, const int32 NumReserved, const TCHAR* TypeName)
    {
        const int32 Current = Manager->GetCurrentCapacity();
        if (Current > NumReserved)
        {
            UE_LOG(LogNsTween,
                   Warning,
                   TEXT("Consider increasing initial capacity for %s tweens with NsTweenCore::EnsureCapacity(). %d were initially reserved, but now there are %d in memory."),
                   TypeName,
                   NumReserved,
                   Current);
        }
    }

    /** Play internal */
    template <typename TweenType, typename ValueType>
    FORCEINLINE static TweenType* PlayInternal(NsTweenManager<TweenType>* Manager,
                                        const ValueType& Start,
                                        const ValueType& End,
                                        float DurationSecs,
                                        ENsTweenEase EaseType,
                                        TFunction<void(ValueType)> OnUpdate)
    {
        TweenType* const NewTween = Manager->CreateTween();
        NewTween->Initialize(Start, End, MoveTemp(OnUpdate), DurationSecs, EaseType);
        return NewTween;
    }

// Variables
private:

    /** Float tween manager */
    static NsTweenManager<FNsTweenInstanceFloat>* FloatTweenManager;

    /** Vector tween manager */
    static NsTweenManager<FNsTweenInstanceVector>* VectorTweenManager;

    /** Vector2D tween manager */
    static NsTweenManager<FNsTweenInstanceVector2D>* Vector2DTweenManager;

    /** Quat tween manager */
    static NsTweenManager<FNsTweenInstanceQuat>* QuatTweenManager;

    /** Reserved floats */
    static int32 CurrentReservedFloat;

    /** Reserved Vector */
    static int32 CurrentReservedVector;

    /** Reserved Vector2D */
    static int32 CurrentReservedVector2D;

    /** Reserved Quat */
    static int32 CurrentReservedQuat;
};
