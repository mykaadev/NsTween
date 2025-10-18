// Copyright (C) 2025 nulled.softworks. All rights reserved.

#include "NsTween.h"
#include "Engine/Engine.h"
#include "Interfaces/IEasingCurve.h"
#include "Interfaces/ITweenValue.h"
#include "Math/UnrealMathUtility.h"
#include "Utils/NsTweenProfiling.h"

/**
 * The non-templated Play overload wires an explicit specification and strategy factory into the builder.
 * Everything else funnels through this helper so we only have one place that knows how to seed the shared state.
 */
FNsTweenBuilder FNsTween::Play(FNsTweenSpec Spec, TFunction<TSharedPtr<ITweenValue>()> StrategyFactory)
{
    NSTWEEN_SCOPE_CYCLE_COUNTER("NsTween::Play");
    const TSharedPtr<FNsTweenBuilder::FState> State = MakeShared<FNsTweenBuilder::FState>();
    State->Spec = MoveTemp(Spec);
    State->StrategyFactory = MoveTemp(StrategyFactory);
    return FNsTweenBuilder(State);
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
}

bool FNsTween::Tick(float DeltaSeconds)
{
    NSTWEEN_SCOPE_CYCLE_COUNTER("NsTween::Tick");
    // Bail out immediately if the tween is already completed, paused, or missing runtime pieces.
    if (!bActive || bPaused)
    {
        return bActive;
    }

    ITweenValue* const StrategyPtr = Strategy.Get();
    IEasingCurve* const EasingPtr = Easing.Get();
    if (StrategyPtr == nullptr || EasingPtr == nullptr)
    {
        bActive = false;
        return false;
    }

    // Lazily initialize the strategy the first time we tick so creation happens on the game thread.
    if (!bInitialized)
    {
        StrategyPtr->Initialize();
        if (Spec.Direction == ENsTweenDirection::Backward)
        {
            StrategyPtr->ApplyFinal();
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

        if (bPlayingForward)
        {
            float MaxAdvance = TargetBoundary - CycleTime;
            if (MaxAdvance <= SMALL_NUMBER)
            {
                MaxAdvance = 0.f;
            }
            if (RemainingTime <= MaxAdvance)
            {
                CycleTime += RemainingTime;
                Apply(CycleTime);
                break;
            }

            CycleTime = TargetBoundary;
            Apply(CycleTime);
            RemainingTime -= MaxAdvance;
        }
        else
        {
            float MaxRetreat = CycleTime - TargetBoundary;
            if (MaxRetreat <= SMALL_NUMBER)
            {
                MaxRetreat = 0.f;
            }
            if (RemainingTime <= MaxRetreat)
            {
                CycleTime -= RemainingTime;
                Apply(CycleTime);
                break;
            }

            CycleTime = TargetBoundary;
            Apply(CycleTime);
            RemainingTime -= MaxRetreat;
        }

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

    if (bApplyFinal && Strategy.IsValid())
    {
        Strategy->ApplyFinal();
    }

    Spec.OnComplete.ExecuteIfBound();

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
    ITweenValue* const StrategyPtr = Strategy.Get();
    IEasingCurve* const EasingPtr = Easing.Get();
    if (!StrategyPtr || !EasingPtr)
    {
        return;
    }

    const float DurationSeconds = Spec.DurationSeconds;
    float LinearAlpha = (DurationSeconds > SMALL_NUMBER) ? (InCycleTime / DurationSeconds) : 0.f;
    LinearAlpha = FMath::Clamp(LinearAlpha, 0.f, 1.f);
    const float EasedAlpha = EasingPtr->Evaluate(LinearAlpha);

    StrategyPtr->Apply(EasedAlpha);
    Spec.OnUpdate.ExecuteIfBound(EasedAlpha);
}

bool FNsTween::HandleBoundary(float& RemainingTime)
{
    NSTWEEN_SCOPE_CYCLE_COUNTER("NsTween::HandleBoundary");
    ITweenValue* const StrategyPtr = Strategy.Get();

    if (Spec.WrapMode == ENsTweenWrapMode::Once)
    {
        Spec.OnComplete.ExecuteIfBound();
        if (StrategyPtr)
        {
            StrategyPtr->ApplyFinal();
        }
        bActive = false;
        return false;
    }

    if (Spec.WrapMode == ENsTweenWrapMode::Loop)
    {
        ++CompletedCycles;
        Spec.OnLoop.ExecuteIfBound();

        if (Spec.LoopCount > 0 && CompletedCycles >= Spec.LoopCount)
        {
            Spec.OnComplete.ExecuteIfBound();
            if (StrategyPtr)
            {
                StrategyPtr->ApplyFinal();
            }
            bActive = false;
            return false;
        }

        CycleTime = 0.f;
        bPlayingForward = (Spec.Direction != ENsTweenDirection::Backward);
    }
    else if (Spec.WrapMode == ENsTweenWrapMode::PingPong)
    {
        bPlayingForward = !bPlayingForward;
        Spec.OnPingPong.ExecuteIfBound();

        if (!bPlayingForward)
        {
            ++CompletedPingPongPairs;
        }
        else if (Spec.LoopCount > 0 && CompletedPingPongPairs >= Spec.LoopCount)
        {
            Spec.OnComplete.ExecuteIfBound();
            if (StrategyPtr)
            {
                StrategyPtr->ApplyFinal();
            }
            bActive = false;
            return false;
        }

        CycleTime = bPlayingForward ? 0.f : Spec.DurationSeconds;
    }

    // Consume whatever time is left using the new direction or wrap mode.
    if (RemainingTime < 0.f)
    {
        RemainingTime = 0.f;
    }
    return true;
}

