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
    DelayRemaining = FMath::Max(0.f, Spec.DelaySeconds);
    Duration = FMath::Max(Spec.DurationSeconds, SMALL_NUMBER);
    DurationInverse = 1.f / Duration;
    TimeScale = FMath::Max(Spec.TimeScale, 0.f);
    Spec.DelaySeconds = DelayRemaining;
    Spec.DurationSeconds = Duration;
    Spec.TimeScale = TimeScale;
    Spec.LoopCount = FMath::Max(Spec.LoopCount, 0);
    LoopLimit = Spec.LoopCount;
    bHasLoopLimit = LoopLimit > 0;
    bWrapOnce = Spec.WrapMode == ENsTweenWrapMode::Once;
    bWrapLoop = Spec.WrapMode == ENsTweenWrapMode::Loop;
    bWrapPingPong = Spec.WrapMode == ENsTweenWrapMode::PingPong;
    StrategyPtr = Strategy.Get();
    EasingPtr = Easing.Get();
    bHasOnUpdate = Spec.OnUpdate.IsBound();
    bHasOnComplete = Spec.OnComplete.IsBound();
    bHasOnLoop = Spec.OnLoop.IsBound();
    bHasOnPingPong = Spec.OnPingPong.IsBound();
    CycleTime = (Spec.Direction == ENsTweenDirection::Forward) ? 0.f : Duration;
    bPlayingForward = (Spec.Direction != ENsTweenDirection::Backward);
}

bool FNsTween::Tick(float DeltaSeconds)
{
    NSTWEEN_SCOPE_CYCLE_COUNTER("NsTween::Tick");
    // Bail out immediately if the tween is already completed, paused, or missing runtime pieces.
    if (!bActive || bPaused || StrategyPtr == nullptr || EasingPtr == nullptr)
    {
        return bActive;
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
    float ScaledDelta = DeltaSeconds * TimeScale;
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
    const float LocalDuration = Duration;

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

    if (bApplyFinal && StrategyPtr)
    {
        StrategyPtr->ApplyFinal();
    }

    if (bHasOnComplete)
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
    if (!StrategyPtr || !EasingPtr)
    {
        return;
    }

    float LinearAlpha = InCycleTime * DurationInverse;
    if (LinearAlpha <= 0.f)
    {
        LinearAlpha = 0.f;
    }
    else if (LinearAlpha >= 1.f)
    {
        LinearAlpha = 1.f;
    }

    const float EasedAlpha = EasingPtr->Evaluate(LinearAlpha);

    StrategyPtr->Apply(EasedAlpha);
    if (bHasOnUpdate)
    {
        Spec.OnUpdate.Execute(EasedAlpha);
    }
}

bool FNsTween::HandleBoundary(float& RemainingTime)
{
    NSTWEEN_SCOPE_CYCLE_COUNTER("NsTween::HandleBoundary");
    if (bWrapOnce)
    {
        if (bHasOnComplete)
        {
            Spec.OnComplete.Execute();
        }
        if (StrategyPtr)
        {
            StrategyPtr->ApplyFinal();
        }
        bActive = false;
        return false;
    }

    if (bWrapLoop)
    {
        ++CompletedCycles;
        if (bHasOnLoop)
        {
            Spec.OnLoop.Execute();
        }

        if (bHasLoopLimit && CompletedCycles >= LoopLimit)
        {
            if (bHasOnComplete)
            {
                Spec.OnComplete.Execute();
            }
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
    else if (bWrapPingPong)
    {
        bPlayingForward = !bPlayingForward;
        if (bHasOnPingPong)
        {
            Spec.OnPingPong.Execute();
        }

        if (!bPlayingForward)
        {
            ++CompletedPingPongPairs;
        }
        else if (bHasLoopLimit && CompletedPingPongPairs >= LoopLimit)
        {
            if (bHasOnComplete)
            {
                Spec.OnComplete.Execute();
            }
            if (StrategyPtr)
            {
                StrategyPtr->ApplyFinal();
            }
            bActive = false;
            return false;
        }

        CycleTime = bPlayingForward ? 0.f : Duration;
    }

    // Consume whatever time is left using the new direction or wrap mode.
    if (RemainingTime < 0.f)
    {
        RemainingTime = 0.f;
    }
    return true;
}

