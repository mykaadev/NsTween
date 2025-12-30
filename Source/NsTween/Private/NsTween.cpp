// Copyright (C) 2025 nulled.softworks. All rights reserved.

#include "NsTween.h"
#include "Engine/Engine.h"
#include "Interfaces/IEasingCurve.h"
#include "Interfaces/ITweenValue.h"
#include "Math/UnrealMathUtility.h"
#include "Utils/NsTweenProfiling.h"

FNsTweenBuilder FNsTween::Play(FNsTweenSpec Spec, TFunction<TSharedPtr<ITweenValue>()> StrategyFactory)
{
    NSTWEEN_SCOPE_CYCLE_COUNTER("NsTween::Play");
    return FNsTweenBuilder(MoveTemp(Spec), MoveTemp(StrategyFactory));
}

FNsTween::FNsTween(const FNsTweenHandle& InHandle, FNsTweenSpec InSpec, TSharedPtr<ITweenValue> InStrategy, TSharedPtr<IEasingCurve> InEasing)
    : Handle(InHandle)
    , Spec(MoveTemp(InSpec))
    , Strategy(MoveTemp(InStrategy))
    , Easing(MoveTemp(InEasing))
{
    NSTWEEN_SCOPE_CYCLE_COUNTER("NsTween::Ctor");

    Spec.DelaySeconds = FMath::Max(0.f, Spec.DelaySeconds);
    Spec.DurationSeconds = FMath::Max(Spec.DurationSeconds, SMALL_NUMBER);
    Spec.TimeScale = FMath::Max(Spec.TimeScale, 0.f);
    Spec.LoopCount = FMath::Max(Spec.LoopCount, 0);

    DelayRemaining = Spec.DelaySeconds;
    CycleTime = (Spec.Direction == ENsTweenDirection::Forward) ? 0.f : Spec.DurationSeconds;
    bPlayingForward = (Spec.Direction != ENsTweenDirection::Backward);
    bStartBackward = (Spec.Direction == ENsTweenDirection::Backward);
}

bool FNsTween::Tick(float DeltaSeconds)
{
    NSTWEEN_SCOPE_CYCLE_COUNTER("NsTween::Tick");
    // Bail out immediately if the tween is already completed, paused, or missing runtime pieces.
    if (!bActive || bPaused || !Strategy.IsValid() || !Easing.IsValid())
    {
        bActive = bActive && Strategy.IsValid() && Easing.IsValid();
        return bActive;
    }

    if (Spec.bEnforceOwnerLifetime && !Spec.Owner.IsValid())
    {
        bActive = false;
        Strategy.Reset();
        Easing.Reset();
        return false;
    }

    // Lazily initialize the strategy the first time we tick so creation happens on the game thread.
    if (!bInitialized)
    {
        Strategy->Initialize();
        if (bStartBackward)
        {
            Strategy->ApplyFinal();
        }
        bInitialized = true;
    }

    // Respect the time scale so tweens can speed up or slow down deterministically.
    float ScaledDelta = DeltaSeconds * Spec.TimeScale;
    if (ScaledDelta <= SMALL_NUMBER)
    {
        return true;
    }

    // Consume any remaining delay before the tween starts ticking.
    if (DelayRemaining > SMALL_NUMBER)
    {
        if (ScaledDelta < DelayRemaining)
        {
            DelayRemaining -= ScaledDelta;
            return true;
        }

        ScaledDelta -= DelayRemaining;
        DelayRemaining = 0.f;
    }

    float RemainingTime = ScaledDelta;
    const float LocalDuration = Spec.DurationSeconds;

    while (RemainingTime > SMALL_NUMBER && bActive)
    {
        const float TargetBoundary = bPlayingForward ? LocalDuration : 0.f;
        const float TimeToBoundary = bPlayingForward ? (TargetBoundary - CycleTime) : (CycleTime - TargetBoundary);
        const float SafeTimeToBoundary = (TimeToBoundary > 0.f) ? TimeToBoundary : 0.f;

        if (RemainingTime <= SafeTimeToBoundary + KINDA_SMALL_NUMBER)
        {
            CycleTime += bPlayingForward ? RemainingTime : -RemainingTime;
            if (CycleTime < 0.f)
            {
                CycleTime = 0.f;
            }
            else if (CycleTime > LocalDuration)
            {
                CycleTime = LocalDuration;
            }
            Apply(CycleTime);
            break;
        }

        RemainingTime -= SafeTimeToBoundary;
        CycleTime = TargetBoundary;
        Apply(CycleTime);

        if (!HandleBoundary(RemainingTime))
        {
            return false;
        }
    }

    return bActive;
}

