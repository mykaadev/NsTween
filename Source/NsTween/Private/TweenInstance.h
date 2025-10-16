// Copyright (C) 2024 mykaa. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "TweenHandle.h"
#include "TweenSpec.h"

class ITweenValue;
class IEasingCurve;

struct FTweenInstance
{
    FTweenInstance(const FNovaTweenHandle& InHandle, FNovaTweenSpec InSpec, TSharedPtr<ITweenValue> InStrategy, TSharedPtr<IEasingCurve> InEasing);

    bool Tick(float DeltaSeconds);
    void Cancel(bool bApplyFinal);
    void SetPaused(bool bInPaused);

    bool IsActive() const { return bActive; }
    const FNovaTweenHandle& GetHandle() const { return Handle; }

private:
    void Apply(float CycleTime);
    bool HandleBoundary(float& RemainingTime);

private:
    FNovaTweenHandle Handle;
    FNovaTweenSpec Spec;
    TSharedPtr<ITweenValue> Strategy;
    TSharedPtr<IEasingCurve> Easing;

    float DelayRemaining = 0.0f;
    float CycleTime = 0.0f;
    int32 CompletedCycles = 0;
    int32 CompletedPingPongPairs = 0;
    bool bActive = true;
    bool bPaused = false;
    bool bInitialized = false;
    bool bPlayingForward = true;
};

