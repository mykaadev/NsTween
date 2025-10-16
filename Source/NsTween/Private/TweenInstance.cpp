// Copyright (C) 2024 mykaa. All rights reserved.

#include "TweenInstance.h"
#include "Interfaces/ITweenValue.h"
#include "Interfaces/IEasingCurve.h"
#include "TweenDelegates.h"
#include "Math/UnrealMathUtility.h"

namespace
{
    constexpr float SMALL_DELTA = 1.e-6f;
}

FTweenInstance::FTweenInstance(const FNovaTweenHandle& InHandle, FNovaTweenSpec InSpec, TSharedPtr<ITweenValue> InStrategy, TSharedPtr<IEasingCurve> InEasing)
    : Handle(InHandle)
    , Spec(MoveTemp(InSpec))
    , Strategy(MoveTemp(InStrategy))
    , Easing(MoveTemp(InEasing))
    , DelayRemaining(FMath::Max(0.f, InSpec.DelaySeconds))
    , CycleTime(InSpec.Direction == ENovaTweenDirection::Forward ? 0.f : FMath::Max(InSpec.DurationSeconds, SMALL_DELTA))
    , bPlayingForward(InSpec.Direction != ENovaTweenDirection::Backward)
{
}

bool FTweenInstance::Tick(float DeltaSeconds)
{
    if (!bActive || bPaused || !Strategy.IsValid() || !Easing.IsValid())
    {
        return bActive;
    }

    if (!bInitialized)
    {
        Strategy->Initialize();
        if (Spec.Direction == ENovaTweenDirection::Backward)
        {
            Strategy->ApplyFinal();
        }
        bInitialized = true;
    }

    float ScaledDelta = DeltaSeconds * FMath::Max(Spec.TimeScale, 0.f);
    if (ScaledDelta <= SMALL_DELTA)
    {
        return true;
    }

    if (DelayRemaining > SMALL_DELTA)
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
    float Duration = FMath::Max(Spec.DurationSeconds, SMALL_DELTA);

    while (RemainingTime > SMALL_DELTA && bActive)
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

void FTweenInstance::Cancel(bool bApplyFinal)
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

void FTweenInstance::SetPaused(bool bInPaused)
{
    bPaused = bInPaused;
}

void FTweenInstance::Apply(float InCycleTime)
{
    if (!Strategy.IsValid() || !Easing.IsValid())
    {
        return;
    }

    float Duration = FMath::Max(Spec.DurationSeconds, SMALL_DELTA);
    float LinearAlpha = FMath::Clamp(InCycleTime / Duration, 0.f, 1.f);
    float EasedAlpha = Easing->Evaluate(LinearAlpha);

    Strategy->Apply(EasedAlpha);
    if (Spec.OnUpdate.IsBound())
    {
        Spec.OnUpdate.Execute(EasedAlpha);
    }
}

bool FTweenInstance::HandleBoundary(float& RemainingTime)
{
    float Duration = FMath::Max(Spec.DurationSeconds, SMALL_DELTA);

    switch (Spec.WrapMode)
    {
    case ENovaTweenWrapMode::Once:
        if (Spec.OnComplete.IsBound())
        {
            Spec.OnComplete.Execute();
        }
        Strategy->ApplyFinal();
        bActive = false;
        return false;

    case ENovaTweenWrapMode::Loop:
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
        bPlayingForward = (Spec.Direction != ENovaTweenDirection::Backward);
        break;

    case ENovaTweenWrapMode::PingPong:
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

