// Copyright (C) 2024 mykaa. All rights reserved.

#include "NsTween.h"
#include "Interfaces/ITweenValue.h"
#include "Interfaces/IEasingCurve.h"
#include "Math/UnrealMathUtility.h"
#include "Engine/Engine.h"
#include "NsTweenSubsystem.h"

namespace
{
    constexpr float SMALL_DELTA = 1.e-6f;

    UNsTweenSubsystem* GetTweenSubsystem()
    {
        if (!GEngine)
        {
            return nullptr;
        }

        return GEngine->GetEngineSubsystem<UNsTweenSubsystem>();
    }
}

namespace NsTweenInternal
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
            const FQuat Q = FQuat::Slerp(QA, QB, T).GetNormalized();
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

struct FNsTween::FBuilder::FState : public TSharedFromThis<FState>
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

FNsTween::FBuilder::FBuilder()
    : State(nullptr)
{
}

FNsTween::FBuilder::FBuilder(const TSharedPtr<FState>& InState)
    : State(InState)
{
}

FNsTween::FBuilder::~FBuilder()
{
    Activate();
}

bool FNsTween::FBuilder::CanConfigure() const
{
    return State.IsValid() && !State->bActivated;
}

void FNsTween::FBuilder::Activate() const
{
    if (!State.IsValid() || State->bActivated)
    {
        return;
    }

    UpdateWrapMode();

    if (!State->StrategyFactory)
    {
        State->bActivated = true;
        return;
    }

    TSharedPtr<ITweenValue> Strategy = State->StrategyFactory();
    if (!Strategy.IsValid())
    {
        State->bActivated = true;
        return;
    }

    if (UNsTweenSubsystem* Subsystem = GetTweenSubsystem())
    {
        State->Handle = Subsystem->EnqueueSpawn(State->Spec, Strategy);
    }

    State->bActivated = true;
}

void FNsTween::FBuilder::UpdateWrapMode() const
{
    if (!State.IsValid())
    {
        return;
    }

    if (State->bPingPong)
    {
        State->Spec.WrapMode = ENsTweenWrapMode::PingPong;
    }
    else if (State->bLooping)
    {
        State->Spec.WrapMode = ENsTweenWrapMode::Loop;
    }
    else
    {
        State->Spec.WrapMode = ENsTweenWrapMode::Once;
        State->Spec.LoopCount = 0;
    }
}

FNsTween::FBuilder& FNsTween::FBuilder::SetPingPong(bool bEnable)
{
    if (CanConfigure())
    {
        State->bPingPong = bEnable;
        UpdateWrapMode();
    }
    return *this;
}

FNsTween::FBuilder& FNsTween::FBuilder::SetLoops(int32 LoopCount)
{
    if (CanConfigure())
    {
        State->bLooping = (LoopCount != 0);
        State->Spec.LoopCount = LoopCount < 0 ? 0 : FMath::Max(LoopCount, 0);
        UpdateWrapMode();
    }
    return *this;
}

FNsTween::FBuilder& FNsTween::FBuilder::SetDelay(float DelaySeconds)
{
    if (CanConfigure())
    {
        State->Spec.DelaySeconds = FMath::Max(0.f, DelaySeconds);
    }
    return *this;
}

FNsTween::FBuilder& FNsTween::FBuilder::SetTimeScale(float TimeScale)
{
    if (CanConfigure())
    {
        State->Spec.TimeScale = FMath::Max(TimeScale, KINDA_SMALL_NUMBER);
    }
    return *this;
}

FNsTween::FBuilder& FNsTween::FBuilder::SetCurveAsset(UCurveFloat* Curve)
{
    if (CanConfigure())
    {
        State->Spec.CurveAsset = Curve;
        if (Curve)
        {
            State->Spec.EasingPreset = ENsTweenEase::CurveAsset;
        }
    }
    return *this;
}

