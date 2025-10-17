// Copyright (C) 2024 mykaa. All rights reserved.

#include "NsTween.h"
#include "Interfaces/ITweenValue.h"
#include "Interfaces/IEasingCurve.h"
#include "Math/UnrealMathUtility.h"
#include "Engine/Engine.h"
#include "NsTweenSubsystem.h"

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
    if (!bActive || bPaused || !Strategy.IsValid() || !Easing.IsValid())
    {
        return bActive;
    }

    if (!bInitialized)
    {
        Strategy->Initialize();
        if (Spec.Direction == ENsTweenDirection::Backward)
        {
            Strategy->ApplyFinal();
        }
        bInitialized = true;
    }

    float ScaledDelta = DeltaSeconds * FMath::Max(Spec.TimeScale, 0.f);
    if (ScaledDelta <= SMALL_NUMBER)
    {
        return true;
    }

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

    RemainingTime = FMath::Max(RemainingTime, 0.f);
    return true;
}

