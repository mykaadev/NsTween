// Copyright (C) 2024 mykaa. All rights reserved.

#include "NsTween.h"
#include "Interfaces/ITweenValue.h"
#include "Interfaces/IEasingCurve.h"
#include "Math/UnrealMathUtility.h"
#include "Engine/Engine.h"
#include "NsTweenSubsystem.h"

static constexpr float GNsTweenSmallDelta = 1.e-6f;

static UNsTweenSubsystem* GetTweenSubsystem()
{
    if (!GEngine)
    {
        return nullptr;
    }

    return GEngine->GetEngineSubsystem<UNsTweenSubsystem>();
}

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

FNsTweenBuilder::FNsTweenBuilder()
    : State(nullptr)
{
}

FNsTweenBuilder::FNsTweenBuilder(const TSharedPtr<FState>& InState)
    : State(InState)
{
}

FNsTweenBuilder::~FNsTweenBuilder()
{
    Activate();
}

bool FNsTweenBuilder::CanConfigure() const
{
    return State.IsValid() && !State->bActivated;
}

void FNsTweenBuilder::Activate() const
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

void FNsTweenBuilder::UpdateWrapMode() const
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

FNsTweenBuilder& FNsTweenBuilder::SetPingPong(bool bEnable)
{
    if (CanConfigure())
    {
        State->bPingPong = bEnable;
        UpdateWrapMode();
    }
    return *this;
}

FNsTweenBuilder& FNsTweenBuilder::SetLoops(int32 LoopCount)
{
    if (CanConfigure())
    {
        State->bLooping = (LoopCount != 0);
        State->Spec.LoopCount = LoopCount < 0 ? 0 : FMath::Max(LoopCount, 0);
        UpdateWrapMode();
    }
    return *this;
}

FNsTweenBuilder& FNsTweenBuilder::SetDelay(float DelaySeconds)
{
    if (CanConfigure())
    {
        State->Spec.DelaySeconds = FMath::Max(0.f, DelaySeconds);
    }
    return *this;
}

FNsTweenBuilder& FNsTweenBuilder::SetTimeScale(float TimeScale)
{
    if (CanConfigure())
    {
        State->Spec.TimeScale = FMath::Max(TimeScale, KINDA_SMALL_NUMBER);
    }
    return *this;
}

FNsTweenBuilder& FNsTweenBuilder::SetCurveAsset(UCurveFloat* Curve)
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

FNsTweenBuilder& FNsTweenBuilder::OnComplete(TFunction<void()> Callback)
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

FNsTweenBuilder& FNsTweenBuilder::OnLoop(TFunction<void()> Callback)
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

FNsTweenBuilder& FNsTweenBuilder::OnPingPong(TFunction<void()> Callback)
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

void FNsTweenBuilder::Pause() const
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

void FNsTweenBuilder::Resume() const
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

void FNsTweenBuilder::Cancel(bool bApplyFinal) const
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

bool FNsTweenBuilder::IsActive() const
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

FNsTweenHandle FNsTweenBuilder::GetHandle() const
{
    Activate();
    if (!State.IsValid())
    {
        return FNsTweenHandle();
    }
    return State->Handle;
}

bool FNsTweenBuilder::IsValid() const
{
    Activate();
    return State.IsValid() && State->Handle.IsValid();
}

FNsTweenBuilder FNsTween::Play(float StartValue, float EndValue, float DurationSeconds, ENsTweenEase Ease, TFunction<void(const float&)> Update)
{
    TSharedPtr<FNsTweenBuilder::FState> State = MakeShared<FNsTweenBuilder::FState>();
    State->Spec.DurationSeconds = FMath::Max(DurationSeconds, 0.f);
    State->Spec.DelaySeconds = 0.f;
    State->Spec.TimeScale = 1.f;
    State->Spec.WrapMode = ENsTweenWrapMode::Once;
    State->Spec.LoopCount = 0;
    State->Spec.Direction = ENsTweenDirection::Forward;
    State->Spec.EasingPreset = Ease;
    State->StrategyFactory = [StartValue, EndValue, Update = MoveTemp(Update)]() mutable -> TSharedPtr<ITweenValue>
    {
        return MakeNsTweenCallbackStrategy(StartValue, EndValue, MoveTemp(Update));
    };
    return FNsTweenBuilder(State);
}