void FNsTween::Cancel(bool bApplyFinal)
{
    NSTWEEN_SCOPE_CYCLE_COUNTER("NsTween::Cancel");
    // Cancellation is idempotent; the first call decides whether to apply the final value.
    if (!bActive)
    {
        return;
    }

    if (bApplyFinal)
    {
        if (Strategy.IsValid())
        {
            Strategy->ApplyFinal();
        }
    }

    if (Spec.OnComplete.IsBound())
    {
        Spec.OnComplete.Execute();
    }

    bActive = false;
}

void FNsTween::SetPaused(bool bInPaused)
{
    NSTWEEN_SCOPE_CYCLE_COUNTER("NsTween::SetPaused");
    bPaused = bInPaused;
}

void FNsTween::Apply(float InCycleTime)
{
    NSTWEEN_SCOPE_CYCLE_COUNTER("NsTween::Apply");
    // Guard against misconfigured tweens that somehow lost their runtime strategy.
    if (!Strategy.IsValid() || !Easing.IsValid())
    {
        return;
    }

    const float Duration = Spec.DurationSeconds;
    const float LinearAlpha = FMath::Clamp(InCycleTime / Duration, 0.f, 1.f);
    const float EasedAlpha = Easing->Evaluate(LinearAlpha);

    Strategy->Apply(EasedAlpha);
    if (Spec.OnUpdate.IsBound())
    {
        Spec.OnUpdate.Execute(EasedAlpha);
    }
}

bool FNsTween::HandleBoundary(float& RemainingTime)
{
    NSTWEEN_SCOPE_CYCLE_COUNTER("NsTween::HandleBoundary");

    const ENsTweenWrapMode WrapMode = Spec.WrapMode;

    if (WrapMode == ENsTweenWrapMode::Once)
    {
        if (Strategy.IsValid())
        {
            Strategy->ApplyFinal();
        }

        if (Spec.OnComplete.IsBound())
        {
            Spec.OnComplete.Execute();
        }

        bActive = false;
        return false;
    }

    if (WrapMode == ENsTweenWrapMode::Loop)
    {
        ++CompletedCycles;
        if (Spec.OnLoop.IsBound())
        {
            Spec.OnLoop.Execute();
        }

        if (Spec.LoopCount > 0 && CompletedCycles >= Spec.LoopCount)
        {
            if (Strategy.IsValid())
            {
                Strategy->ApplyFinal();
            }

            if (Spec.OnComplete.IsBound())
            {
                Spec.OnComplete.Execute();
            }

            bActive = false;
            return false;
        }

        CycleTime = 0.f;
        bPlayingForward = (Spec.Direction != ENsTweenDirection::Backward);
    }
    else if (WrapMode == ENsTweenWrapMode::PingPong)
    {
        bPlayingForward = !bPlayingForward;
        if (Spec.OnPingPong.IsBound())
        {
            Spec.OnPingPong.Execute();
        }

        if (!bPlayingForward)
        {
            ++CompletedPingPongPairs;
        }
        else if (Spec.LoopCount > 0 && CompletedPingPongPairs >= Spec.LoopCount)
        {
            if (Strategy.IsValid())
            {
                Strategy->ApplyFinal();
            }

            if (Spec.OnComplete.IsBound())
            {
                Spec.OnComplete.Execute();
            }

            bActive = false;
            return false;
        }

        CycleTime = bPlayingForward ? 0.f : Spec.DurationSeconds;
    }

    if (RemainingTime < 0.f)
    {
        RemainingTime = 0.f;
    }
    return true;
}

