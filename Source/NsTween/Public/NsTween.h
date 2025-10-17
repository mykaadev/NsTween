// Copyright (C) 2024 mykaa. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "NsTweenTypeLibrary.h"

class ITweenValue;
class IEasingCurve;
class UCurveFloat;
class UNsTweenSubsystem;

struct FNsTween;

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

struct FNsTween
{
    static FNsTweenBuilder Play(float StartValue, float EndValue, float DurationSeconds, ENsTweenEase Ease, TFunction<void(const float&)> Update);
    static FNsTweenBuilder Play(const FVector& StartValue, const FVector& EndValue, float DurationSeconds, ENsTweenEase Ease, TFunction<void(const FVector&)> Update);
    static FNsTweenBuilder Play(const FVector2D& StartValue, const FVector2D& EndValue, float DurationSeconds, ENsTweenEase Ease, TFunction<void(const FVector2D&)> Update);
    static FNsTweenBuilder Play(const FRotator& StartValue, const FRotator& EndValue, float DurationSeconds, ENsTweenEase Ease, TFunction<void(const FRotator&)> Update);
    static FNsTweenBuilder Play(const FQuat& StartValue, const FQuat& EndValue, float DurationSeconds, ENsTweenEase Ease, TFunction<void(const FQuat&)> Update);
    static FNsTweenBuilder Play(const FTransform& StartValue, const FTransform& EndValue, float DurationSeconds, ENsTweenEase Ease, TFunction<void(const FTransform&)> Update);
    static FNsTweenBuilder Play(const FLinearColor& StartValue, const FLinearColor& EndValue, float DurationSeconds, ENsTweenEase Ease, TFunction<void(const FLinearColor&)> Update);

    template <typename TCallable>
    static FNsTweenBuilder Play(float StartValue, float EndValue, float DurationSeconds, ENsTweenEase Ease, TCallable&& Update)
    {
        return Play(StartValue, EndValue, DurationSeconds, Ease, TFunction<void(const float&)>(Forward<TCallable>(Update)));
    }

    template <typename TCallable>
    static FNsTweenBuilder Play(const FVector& StartValue, const FVector& EndValue, float DurationSeconds, ENsTweenEase Ease, TCallable&& Update)
    {
        return Play(StartValue, EndValue, DurationSeconds, Ease, TFunction<void(const FVector&)>(Forward<TCallable>(Update)));
    }

    template <typename TCallable>
    static FNsTweenBuilder Play(const FVector2D& StartValue, const FVector2D& EndValue, float DurationSeconds, ENsTweenEase Ease, TCallable&& Update)
    {
        return Play(StartValue, EndValue, DurationSeconds, Ease, TFunction<void(const FVector2D&)>(Forward<TCallable>(Update)));
    }

    template <typename TCallable>
    static FNsTweenBuilder Play(const FRotator& StartValue, const FRotator& EndValue, float DurationSeconds, ENsTweenEase Ease, TCallable&& Update)
    {
        return Play(StartValue, EndValue, DurationSeconds, Ease, TFunction<void(const FRotator&)>(Forward<TCallable>(Update)));
    }

    template <typename TCallable>
    static FNsTweenBuilder Play(const FQuat& StartValue, const FQuat& EndValue, float DurationSeconds, ENsTweenEase Ease, TCallable&& Update)
    {
        return Play(StartValue, EndValue, DurationSeconds, Ease, TFunction<void(const FQuat&)>(Forward<TCallable>(Update)));
    }

    template <typename TCallable>
    static FNsTweenBuilder Play(const FTransform& StartValue, const FTransform& EndValue, float DurationSeconds, ENsTweenEase Ease, TCallable&& Update)
    {
        return Play(StartValue, EndValue, DurationSeconds, Ease, TFunction<void(const FTransform&)>(Forward<TCallable>(Update)));
    }

    template <typename TCallable>
    static FNsTweenBuilder Play(const FLinearColor& StartValue, const FLinearColor& EndValue, float DurationSeconds, ENsTweenEase Ease, TCallable&& Update)
    {
        return Play(StartValue, EndValue, DurationSeconds, Ease, TFunction<void(const FLinearColor&)>(Forward<TCallable>(Update)));
    }

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

struct NsTween
{
    using FBuilder = FNsTweenBuilder;

    static FNsTweenBuilder Play(float StartValue, float EndValue, float DurationSeconds, ENsTweenEase Ease, TFunction<void(const float&)> Update)
    {
        return FNsTween::Play(StartValue, EndValue, DurationSeconds, Ease, MoveTemp(Update));
    }

    static FNsTweenBuilder Play(const FVector& StartValue, const FVector& EndValue, float DurationSeconds, ENsTweenEase Ease, TFunction<void(const FVector&)> Update)
    {
        return FNsTween::Play(StartValue, EndValue, DurationSeconds, Ease, MoveTemp(Update));
    }

    static FNsTweenBuilder Play(const FVector2D& StartValue, const FVector2D& EndValue, float DurationSeconds, ENsTweenEase Ease, TFunction<void(const FVector2D&)> Update)
    {
        return FNsTween::Play(StartValue, EndValue, DurationSeconds, Ease, MoveTemp(Update));
    }

    static FNsTweenBuilder Play(const FRotator& StartValue, const FRotator& EndValue, float DurationSeconds, ENsTweenEase Ease, TFunction<void(const FRotator&)> Update)
    {
        return FNsTween::Play(StartValue, EndValue, DurationSeconds, Ease, MoveTemp(Update));
    }

    static FNsTweenBuilder Play(const FQuat& StartValue, const FQuat& EndValue, float DurationSeconds, ENsTweenEase Ease, TFunction<void(const FQuat&)> Update)
    {
        return FNsTween::Play(StartValue, EndValue, DurationSeconds, Ease, MoveTemp(Update));
    }

    static FNsTweenBuilder Play(const FTransform& StartValue, const FTransform& EndValue, float DurationSeconds, ENsTweenEase Ease, TFunction<void(const FTransform&)> Update)
    {
        return FNsTween::Play(StartValue, EndValue, DurationSeconds, Ease, MoveTemp(Update));
    }

    static FNsTweenBuilder Play(const FLinearColor& StartValue, const FLinearColor& EndValue, float DurationSeconds, ENsTweenEase Ease, TFunction<void(const FLinearColor&)> Update)
    {
        return FNsTween::Play(StartValue, EndValue, DurationSeconds, Ease, MoveTemp(Update));
    }

    template <typename TValue, typename TCallable>
    static FNsTweenBuilder Play(const TValue& StartValue, const TValue& EndValue, float DurationSeconds, ENsTweenEase Ease, TCallable&& Update)
    {
        return FNsTween::Play(StartValue, EndValue, DurationSeconds, Ease, Forward<TCallable>(Update));
    }
};
