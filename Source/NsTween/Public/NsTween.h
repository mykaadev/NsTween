// Copyright (C) 2024 mykaa. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "NsTweenBuilder.h"
#include "NsTweenTypeLibrary.h"

class ITweenValue;
class IEasingCurve;
class UCurveFloat;
class UNsTweenSubsystem;

template <typename TValue>
struct FNsTweenInterpolator
{
    static TValue Lerp(const TValue& A, const TValue& B, float Alpha)
    {
        return FMath::Lerp(A, B, Alpha);
    }
};

template <>
struct FNsTweenInterpolator<FVector2D>
{
    static FVector2D Lerp(const FVector2D& A, const FVector2D& B, float Alpha)
    {
        return A + (B - A) * Alpha;
    }
};

template <>
struct FNsTweenInterpolator<FRotator>
{
    static FRotator Lerp(const FRotator& A, const FRotator& B, float Alpha)
    {
        const float T = FMath::Clamp(Alpha, 0.f, 1.f);
        const FQuat QA = A.Quaternion();
        const FQuat QB = B.Quaternion();
        const FQuat Q = FQuat::Slerp(QA, QB, T).GetNormalized();
        return Q.Rotator();
    }
};

template <>
struct FNsTweenInterpolator<FTransform>
{
    static FTransform Lerp(const FTransform& A, const FTransform& B, float Alpha)
    {
        FTransform Result;
        Result.Blend(A, B, Alpha);
        return Result;
    }
};

template <>
struct FNsTweenInterpolator<FLinearColor>
{
    static FLinearColor Lerp(const FLinearColor& A, const FLinearColor& B, float Alpha)
    {
        return FLinearColor::LerpUsingHSV(A, B, Alpha);
    }
};

template <>
struct FNsTweenInterpolator<FQuat>
{
    static FQuat Lerp(const FQuat& A, const FQuat& B, float Alpha)
    {
        const float T = FMath::Clamp(Alpha, 0.f, 1.f);
        return FQuat::Slerp(A, B, T).GetNormalized();
    }
};



template <typename TValue>
class FNsTweenCallbackValue : public ITweenValue
{
public:
    using FUpdateFunction = TFunction<void(const TValue&)>;

    FNsTweenCallbackValue(const TValue& InStart, const TValue& InEnd, FUpdateFunction InUpdate)
        : StartValue(InStart)
        , EndValue(InEnd)
        , UpdateFunction(MoveTemp(InUpdate))
    {
    }

    virtual void Initialize() override
    {
        if (UpdateFunction)
        {
            UpdateFunction(StartValue);
        }
    }

    virtual void Apply(float EasedAlpha) override
    {
        if (UpdateFunction)
        {
            const TValue Interpolated = FNsTweenInterpolator<TValue>::Lerp(StartValue, EndValue, EasedAlpha);
            UpdateFunction(Interpolated);
        }
    }

    virtual void ApplyFinal() override
    {
        if (UpdateFunction)
        {
            UpdateFunction(EndValue);
        }
    }

private:
    TValue StartValue;
    TValue EndValue;
    FUpdateFunction UpdateFunction;
};

template <typename TValue>
TSharedPtr<ITweenValue> MakeNsTweenCallbackStrategy(const TValue& StartValue, const TValue& EndValue, TFunction<void(const TValue&)> Update)
{
    return MakeShared<FNsTweenCallbackValue<TValue>>(StartValue, EndValue, MoveTemp(Update));
}

struct FNsTween
{

public:

    /** Generic convenience wrapper: accepts any callable and forwards to the correct typed overload */
    template <typename TCallable, typename TType>
    static FNsTweenBuilder Play(const TType& StartValue, const TType& EndValue, float DurationSeconds, ENsTweenEase Ease, TCallable&& Update)
    {
        // Own the callable as a TFunction<void(const TType&)>
        TFunction<void(const TType&)> Fn(Forward<TCallable>(Update));
        return BuildT<TType>(StartValue, EndValue, DurationSeconds, Ease, MoveTemp(Fn));
    }

private:

    /** Default Build for any TType that has a valid MakeNsTweenCallbackStrategy<TType> */
    template <typename TType>
    static FNsTweenBuilder BuildT(const TType& StartValue, const TType& EndValue, float DurationSeconds, ENsTweenEase Ease, TFunction<void(const TType&)> Update)
    {
        const TSharedPtr<FNsTweenBuilder::FState> State = MakeShared<FNsTweenBuilder::FState>();
        State->Spec.DurationSeconds = FMath::Max(DurationSeconds, 0.f);
        State->Spec.DelaySeconds    = 0.f;
        State->Spec.TimeScale       = 1.f;
        State->Spec.WrapMode        = ENsTweenWrapMode::Once;
        State->Spec.LoopCount       = 0;
        State->Spec.Direction       = ENsTweenDirection::Forward;
        State->Spec.EasingPreset    = Ease;

        // Own Update so it outlives this call, then create the strategy with TType
        TFunction<void(const TType&)> Owned = MoveTemp(Update);
        State->StrategyFactory = [StartValue, EndValue, Owned = MoveTemp(Owned)]() mutable -> TSharedPtr<class ITweenValue>
        {
            return MakeNsTweenCallbackStrategy<TType>(StartValue, EndValue, MoveTemp(Owned));
        };

        return FNsTweenBuilder(State);
    }

public:

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