FNsTween::FBuilder& FNsTween::FBuilder::OnComplete(TFunction<void()> Callback)
{
    if (CanConfigure())
    {
        if (Callback)
        {
            State->CompleteCallback = MakeShared<TFunction<void()>>(MoveTemp(Callback));
            State->Spec.OnComplete.Unbind();
            State->Spec.OnComplete.BindLambda([CallbackPtr = State->CompleteCallback]()
            {
                if (CallbackPtr && *CallbackPtr)
                {
                    (*CallbackPtr)();
                }
            });
        }
        else
        {
            State->CompleteCallback.Reset();
            State->Spec.OnComplete.Unbind();
        }
    }
    return *this;
}

FNsTween::FBuilder& FNsTween::FBuilder::OnLoop(TFunction<void()> Callback)
{
    if (CanConfigure())
    {
        if (Callback)
        {
            State->LoopCallback = MakeShared<TFunction<void()>>(MoveTemp(Callback));
            State->Spec.OnLoop.Unbind();
            State->Spec.OnLoop.BindLambda([CallbackPtr = State->LoopCallback]()
            {
                if (CallbackPtr && *CallbackPtr)
                {
                    (*CallbackPtr)();
                }
            });
        }
        else
        {
            State->LoopCallback.Reset();
            State->Spec.OnLoop.Unbind();
        }
    }
    return *this;
}

FNsTween::FBuilder& FNsTween::FBuilder::OnPingPong(TFunction<void()> Callback)
{
    if (CanConfigure())
    {
        if (Callback)
        {
            State->PingPongCallback = MakeShared<TFunction<void()>>(MoveTemp(Callback));
            State->Spec.OnPingPong.Unbind();
            State->Spec.OnPingPong.BindLambda([CallbackPtr = State->PingPongCallback]()
            {
                if (CallbackPtr && *CallbackPtr)
                {
                    (*CallbackPtr)();
                }
            });
        }
        else
        {
            State->PingPongCallback.Reset();
            State->Spec.OnPingPong.Unbind();
        }
    }
    return *this;
}

void FNsTween::FBuilder::Pause() const
{
    Activate();
    if (State.IsValid() && State->Handle.IsValid())
    {
        if (UNsTweenSubsystem* Subsystem = GetTweenSubsystem())
        {
            Subsystem->EnqueuePause(State->Handle);
        }
    }
}

void FNsTween::FBuilder::Resume() const
{
    Activate();
    if (State.IsValid() && State->Handle.IsValid())
    {
        if (UNsTweenSubsystem* Subsystem = GetTweenSubsystem())
        {
            Subsystem->EnqueueResume(State->Handle);
        }
    }
}

void FNsTween::FBuilder::Cancel(bool bApplyFinal) const
{
    Activate();
    if (State.IsValid() && State->Handle.IsValid())
    {
        if (UNsTweenSubsystem* Subsystem = GetTweenSubsystem())
        {
            Subsystem->EnqueueCancel(State->Handle, bApplyFinal);
        }
    }
}

bool FNsTween::FBuilder::IsActive() const
{
    if (!State.IsValid())
    {
        return false;
    }

    Activate();
    if (UNsTweenSubsystem* Subsystem = GetTweenSubsystem())
    {
        return Subsystem->IsActive(State->Handle);
    }

    return false;
}

FNsTweenHandle FNsTween::FBuilder::GetHandle() const
{
    Activate();
    if (!State.IsValid())
    {
        return FNsTweenHandle();
    }
    return State->Handle;
}

bool FNsTween::FBuilder::IsValid() const
{
    Activate();
    return State.IsValid() && State->Handle.IsValid();
}

