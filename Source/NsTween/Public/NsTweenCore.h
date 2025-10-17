// Copyright (C) 2024 mykaa. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "Interfaces/ITweenValue.h"
#include "NsTweenTypeLibrary.h"

class UCurveFloat;

class UNsTweenSubsystem;

class NSTWEEN_API FNsTweenHandleRef
{
public:
    struct FSharedState : public TSharedFromThis<FSharedState>
    {
        FNsTweenSpec Spec;
        TFunction<TSharedPtr<ITweenValue>()> StrategyFactory;
        TSharedPtr<TFunction<void()>> CompleteCallback;
        TSharedPtr<TFunction<void()>> LoopCallback;
        TSharedPtr<TFunction<void()>> PingPongCallback;
        FNsTweenHandle Handle;
        int32 RequestedLoopCount = 0;
        bool bPingPong = false;
        bool bActivated = false;
    };

    FNsTweenHandleRef();
    explicit FNsTweenHandleRef(const TSharedPtr<FSharedState>& InState);

    bool IsValid() const;

    FNsTweenHandleRef& SetPingPong(bool bEnable);
    FNsTweenHandleRef& SetLoops(int32 LoopCount);
    FNsTweenHandleRef& SetDelay(float DelaySeconds);
    FNsTweenHandleRef& SetTimeScale(float TimeScale);
    FNsTweenHandleRef& SetCurveAsset(UCurveFloat* Curve);

    FNsTweenHandleRef& OnComplete(TFunction<void()> Callback);
    FNsTweenHandleRef& OnLoop(TFunction<void()> Callback);
    FNsTweenHandleRef& OnPingPong(TFunction<void()> Callback);

    void Pause() const;
    void Resume() const;
    void Cancel(bool bApplyFinal = true) const;
    bool IsActive() const;

    FNsTweenHandle GetHandle() const;

protected:
    void Activate() const;
    bool CanConfigure() const;
    void UpdateLoopSettings() const;

    TSharedPtr<FSharedState> State;
};

