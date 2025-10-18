// Copyright (C) 2025 nulled.softworks. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "NsTweenBuilder.h"
#include "NsTweenTypeLibrary.h"
#include "Templates/NsTweenCallbackStrategy.h"
#include "Templates/NsTweenInterpolator.h"

class ITweenValue;
class IEasingCurve;
class UNsTweenSubsystem;
class UCurveFloat;

struct NSTWEEN_API FNsTween
{
public:
    /** Constructs a tween using the provided runtime data. */
    FNsTween(const FNsTweenHandle& InHandle, FNsTweenSpec InSpec, TSharedPtr<ITweenValue> InStrategy, TSharedPtr<IEasingCurve> InEasing);

    /** Advances the tween state by the provided delta time. */
    bool Tick(float DeltaSeconds);

    /** Cancels the tween and optionally applies the final value. */
    void Cancel(bool bApplyFinal);

    /** Sets whether the tween is currently paused. */
    void SetPaused(bool bInPaused);

    /** Returns true when the tween is still actively running. */
    bool IsActive() const { return bActive; }

    /** Returns the handle that uniquely identifies this tween. */
    const FNsTweenHandle& GetHandle() const { return Handle; }

    /** Generic convenience wrapper that forwards to the correct typed overload. */
    template <typename TCallable, typename TType>
    static FNsTweenBuilder Play(const TType& StartValue, const TType& EndValue, float DurationSeconds, ENsTweenEase Ease, TCallable&& Update)
    {
        TFunction<void(const TType&)> Fn(Forward<TCallable>(Update));
        return BuildT<TType>(StartValue, EndValue, DurationSeconds, Ease, MoveTemp(Fn));
    }

    /** Constructs a builder using explicit tween specification data. */
    static FNsTweenBuilder Play(FNsTweenSpec Spec, TFunction<TSharedPtr<ITweenValue>()> StrategyFactory);

private:
    /** Applies the tween update for the given cycle time. */
    void Apply(float CycleTime);

    /** Handles wrap-mode transitions, returning false when the tween completes. */
    bool HandleBoundary(float& RemainingTime);

    /** Default typed builder that wires up the callback strategy for the tween. */
    template <typename TType>
    static FNsTweenBuilder BuildT(const TType& StartValue, const TType& EndValue, float DurationSeconds, ENsTweenEase Ease, TFunction<void(const TType&)> Update)
    {
        FNsTweenSpec Spec;
        Spec.DurationSeconds = FMath::Max(DurationSeconds, 0.f);
        Spec.DelaySeconds = 0.f;
        Spec.TimeScale = 1.f;
        Spec.WrapMode = ENsTweenWrapMode::Once;
        Spec.LoopCount = 0;
        Spec.Direction = ENsTweenDirection::Forward;
        Spec.EasingPreset = Ease;

        TFunction<void(const TType&)> Owned = MoveTemp(Update);
        return Play(
            MoveTemp(Spec),
            [StartValue, EndValue, Owned = MoveTemp(Owned)]() mutable -> TSharedPtr<class ITweenValue>
            {
                return MakeNsTweenCallbackStrategy<TType>(StartValue, EndValue, MoveTemp(Owned));
            });
    }

private:
    /** The handle assigned to this tween. */
    FNsTweenHandle Handle;

    /** Specification describing how the tween behaves. */
    FNsTweenSpec Spec;

    /** Strategy responsible for applying value updates. */
    TSharedPtr<ITweenValue> Strategy;

    /** Easing curve used to transform normalized time. */
    TSharedPtr<IEasingCurve> Easing;

    /** Cached raw pointer for the strategy to avoid smart pointer overhead on hot paths. */
    ITweenValue* StrategyPtr = nullptr;

    /** Cached raw pointer for the easing curve to avoid smart pointer overhead on hot paths. */
    IEasingCurve* EasingPtr = nullptr;

    /** Cached loop limit so we do not repeatedly access the spec in hot paths. */
    int32 LoopLimit = 0;

    /** Indicates whether the tween has a finite loop limit. */
    bool bHasLoopLimit = false;

    /** Cached wrap mode flags so boundary handling avoids repeated enum comparisons. */
    bool bWrapOnce = false;
    bool bWrapLoop = false;
    bool bWrapPingPong = false;

    /** Remaining delay before the tween starts. */
    float DelayRemaining = 0.0f;

    /** Cached duration to avoid re-evaluating clamped values every tick. */
    float Duration = 0.0f;

    /** Reciprocal of the cached duration for fast alpha calculation. */
    float DurationInverse = 1.0f;

    /** Cached time scale to avoid repeated clamping. */
    float TimeScale = 1.0f;

    /** Accumulated time within the current tween cycle. */
    float CycleTime = 0.0f;

    /** Number of completed cycles for looping tweens. */
    int32 CompletedCycles = 0;

    /** Number of completed ping-pong direction pairs. */
    int32 CompletedPingPongPairs = 0;

    /** Tracks whether the tween is still active. */
    bool bActive = true;

    /** Tracks whether the tween is currently paused. */
    bool bPaused = false;

    /** Tracks whether the tween strategy has been initialized. */
    bool bInitialized = false;

    /** Tracks whether the tween is currently playing forward. */
    bool bPlayingForward = true;

    /** Cached delegate state to eliminate repeated IsBound checks in hot paths. */
    bool bHasOnUpdate = false;

    /** Cached delegate state to eliminate repeated IsBound checks in hot paths. */
    bool bHasOnComplete = false;

    /** Cached delegate state to eliminate repeated IsBound checks in hot paths. */
    bool bHasOnLoop = false;

    /** Cached delegate state to eliminate repeated IsBound checks in hot paths. */
    bool bHasOnPingPong = false;
};