FNsTweenBuilder FNsTween::Play(const FVector& StartValue, const FVector& EndValue, float DurationSeconds, ENsTweenEase Ease, T
Function<void(const FVector&)> Update)
{
    TSharedPtr<FNsTweenBuilder::FState> State = MakeShared<FNsTweenBuilder::FState>();
    State->Spec.DurationSeconds = FMath::Max(DurationSeconds, 0.f);
    State->Spec.DelaySeconds = 0.f;
    State->Spec.TimeScale = 1.f;
    State->Spec.WrapMode = ENsTweenWrapMode::Once;
    State->Spec.LoopCount = 0;
    State->Spec.Direction = ENsTweenDirection::Forward;
    State->Spec.EasingPreset = Ease;
    State->StrategyFactory = [StartValue, EndValue, Update = MoveTemp(Update)]() mutable -> TSharedPtr<ITweenValue>
    {
        return MakeNsTweenCallbackStrategy(StartValue, EndValue, MoveTemp(Update));
    };
    return FNsTweenBuilder(State);
}

FNsTweenBuilder FNsTween::Play(const FVector2D& StartValue, const FVector2D& EndValue, float DurationSeconds, ENsTweenEase Ease
, TFunction<void(const FVector2D&)> Update)
{
    TSharedPtr<FNsTweenBuilder::FState> State = MakeShared<FNsTweenBuilder::FState>();
    State->Spec.DurationSeconds = FMath::Max(DurationSeconds, 0.f);
    State->Spec.DelaySeconds = 0.f;
    State->Spec.TimeScale = 1.f;
    State->Spec.WrapMode = ENsTweenWrapMode::Once;
    State->Spec.LoopCount = 0;
    State->Spec.Direction = ENsTweenDirection::Forward;
    State->Spec.EasingPreset = Ease;
    State->StrategyFactory = [StartValue, EndValue, Update = MoveTemp(Update)]() mutable -> TSharedPtr<ITweenValue>
    {
        return MakeNsTweenCallbackStrategy(StartValue, EndValue, MoveTemp(Update));
    };
    return FNsTweenBuilder(State);
}

FNsTweenBuilder FNsTween::Play(const FRotator& StartValue, const FRotator& EndValue, float DurationSeconds, ENsTweenEase Ease, T
Function<void(const FRotator&)> Update)
{
    TSharedPtr<FNsTweenBuilder::FState> State = MakeShared<FNsTweenBuilder::FState>();
    State->Spec.DurationSeconds = FMath::Max(DurationSeconds, 0.f);
    State->Spec.DelaySeconds = 0.f;
    State->Spec.TimeScale = 1.f;
    State->Spec.WrapMode = ENsTweenWrapMode::Once;
    State->Spec.LoopCount = 0;
    State->Spec.Direction = ENsTweenDirection::Forward;
    State->Spec.EasingPreset = Ease;
    State->StrategyFactory = [StartValue, EndValue, Update = MoveTemp(Update)]() mutable -> TSharedPtr<ITweenValue>
    {
        return MakeNsTweenCallbackStrategy(StartValue, EndValue, MoveTemp(Update));
    };
    return FNsTweenBuilder(State);
}

