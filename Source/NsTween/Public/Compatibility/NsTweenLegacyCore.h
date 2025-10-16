// Copyright (C) 2024 mykaa. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "Interfaces/ITweenValue.h"
#include "TweenEnums.h"
#include "TweenHandle.h"
#include "TweenSpec.h"

class UNsTweenManagerSubsystem;

namespace NsTweenCore
{
enum class ENsTweenEase : uint8
{
    Linear = 0,
    InSine,
    OutSine,
    InOutSine,
    InQuad,
    OutQuad,
    InOutQuad,
    InCubic,
    OutCubic,
    InOutCubic,
    InExpo,
    OutExpo,
    InOutExpo
};
}

class NSTWEEN_API FNsTweenInstance
{
public:
    struct FSharedState : public TSharedFromThis<FSharedState>
    {
        FNovaTweenSpec Spec;
        TFunction<TSharedPtr<ITweenValue>()> StrategyFactory;
        TSharedPtr<TFunction<void()>> CompleteCallback;
        TSharedPtr<TFunction<void()>> LoopCallback;
        TSharedPtr<TFunction<void()>> PingPongCallback;
        FNovaTweenHandle Handle;
        int32 RequestedLoopCount = 0;
        bool bPingPong = false;
        bool bActivated = false;
    };

    FNsTweenInstance();
    explicit FNsTweenInstance(const TSharedPtr<FSharedState>& InState);

    bool IsValid() const;

    FNsTweenInstance& SetPingPong(bool bEnable);
    FNsTweenInstance& SetLoops(int32 LoopCount);
    FNsTweenInstance& SetDelay(float DelaySeconds);
    FNsTweenInstance& SetTimeScale(float TimeScale);

    FNsTweenInstance& OnComplete(TFunction<void()> Callback);
    FNsTweenInstance& OnLoop(TFunction<void()> Callback);
    FNsTweenInstance& OnPingPong(TFunction<void()> Callback);

    void Pause() const;
    void Resume() const;
    void Cancel(bool bApplyFinal = true) const;
    bool IsActive() const;

    FNovaTweenHandle GetHandle() const;

protected:
    void Activate() const;
    bool CanConfigure() const;
    void UpdateLoopSettings() const;

    TSharedPtr<FSharedState> State;
};

namespace NsTweenCore
{
namespace Internal
{
    template <typename TValue>
    struct TInterpolator
    {
        static TValue Lerp(const TValue& A, const TValue& B, float Alpha)
        {
            return FMath::Lerp(A, B, Alpha);
        }
    };

    template <>
    struct TInterpolator<FRotator>
    {
        static FRotator Lerp(const FRotator& A, const FRotator& B, float Alpha)
        {
            return FMath::LerpShortestPath(A, B, Alpha);
        }
    };

    template <>
    struct TInterpolator<FTransform>
    {
        static FTransform Lerp(const FTransform& A, const FTransform& B, float Alpha)
        {
            FTransform Result;
            Result.Blend(A, B, Alpha);
            return Result;
        }
    };

    template <>
    struct TInterpolator<FLinearColor>
    {
        static FLinearColor Lerp(const FLinearColor& A, const FLinearColor& B, float Alpha)
        {
            return FLinearColor::LerpUsingHSV(A, B, Alpha);
        }
    };

    template <typename TValue>
    class TCallbackTweenValue : public ITweenValue
    {
    public:
        using FUpdateFunction = TFunction<void(const TValue&)>;

        TCallbackTweenValue(const TValue& InStart, const TValue& InEnd, FUpdateFunction InUpdate)
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
                const TValue Interpolated = TInterpolator<TValue>::Lerp(StartValue, EndValue, EasedAlpha);
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
    TSharedPtr<ITweenValue> MakeCallbackStrategy(const TValue& StartValue, const TValue& EndValue, TFunction<void(const TValue&)> Update)
    {
        return MakeShared<TCallbackTweenValue<TValue>>(StartValue, EndValue, MoveTemp(Update));
    }

    NSTWEEN_API ENovaEasingPreset ConvertEase(ENsTweenEase Ease);
}

template <typename TValue>
class TTweenBuilder : public FNsTweenInstance
{
public:
    using FUpdateFunction = TFunction<void(const TValue&)>;