FNsTween::FBuilder FNsTween::Play(float StartValue, float EndValue, float DurationSeconds, ENsTweenEase Ease, TFunction<void(const float&)> Update)
{
    TSharedPtr<FBuilder::FState> State = MakeShared<FBuilder::FState>();
    State->Spec.DurationSeconds = FMath::Max(DurationSeconds, 0.f);
    State->Spec.DelaySeconds = 0.f;
    State->Spec.TimeScale = 1.f;
    State->Spec.WrapMode = ENsTweenWrapMode::Once;
    State->Spec.LoopCount = 0;
    State->Spec.Direction = ENsTweenDirection::Forward;
    State->Spec.EasingPreset = Ease;
    State->StrategyFactory = [StartValue, EndValue, Update = MoveTemp(Update)]() mutable -> TSharedPtr<ITweenValue>
    {
        return NsTweenInternal::MakeCallbackStrategy(StartValue, EndValue, MoveTemp(Update));
    };
    return FBuilder(State);
}

FNsTween::FBuilder FNsTween::Play(const FVector& StartValue, const FVector& EndValue, float DurationSeconds, ENsTweenEase Ease, T
Function<void(const FVector&)> Update)
{
    TSharedPtr<FBuilder::FState> State = MakeShared<FBuilder::FState>();
    State->Spec.DurationSeconds = FMath::Max(DurationSeconds, 0.f);
    State->Spec.DelaySeconds = 0.f;
    State->Spec.TimeScale = 1.f;
    State->Spec.WrapMode = ENsTweenWrapMode::Once;
    State->Spec.LoopCount = 0;
    State->Spec.Direction = ENsTweenDirection::Forward;
    State->Spec.EasingPreset = Ease;
    State->StrategyFactory = [StartValue, EndValue, Update = MoveTemp(Update)]() mutable -> TSharedPtr<ITweenValue>
    {
        return NsTweenInternal::MakeCallbackStrategy(StartValue, EndValue, MoveTemp(Update));
    };
    return FBuilder(State);
}

FNsTween::FBuilder FNsTween::Play(const FVector2D& StartValue, const FVector2D& EndValue, float DurationSeconds, ENsTweenEase Ease
, TFunction<void(const FVector2D&)> Update)
{
    TSharedPtr<FBuilder::FState> State = MakeShared<FBuilder::FState>();
    State->Spec.DurationSeconds = FMath::Max(DurationSeconds, 0.f);
    State->Spec.DelaySeconds = 0.f;
    State->Spec.TimeScale = 1.f;
    State->Spec.WrapMode = ENsTweenWrapMode::Once;
    State->Spec.LoopCount = 0;
    State->Spec.Direction = ENsTweenDirection::Forward;
    State->Spec.EasingPreset = Ease;
    State->StrategyFactory = [StartValue, EndValue, Update = MoveTemp(Update)]() mutable -> TSharedPtr<ITweenValue>
    {
        return NsTweenInternal::MakeCallbackStrategy(StartValue, EndValue, MoveTemp(Update));
    };
    return FBuilder(State);
}

FNsTween::FBuilder FNsTween::Play(const FRotator& StartValue, const FRotator& EndValue, float DurationSeconds, ENsTweenEase Ease, T
Function<void(const FRotator&)> Update)
{
    TSharedPtr<FBuilder::FState> State = MakeShared<FBuilder::FState>();
    State->Spec.DurationSeconds = FMath::Max(DurationSeconds, 0.f);
    State->Spec.DelaySeconds = 0.f;
    State->Spec.TimeScale = 1.f;
    State->Spec.WrapMode = ENsTweenWrapMode::Once;
    State->Spec.LoopCount = 0;
    State->Spec.Direction = ENsTweenDirection::Forward;
    State->Spec.EasingPreset = Ease;
    State->StrategyFactory = [StartValue, EndValue, Update = MoveTemp(Update)]() mutable -> TSharedPtr<ITweenValue>
    {
        return NsTweenInternal::MakeCallbackStrategy(StartValue, EndValue, MoveTemp(Update));
    };
    return FBuilder(State);
}