namespace NsTween
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
    struct TInterpolator<FVector2D>
    {
        static FVector2D Lerp(const FVector2D& A, const FVector2D& B, float Alpha)
        {
            return A + (B - A) * Alpha;
        }
    };

    template <>
    struct TInterpolator<FRotator>
    {
        static FRotator Lerp(const FRotator& A, const FRotator& B, float Alpha)
        {
            const float T = FMath::Clamp(Alpha, 0.f, 1.f);

            const FQuat QA = A.Quaternion();
            const FQuat QB = B.Quaternion();
            const FQuat Q  = FQuat::Slerp(QA, QB, T).GetNormalized();

            return Q.Rotator();
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

    template <>
    struct TInterpolator<FQuat>
    {
        static FQuat Lerp(const FQuat& A, const FQuat& B, float Alpha)
        {
            const float T = FMath::Clamp(Alpha, 0.f, 1.f);
            return FQuat::Slerp(A, B, T).GetNormalized();
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
    TSharedPtr<ITweenValue> MakeCallbackStrategy(const TValue& StartValue, const TValue& EndValue, TFunction<void(const TValue&)>
 Update)
    {
        return MakeShared<TCallbackTweenValue<TValue>>(StartValue, EndValue, MoveTemp(Update));
    }
}

class NSTWEEN_API FBuilder : public FNsTweenHandleRef
{
public:
    using FNsTweenHandleRef::FNsTweenHandleRef;

    template <typename TValue>
    static FBuilder Create(const TValue& StartValue, const TValue& EndValue, float Duration, ENsTweenEase Ease, TFunction<void(const TValue&)> Update)
    {
        TSharedPtr<FSharedState> State = MakeShared<FSharedState>();
        State->Spec.DurationSeconds = Duration;
        State->Spec.DelaySeconds = 0.f;
        State->Spec.TimeScale = 1.f;
        State->Spec.WrapMode = ENsTweenWrapMode::Once;
        State->Spec.LoopCount = 0;
        State->Spec.Direction = ENsTweenDirection::Forward;
        State->Spec.EasingPreset = Ease;
        State->StrategyFactory = [StartValue, EndValue, Update = MoveTemp(Update)]() mutable -> TSharedPtr<ITweenValue>
        {
            return Internal::MakeCallbackStrategy(StartValue, EndValue, MoveTemp(Update));
        };

        return FBuilder(State);
    }

    FBuilder(const FBuilder& Other) = default;
    FBuilder(FBuilder&& Other) = default;

    ~FBuilder()
    {
        Activate();
    }

    operator FNsTweenHandleRef()
    {
        Activate();
        return FNsTweenHandleRef(State);
    }
};

NSTWEEN_API FBuilder Play(float StartValue, float EndValue, float DurationSeconds, ENsTweenEase Ease, TFunction<void(const float&)> Update);
NSTWEEN_API FBuilder Play(const FVector& StartValue, const FVector& EndValue, float DurationSeconds, ENsTweenEase Ease, TFunction<void(const FVector&)> Update);
NSTWEEN_API FBuilder Play(const FVector2D& StartValue, const FVector2D& EndValue, float DurationSeconds, ENsTweenEase Ease, TFunction<void(const FVector2D&)> Update);
NSTWEEN_API FBuilder Play(const FRotator& StartValue, const FRotator& EndValue, float DurationSeconds, ENsTweenEase Ease, TFunction<void(const FRotator&)> Update);
NSTWEEN_API FBuilder Play(const FQuat& StartValue, const FQuat& EndValue, float DurationSeconds, ENsTweenEase Ease, TFunction<void(const FQuat&)> Update);
NSTWEEN_API FBuilder Play(const FTransform& StartValue, const FTransform& EndValue, float DurationSeconds, ENsTweenEase Ease, TFunction<void(const FTransform&)> Update);
NSTWEEN_API FBuilder Play(const FLinearColor& StartValue, const FLinearColor& EndValue, float DurationSeconds, ENsTweenEase Ease, TFunction<void(const FLinearColor&)> Update);

} // namespace NsTween

namespace NsTween
{
    template <typename TCallable>
    FBuilder Play(float StartValue, float EndValue, float DurationSeconds, ENsTweenEase Ease, TCallable&& Update)
    {
        return Play(StartValue, EndValue, DurationSeconds, Ease, TFunction<void(const float&)>(Forward<TCallable>(Update)));
    }

    template <typename TCallable>
    FBuilder Play(const FVector& StartValue, const FVector& EndValue, float DurationSeconds, ENsTweenEase Ease, TCallable&& Update)
    {
        return Play(StartValue, EndValue, DurationSeconds, Ease, TFunction<void(const FVector&)>(Forward<TCallable>(Update)));
    }

    template <typename TCallable>
    FBuilder Play(const FRotator& StartValue, const FRotator& EndValue, float DurationSeconds, ENsTweenEase Ease, TCallable&& Update)
    {
        return Play(StartValue, EndValue, DurationSeconds, Ease, TFunction<void(const FRotator&)>(Forward<TCallable>(Update)));
    }

    template <typename TCallable>
    FBuilder Play(const FTransform& StartValue, const FTransform& EndValue, float DurationSeconds, ENsTweenEase Ease, TCallable&& Update)
    {
        return Play(StartValue, EndValue, DurationSeconds, Ease, TFunction<void(const FTransform&)>(Forward<TCallable>(Update)));
    }

    template <typename TCallable>
    FBuilder Play(const FLinearColor& StartValue, const FLinearColor& EndValue, float DurationSeconds, ENsTweenEase Ease, TCallable&& Update)
    {
        return Play(StartValue, EndValue, DurationSeconds, Ease, TFunction<void(const FLinearColor&)>(Forward<TCallable>(Update)));
    }
}

using FNsTweenInstance = FNsTweenHandleRef;
using FNsTweenBuilder = NsTween::FBuilder;