    TTweenBuilder(const TValue& StartValue, const TValue& EndValue, float Duration, ENsTweenEase Ease, FUpdateFunction Update)
    {
        State = MakeShared<FSharedState>();
        State->Spec.DurationSeconds = Duration;
        State->Spec.DelaySeconds = 0.f;
        State->Spec.TimeScale = 1.f;
        State->Spec.WrapMode = ENovaTweenWrapMode::Once;
        State->Spec.LoopCount = 0;
        State->Spec.Direction = ENovaTweenDirection::Forward;
        State->Spec.EasingPreset = Internal::ConvertEase(Ease);
        State->StrategyFactory = [StartValue, EndValue, Update = MoveTemp(Update)]() mutable -> TSharedPtr<ITweenValue>
        {
            return Internal::MakeCallbackStrategy(StartValue, EndValue, MoveTemp(Update));
        };
    }

    TTweenBuilder(const TTweenBuilder& Other) = default;
    TTweenBuilder(TTweenBuilder&& Other) = default;

    ~TTweenBuilder()
    {
        Activate();
    }

    operator FNsTweenInstance()
    {
        Activate();
        return FNsTweenInstance(State);
    }
};

NSTWEEN_API TTweenBuilder<float> Play(float StartValue, float EndValue, float DurationSeconds, ENsTweenEase Ease, TFunction<void(float)> Update);
NSTWEEN_API TTweenBuilder<FVector> Play(const FVector& StartValue, const FVector& EndValue, float DurationSeconds, ENsTweenEase Ease, TFunction<void(const FVector&)> Update);
NSTWEEN_API TTweenBuilder<FRotator> Play(const FRotator& StartValue, const FRotator& EndValue, float DurationSeconds, ENsTweenEase Ease, TFunction<void(const FRotator&)> Update);
NSTWEEN_API TTweenBuilder<FTransform> Play(const FTransform& StartValue, const FTransform& EndValue, float DurationSeconds, ENsTweenEase Ease, TFunction<void(const FTransform&)> Update);
NSTWEEN_API TTweenBuilder<FLinearColor> Play(const FLinearColor& StartValue, const FLinearColor& EndValue, float DurationSeconds, ENsTweenEase Ease, TFunction<void(const FLinearColor&)> Update);

template <typename TCallable>
TTweenBuilder<float> Play(float StartValue, float EndValue, float DurationSeconds, ENsTweenEase Ease, TCallable&& Update)
{
    return Play(StartValue, EndValue, DurationSeconds, Ease, TFunction<void(float)>(Forward<TCallable>(Update)));
}

template <typename TCallable>
TTweenBuilder<FVector> Play(const FVector& StartValue, const FVector& EndValue, float DurationSeconds, ENsTweenEase Ease, TCallable&& Update)
{
    return Play(StartValue, EndValue, DurationSeconds, Ease, TFunction<void(const FVector&)>(Forward<TCallable>(Update)));
}

template <typename TCallable>
TTweenBuilder<FRotator> Play(const FRotator& StartValue, const FRotator& EndValue, float DurationSeconds, ENsTweenEase Ease, TCallable&& Update)
{
    return Play(StartValue, EndValue, DurationSeconds, Ease, TFunction<void(const FRotator&)>(Forward<TCallable>(Update)));
}

template <typename TCallable>
TTweenBuilder<FTransform> Play(const FTransform& StartValue, const FTransform& EndValue, float DurationSeconds, ENsTweenEase Ease, TCallable&& Update)
{
    return Play(StartValue, EndValue, DurationSeconds, Ease, TFunction<void(const FTransform&)>(Forward<TCallable>(Update)));
}

template <typename TCallable>
TTweenBuilder<FLinearColor> Play(const FLinearColor& StartValue, const FLinearColor& EndValue, float DurationSeconds, ENsTweenEase Ease, TCallable&& Update)
{
    return Play(StartValue, EndValue, DurationSeconds, Ease, TFunction<void(const FLinearColor&)>(Forward<TCallable>(Update)));
}

} // namespace NsTweenCore