FNsTween::FBuilder FNsTween::Play(const FQuat& StartValue, const FQuat& EndValue, float DurationSeconds, ENsTweenEase Ease, TFuncti
on<void(const FQuat&)> Update)
{
    TSharedPtr<FBuilder::FState> State = MakeShared<FBuilder::FState>();
    State->Spec.DurationSeconds = FMath::Max(DurationSeconds, 0.f);
    State->Spec.DelaySeconds = 0.f;
    State->Spec.TimeScale = 1.f;
    State->Spec.WrapMode = ENsTweenWrapMode::Once;
    State->Spec.LoopCount = 0;
    State->Spec.Direction = ENsTweenDirection::Forward;
    State->Spec.EasingPreset = Ease;
    State->StrategyFactory = [StartValue, EndValue, Update = MoveTemp(Update)]() mutable -> TSharedPtr<ITweenValue>
    {
        return NsTweenInternal::MakeCallbackStrategy(StartValue, EndValue, MoveTemp(Update));
    };
    return FBuilder(State);
}

FNsTween::FBuilder FNsTween::Play(const FTransform& StartValue, const FTransform& EndValue, float DurationSeconds, ENsTweenEase Eas
e, TFunction<void(const FTransform&)> Update)
{
    TSharedPtr<FBuilder::FState> State = MakeShared<FBuilder::FState>();
    State->Spec.DurationSeconds = FMath::Max(DurationSeconds, 0.f);
    State->Spec.DelaySeconds = 0.f;
    State->Spec.TimeScale = 1.f;
    State->Spec.WrapMode = ENsTweenWrapMode::Once;
    State->Spec.LoopCount = 0;
    State->Spec.Direction = ENsTweenDirection::Forward;
    State->Spec.EasingPreset = Ease;
    State->StrategyFactory = [StartValue, EndValue, Update = MoveTemp(Update)]() mutable -> TSharedPtr<ITweenValue>
    {
        return NsTweenInternal::MakeCallbackStrategy(StartValue, EndValue, MoveTemp(Update));
    };
    return FBuilder(State);
}

FNsTween::FBuilder FNsTween::Play(const FLinearColor& StartValue, const FLinearColor& EndValue, float DurationSeconds, ENsTweenEase
 Ease, TFunction<void(const FLinearColor&)> Update)
{
    TSharedPtr<FBuilder::FState> State = MakeShared<FBuilder::FState>();
    State->Spec.DurationSeconds = FMath::Max(DurationSeconds, 0.f);
    State->Spec.DelaySeconds = 0.f;
    State->Spec.TimeScale = 1.f;
    State->Spec.WrapMode = ENsTweenWrapMode::Once;
    State->Spec.LoopCount = 0;
    State->Spec.Direction = ENsTweenDirection::Forward;
    State->Spec.EasingPreset = Ease;
    State->StrategyFactory = [StartValue, EndValue, Update = MoveTemp(Update)]() mutable -> TSharedPtr<ITweenValue>
    {
        return NsTweenInternal::MakeCallbackStrategy(StartValue, EndValue, MoveTemp(Update));
    };
    return FBuilder(State);
}

FNsTween::FNsTween(const FNsTweenHandle& InHandle, FNsTweenSpec InSpec, TSharedPtr<ITweenValue> InStrategy, TSharedPtr<IEasingCurve> InEasing)
    : Handle(InHandle)
    , Spec(MoveTemp(InSpec))
    , Strategy(MoveTemp(InStrategy))
    , Easing(MoveTemp(InEasing))
    , DelayRemaining(FMath::Max(0.f, InSpec.DelaySeconds))
    , CycleTime(InSpec.Direction == ENsTweenDirection::Forward ? 0.f : FMath::Max(InSpec.DurationSeconds, SMALL_DELTA))
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

    const float Duration = FMath::Max(Spec.DurationSeconds, SMALL_DELTA);
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
    float Duration = FMath::Max(Spec.DurationSeconds, SMALL_DELTA);

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