FNsTweenBuilder FNsTween::Play(const FQuat& StartValue, const FQuat& EndValue, float DurationSeconds, ENsTweenEase Ease, TFuncti
on<void(const FQuat&)> Update)
{
    TSharedPtr<FNsTweenBuilder::FState> State = MakeShared<FNsTweenBuilder::FState>();
    State->Spec.DurationSeconds = FMath::Max(DurationSeconds, 0.f);
    State->Spec.DelaySeconds = 0.f;
    State->Spec.TimeScale = 1.f;
    State->Spec.WrapMode = ENsTweenWrapMode::Once;
    State->Spec.LoopCount = 0;
    State->Spec.Direction = ENsTweenDirection::Forward;
    State->Spec.EasingPreset = Ease;
    State->StrategyFactory = [StartValue, EndValue, Update = MoveTemp(Update)]() mutable -> TSharedPtr<ITweenValue>
    {
        return MakeNsTweenCallbackStrategy(StartValue, EndValue, MoveTemp(Update));
    };
    return FNsTweenBuilder(State);
}

FNsTweenBuilder FNsTween::Play(const FTransform& StartValue, const FTransform& EndValue, float DurationSeconds, ENsTweenEase Eas
e, TFunction<void(const FTransform&)> Update)
{
    TSharedPtr<FNsTweenBuilder::FState> State = MakeShared<FNsTweenBuilder::FState>();
    State->Spec.DurationSeconds = FMath::Max(DurationSeconds, 0.f);
    State->Spec.DelaySeconds = 0.f;
    State->Spec.TimeScale = 1.f;
    State->Spec.WrapMode = ENsTweenWrapMode::Once;
    State->Spec.LoopCount = 0;
    State->Spec.Direction = ENsTweenDirection::Forward;
    State->Spec.EasingPreset = Ease;
    State->StrategyFactory = [StartValue, EndValue, Update = MoveTemp(Update)]() mutable -> TSharedPtr<ITweenValue>
    {
        return MakeNsTweenCallbackStrategy(StartValue, EndValue, MoveTemp(Update));
    };
    return FNsTweenBuilder(State);
}

FNsTweenBuilder FNsTween::Play(const FLinearColor& StartValue, const FLinearColor& EndValue, float DurationSeconds, ENsTweenEase
 Ease, TFunction<void(const FLinearColor&)> Update)
{
    TSharedPtr<FNsTweenBuilder::FState> State = MakeShared<FNsTweenBuilder::FState>();
    State->Spec.DurationSeconds = FMath::Max(DurationSeconds, 0.f);
    State->Spec.DelaySeconds = 0.f;
    State->Spec.TimeScale = 1.f;
    State->Spec.WrapMode = ENsTweenWrapMode::Once;
    State->Spec.LoopCount = 0;
    State->Spec.Direction = ENsTweenDirection::Forward;
    State->Spec.EasingPreset = Ease;
    State->StrategyFactory = [StartValue, EndValue, Update = MoveTemp(Update)]() mutable -> TSharedPtr<ITweenValue>
    {
        return MakeNsTweenCallbackStrategy(StartValue, EndValue, MoveTemp(Update));
    };
    return FNsTweenBuilder(State);
}

FNsTween::FNsTween(const FNsTweenHandle& InHandle, FNsTweenSpec InSpec, TSharedPtr<ITweenValue> InStrategy, TSharedPtr<IEasingCurve> InEasing)
    : Handle(InHandle)
    , Spec(MoveTemp(InSpec))
    , Strategy(MoveTemp(InStrategy))
    , Easing(MoveTemp(InEasing))
    , DelayRemaining(FMath::Max(0.f, InSpec.DelaySeconds))
    , CycleTime(InSpec.Direction == ENsTweenDirection::Forward ? 0.f : FMath::Max(InSpec.DurationSeconds, GNsTweenSmallDelta))
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
    if (ScaledDelta <= GNsTweenSmallDelta)
    {
        return true;
    }

    if (DelayRemaining > GNsTweenSmallDelta)
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
    float Duration = FMath::Max(Spec.DurationSeconds, GNsTweenSmallDelta);

    while (RemainingTime > GNsTweenSmallDelta && bActive)
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

    const float Duration = FMath::Max(Spec.DurationSeconds, GNsTweenSmallDelta);
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
    float Duration = FMath::Max(Spec.DurationSeconds, GNsTweenSmallDelta);

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

