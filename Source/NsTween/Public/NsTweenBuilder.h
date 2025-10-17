// Copyright (C) 2024 mykaa. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "NsTweenTypeLibrary.h"

class ITweenValue;
class IEasingCurve;
class UCurveFloat;
class UNsTweenSubsystem;

struct NSTWEEN_API FNsTweenBuilder
{
    FNsTweenBuilder();

    FNsTweenBuilder(const FNsTweenBuilder& Other) = default;
    FNsTweenBuilder(FNsTweenBuilder&& Other) = default;
    FNsTweenBuilder& operator=(const FNsTweenBuilder& Other) = default;
    FNsTweenBuilder& operator=(FNsTweenBuilder&& Other) = default;

    ~FNsTweenBuilder();

    FNsTweenBuilder& SetPingPong(bool bEnable);
    FNsTweenBuilder& SetLoops(int32 LoopCount);
    FNsTweenBuilder& SetDelay(float DelaySeconds);
    FNsTweenBuilder& SetTimeScale(float TimeScale);
    FNsTweenBuilder& SetCurveAsset(UCurveFloat* Curve);

    FNsTweenBuilder& OnComplete(TFunction<void()> Callback);
    FNsTweenBuilder& OnLoop(TFunction<void()> Callback);
    FNsTweenBuilder& OnPingPong(TFunction<void()> Callback);

    void Pause() const;
    void Resume() const;
    void Cancel(bool bApplyFinal = true) const;
    bool IsActive() const;

    FNsTweenHandle GetHandle() const;
    bool IsValid() const;

    operator FNsTweenHandle() const
    {
        return GetHandle();
    }

private:
    struct FState;

    explicit FNsTweenBuilder(const TSharedPtr<FState>& InState);

    bool CanConfigure() const;
    void Activate() const;
    void UpdateWrapMode() const;

private:
    TSharedPtr<FState> State;

    friend struct FNsTween;
};

struct FNsTweenBuilder::FState : public TSharedFromThis<FState>
{
    FNsTweenSpec Spec;
    TFunction<TSharedPtr<ITweenValue>()> StrategyFactory;
    TSharedPtr<TFunction<void()>> CompleteCallback;
    TSharedPtr<TFunction<void()>> LoopCallback;
    TSharedPtr<TFunction<void()>> PingPongCallback;
    FNsTweenHandle Handle;
    bool bLooping = false;
    bool bPingPong = false;
    bool bActivated = false;
};