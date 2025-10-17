// Copyright (C) 2025 nulled.softworks. All rights reserved.

#include "NsTween.h"

#include "Interfaces/ITweenValue.h"
#include "Interfaces/IEasingCurve.h"
#include "Math/UnrealMathUtility.h"
#include "Engine/Engine.h"
#include "NsTweenSubsystem.h"

/**
 * The non-templated Play overload wires an explicit specification and strategy factory into the builder.
 * Everything else funnels through this helper so we only have one place that knows how to seed the shared state.
 */
FNsTweenBuilder FNsTween::Play(FNsTweenSpec Spec, TFunction<TSharedPtr<ITweenValue>()> StrategyFactory)
{
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
    , DelayRemaining(FMath::Max(0.f, InSpec.DelaySeconds))
    , CycleTime(InSpec.Direction == ENsTweenDirection::Forward ? 0.f : FMath::Max(InSpec.DurationSeconds, SMALL_NUMBER))
    , bPlayingForward(InSpec.Direction != ENsTweenDirection::Backward)
{
}

bool FNsTween::Tick(float DeltaSeconds)
{
    // Bail out immediately if the tween is already completed, paused, or missing runtime pieces.
    if (!bActive || bPaused || !Strategy.IsValid() || !Easing.IsValid())
    {
        return bActive;
    }

    // Lazily initialize the strategy the first time we tick so creation happens on the game thread.
    if (!bInitialized)
    {
        Strategy->Initialize();
        if (Spec.Direction == ENsTweenDirection::Backward)
        {
            Strategy->ApplyFinal();
        }
        bInitialized = true;
    }

    // Respect the time scale so tweens can speed up or slow down deterministically.
    float ScaledDelta = DeltaSeconds * FMath::Max(Spec.TimeScale, 0.f);
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
    float Duration = FMath::Max(Spec.DurationSeconds, SMALL_NUMBER);

    while (RemainingTime > SMALL_NUMBER && bActive)
    {
        // Figure out how far we will advance during this tick, respecting direction and wrap mode.
        const float DirectionFactor = bPlayingForward ? 1.f : -1.f;
        float NextCycleTime = CycleTime + RemainingTime * DirectionFactor;

        const float TargetBoundary = bPlayingForward ? Duration : 0.f;
        const bool bCrossingBoundary = bPlayingForward ? (NextCycleTime >= TargetBoundary) : (NextCycleTime <= TargetBoundary);

        if (!bCrossingBoundary)
        {
            CycleTime = FMath::Clamp(NextCycleTime, 0.f, Duration);
            Apply(CycleTime);
            break;
        }

        // We will cross a boundary this frame, so finish the current segment first.
        const float TimeToBoundary = FMath::Abs(TargetBoundary - CycleTime);
        CycleTime = TargetBoundary;
        Apply(CycleTime);
        RemainingTime -= TimeToBoundary;

        if (!HandleBoundary(RemainingTime))
        {
            return false;
        }
    }

    return bActive;
}

void FNsTween::Cancel(bool bApplyFinal)
{
    // Cancellation is idempotent; the first call decides whether to apply the final value.
    if (!bActive)
    {
        return;
    }

    if (bApplyFinal && Strategy.IsValid())
    {
        Strategy->ApplyFinal();
    }

    if (Spec.OnComplete.IsBound())
    {
        Spec.OnComplete.Execute();
    }

    bActive = false;
}

void FNsTween::SetPaused(bool bInPaused)
{
    bPaused = bInPaused;
}

void FNsTween::Apply(float InCycleTime)
{
    // Guard against misconfigured tweens that somehow lost their runtime strategy.
    if (!Strategy.IsValid() || !Easing.IsValid())
    {
        return;
    }

    const float Duration = FMath::Max(Spec.DurationSeconds, SMALL_NUMBER);
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
    float Duration = FMath::Max(Spec.DurationSeconds, SMALL_NUMBER);

    switch (Spec.WrapMode)
    {
    case ENsTweenWrapMode::Once:
        // Notify completion exactly once and ensure we end on the last value.
        if (Spec.OnComplete.IsBound())
        {
            Spec.OnComplete.Execute();
        }
        Strategy->ApplyFinal();
        bActive = false;
        return false;

    case ENsTweenWrapMode::Loop:
        ++CompletedCycles;
        if (Spec.OnLoop.IsBound())
        {
            Spec.OnLoop.Execute();
        }

        if (Spec.LoopCount > 0 && CompletedCycles >= Spec.LoopCount)
        {
            // Exhausted the requested loop count, so wrap like a completion.
            if (Spec.OnComplete.IsBound())
            {
                Spec.OnComplete.Execute();
            }
            Strategy->ApplyFinal();
            bActive = false;
            return false;
        }

        CycleTime = 0.f;
        bPlayingForward = (Spec.Direction != ENsTweenDirection::Backward);
        break;

    case ENsTweenWrapMode::PingPong:
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
            // A ping-pong pair counts as a single loop when evaluating completion.
            if (Spec.OnComplete.IsBound())
            {
                Spec.OnComplete.Execute();
            }
            Strategy->ApplyFinal();
            bActive = false;
            return false;
        }

        CycleTime = bPlayingForward ? 0.f : Duration;
        break;
    }

    // Consume whatever time is left using the new direction or wrap mode.
    RemainingTime = FMath::Max(RemainingTime, 0.f);
    return true;
}

