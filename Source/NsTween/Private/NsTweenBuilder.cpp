// Copyright (C) 2025 nulled.softworks. All rights reserved.

#include "NsTweenBuilder.h"
#include "NsTweenSubsystem.h"
#include "Utils/NsTweenProfiling.h"

struct FNsTweenBuilder::FState
{
    FState(FNsTweenSpec&& InSpec, TFunction<TSharedPtr<ITweenValue>()>&& InStrategyFactory)
        : Spec(MoveTemp(InSpec))
        , StrategyFactory(MoveTemp(InStrategyFactory))
    {
        bLooping = (Spec.WrapMode == ENsTweenWrapMode::Loop);
        bPingPong = (Spec.WrapMode == ENsTweenWrapMode::PingPong);
        UpdateWrapMode();
    }

    bool CanConfigure() const
    {
        return !bActivated;
    }

    void UpdateWrapMode()
    {
        if (bPingPong)
        {
            Spec.WrapMode = ENsTweenWrapMode::PingPong;
        }
        else if (bLooping)
        {
            Spec.WrapMode = ENsTweenWrapMode::Loop;
        }
        else
        {
            Spec.WrapMode = ENsTweenWrapMode::Once;
            Spec.LoopCount = 0;
        }
    }

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

FNsTweenBuilder::FNsTweenBuilder() = default;

FNsTweenBuilder::FNsTweenBuilder(FNsTweenSpec&& InSpec, TFunction<TSharedPtr<ITweenValue>()>&& InStrategyFactory)
{
    State = MakeUnique<FState>(MoveTemp(InSpec), MoveTemp(InStrategyFactory));
}

FNsTweenBuilder::FNsTweenBuilder(FNsTweenBuilder&& Other) noexcept = default;

FNsTweenBuilder& FNsTweenBuilder::operator=(FNsTweenBuilder&& Other) noexcept = default;

FNsTweenBuilder::~FNsTweenBuilder()
{
    // Builders are designed to auto-activate on destruction so temporary helpers still spawn tweens.
    Activate();
}

void FNsTweenBuilder::Activate() const
{
    NSTWEEN_SCOPE_CYCLE_COUNTER("NsTweenBuilder::Activate");
    if (FState* LocalState = State.Get(); LocalState)
    {
        if (LocalState->bActivated)
        {
            return;
        }

        LocalState->UpdateWrapMode();

        if (!LocalState->StrategyFactory)
        {
            LocalState->bActivated = true;
            return;
        }

        TSharedPtr<ITweenValue> Strategy = LocalState->StrategyFactory();
        LocalState->StrategyFactory = {};
        if (!Strategy.IsValid())
        {
            LocalState->bActivated = true;
            return;
        }

        if (UNsTweenSubsystem* Subsystem = UNsTweenSubsystem::GetSubsystem())
        {
            LocalState->Handle = Subsystem->EnqueueSpawn(LocalState->Spec, Strategy);
        }

        LocalState->bActivated = true;
    }
}

void FNsTweenBuilder::UpdateWrapMode() const
{
    NSTWEEN_SCOPE_CYCLE_COUNTER("NsTweenBuilder::UpdateWrapMode");
    if (FState* LocalState = State.Get(); LocalState)
    {
        LocalState->UpdateWrapMode();
    }
}

FNsTweenBuilder& FNsTweenBuilder::SetPingPong(bool bEnable)
{
    NSTWEEN_SCOPE_CYCLE_COUNTER("NsTweenBuilder::SetPingPong");
    if (FState* LocalState = State.Get(); LocalState && LocalState->CanConfigure())
    {
        LocalState->bPingPong = bEnable;
        LocalState->UpdateWrapMode();
    }
    return *this;
}

FNsTweenBuilder& FNsTweenBuilder::SetLoops(int32 LoopCount)
{
    NSTWEEN_SCOPE_CYCLE_COUNTER("NsTweenBuilder::SetLoops");
    if (FState* LocalState = State.Get(); LocalState && LocalState->CanConfigure())
    {
        LocalState->bLooping = (LoopCount != 0);
        LocalState->Spec.LoopCount = FMath::Max(LoopCount, 0);
        LocalState->UpdateWrapMode();
    }
    return *this;
}

FNsTweenBuilder& FNsTweenBuilder::SetDelay(float DelaySeconds)
{
    NSTWEEN_SCOPE_CYCLE_COUNTER("NsTweenBuilder::SetDelay");
    if (FState* LocalState = State.Get(); LocalState && LocalState->CanConfigure())
    {
        LocalState->Spec.DelaySeconds = FMath::Max(0.f, DelaySeconds);
    }
    return *this;
}

FNsTweenBuilder& FNsTweenBuilder::SetTimeScale(float TimeScale)
{
    NSTWEEN_SCOPE_CYCLE_COUNTER("NsTweenBuilder::SetTimeScale");
    if (FState* LocalState = State.Get(); LocalState && LocalState->CanConfigure())
    {
        LocalState->Spec.TimeScale = FMath::Max(TimeScale, KINDA_SMALL_NUMBER);
    }
    return *this;
}

FNsTweenBuilder& FNsTweenBuilder::SetCurveAsset(UCurveFloat* Curve)
{
    NSTWEEN_SCOPE_CYCLE_COUNTER("NsTweenBuilder::SetCurveAsset");
    if (FState* LocalState = State.Get(); LocalState && LocalState->CanConfigure())
    {
        LocalState->Spec.CurveAsset = Curve;
        if (Curve)
        {
            LocalState->Spec.EasingPreset = ENsTweenEase::CurveAsset;
        }
    }
    return *this;
}

FNsTweenBuilder& FNsTweenBuilder::OnComplete(TFunction<void()> Callback)
{
    NSTWEEN_SCOPE_CYCLE_COUNTER("NsTweenBuilder::OnComplete");
    if (FState* LocalState = State.Get(); LocalState)
    {
        ConfigureCallback(MoveTemp(Callback), LocalState->CompleteCallback, LocalState->Spec.OnComplete);
    }
    return *this;
}

FNsTweenBuilder& FNsTweenBuilder::OnLoop(TFunction<void()> Callback)
{
    NSTWEEN_SCOPE_CYCLE_COUNTER("NsTweenBuilder::OnLoop");
    if (FState* LocalState = State.Get(); LocalState)
    {
        ConfigureCallback(MoveTemp(Callback), LocalState->LoopCallback, LocalState->Spec.OnLoop);
    }
    return *this;
}

FNsTweenBuilder& FNsTweenBuilder::OnPingPong(TFunction<void()> Callback)
{
    NSTWEEN_SCOPE_CYCLE_COUNTER("NsTweenBuilder::OnPingPong");
    if (FState* LocalState = State.Get(); LocalState)
    {
        ConfigureCallback(MoveTemp(Callback), LocalState->PingPongCallback, LocalState->Spec.OnPingPong);
    }
    return *this;
}

template <typename DelegateType>
void FNsTweenBuilder::ConfigureCallback(TFunction<void()>&& Callback, TSharedPtr<TFunction<void()>>& Storage, DelegateType& Delegate) const
{
    FState* LocalState = State.Get();
    if (!LocalState || !LocalState->CanConfigure())
    {
        return;
    }

    if (Callback)
    {
        Storage = MakeShared<TFunction<void()>>(MoveTemp(Callback));
        Delegate.Unbind();
        Delegate.BindLambda([CallbackPtr = Storage]()
        {
            if (CallbackPtr && *CallbackPtr)
            {
                (*CallbackPtr)();
            }
        });
    }
    else
    {
        Storage.Reset();
        Delegate.Unbind();
    }
}

void FNsTweenBuilder::WithHandle(TFunctionRef<void(UNsTweenSubsystem&, const FNsTweenHandle&)> Operation) const
{
    Activate();
    if (FState* LocalState = State.Get(); LocalState && LocalState->Handle.IsValid())
    {
        if (UNsTweenSubsystem* Subsystem = UNsTweenSubsystem::GetSubsystem())
        {
            Operation(*Subsystem, LocalState->Handle);
        }
    }
}

template void FNsTweenBuilder::ConfigureCallback<FNsTweenOnComplete>(TFunction<void()>&& Callback, TSharedPtr<TFunction<void()>>& Storage, FNsTweenOnComplete& Delegate) const;
template void FNsTweenBuilder::ConfigureCallback<FNsTweenOnLoop>(TFunction<void()>&& Callback, TSharedPtr<TFunction<void()>>& Storage, FNsTweenOnLoop& Delegate) const;
template void FNsTweenBuilder::ConfigureCallback<FNsTweenOnPingPong>(TFunction<void()>&& Callback, TSharedPtr<TFunction<void()>>& Storage, FNsTweenOnPingPong& Delegate) const;

void FNsTweenBuilder::Pause() const
{
    WithHandle([](UNsTweenSubsystem& Subsystem, const FNsTweenHandle& Handle)
    {
        Subsystem.EnqueuePause(Handle);
    });
}

void FNsTweenBuilder::Resume() const
{
    WithHandle([](UNsTweenSubsystem& Subsystem, const FNsTweenHandle& Handle)
    {
        Subsystem.EnqueueResume(Handle);
    });
}

void FNsTweenBuilder::Cancel(bool bApplyFinal) const
{
    WithHandle([bApplyFinal](UNsTweenSubsystem& Subsystem, const FNsTweenHandle& Handle)
    {
        Subsystem.EnqueueCancel(Handle, bApplyFinal);
    });
}

bool FNsTweenBuilder::IsActive() const
{
    bool bIsActive = false;
    WithHandle([&bIsActive](UNsTweenSubsystem& Subsystem, const FNsTweenHandle& Handle)
    {
        bIsActive = Subsystem.IsActive(Handle);
    });
    return bIsActive;
}

FNsTweenHandle FNsTweenBuilder::GetHandle() const
{
    Activate();
    if (FState* LocalState = State.Get(); LocalState)
    {
        return LocalState->Handle;
    }
    return FNsTweenHandle();
}

bool FNsTweenBuilder::IsValid() const
{
    Activate();
    if (FState* LocalState = State.Get(); LocalState)
    {
        return LocalState->Handle.IsValid();
    }
    return false;
}
