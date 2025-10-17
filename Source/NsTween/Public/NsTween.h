// Copyright (C) 2024 mykaa. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "NsTweenTypeLibrary.h"

class ITweenValue;
class IEasingCurve;

struct FNsTween
{
    FNsTween(const FNsTweenHandle& InHandle, FNsTweenSpec InSpec, TSharedPtr<ITweenValue> InStrategy, TSharedPtr<IEasingCurve> InEasing);

    bool Tick(float DeltaSeconds);
    void Cancel(bool bApplyFinal);
    void SetPaused(bool bInPaused);

    bool IsActive() const { return bActive; }
    const FNsTweenHandle& GetHandle() const { return Handle; }

private:
    void Apply(float CycleTime);
    bool HandleBoundary(float& RemainingTime);

private:
    FNsTweenHandle Handle;
    FNsTweenSpec Spec;
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

