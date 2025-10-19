// Copyright (C) 2025 nulled.softworks. All rights reserved.

#include "NsTweenBuilder.h"
#include "NsTweenSubsystem.h"
#include "Utils/NsTweenProfiling.h"

namespace
{
    /** Helper used to reset a delegate when no callback storage is provided. */
    template <typename DelegateType>
    void ResetDelegate(DelegateType& Delegate)
    {
        if (Delegate.IsBound())
        {
            Delegate.Unbind();
        }
    }
}

FNsTweenBuilder::FNsTweenBuilder()
{
    Reset();
}

FNsTweenBuilder::FNsTweenBuilder(FNsTweenSpec&& InSpec, TFunction<TSharedPtr<ITweenValue>()>&& InStrategyFactory)
    : Spec(MoveTemp(InSpec))
    , StrategyFactory(MoveTemp(InStrategyFactory))
{
    bHasState = true;
    bActivated = false;
    bLooping = (Spec.WrapMode == ENsTweenWrapMode::Loop);
    bPingPong = (Spec.WrapMode == ENsTweenWrapMode::PingPong);
    UpdateWrapMode();
}

FNsTweenBuilder::FNsTweenBuilder(FNsTweenBuilder&& Other) noexcept
{
    *this = MoveTemp(Other);
}

FNsTweenBuilder& FNsTweenBuilder::operator=(FNsTweenBuilder&& Other) noexcept
{
    if (this != &Other)
    {
        Reset();

        if (Other.bHasState)
        {
            Spec = MoveTemp(Other.Spec);
            StrategyFactory = MoveTemp(Other.StrategyFactory);
            CompleteCallback = MoveTemp(Other.CompleteCallback);
            LoopCallback = MoveTemp(Other.LoopCallback);
            PingPongCallback = MoveTemp(Other.PingPongCallback);
            Handle = MoveTemp(Other.Handle);
            bLooping = Other.bLooping;
            bPingPong = Other.bPingPong;
            bActivated = Other.bActivated;
            bHasState = Other.bHasState;
        }

        Other.Reset();
    }

    return *this;
}

FNsTweenBuilder::~FNsTweenBuilder()
{
    // Builders are designed to auto-activate on destruction so temporary helpers still spawn tweens.
    Activate();
}

void FNsTweenBuilder::Reset()
{
    Spec = FNsTweenSpec();
    StrategyFactory = TFunction<TSharedPtr<ITweenValue>()>();
    CompleteCallback.Reset();
    LoopCallback.Reset();
    PingPongCallback.Reset();
    Handle = FNsTweenHandle();
    bLooping = false;
    bPingPong = false;
    bActivated = false;
    bHasState = false;
}

bool FNsTweenBuilder::CanConfigure() const
{
    NSTWEEN_SCOPE_CYCLE_COUNTER("NsTweenBuilder::CanConfigure");
    // Any configuration step must occur before the builder activates the tween.
    return bHasState && !bActivated;
}

void FNsTweenBuilder::Activate() const
{
    NSTWEEN_SCOPE_CYCLE_COUNTER("NsTweenBuilder::Activate");
    if (!bHasState || bActivated)
    {
        return;
    }

    // Ensure wrap-mode flags are synchronized before we create the runtime strategy.
    UpdateWrapMode();

    if (!StrategyFactory)
    {
        bActivated = true;
        return;
    }

    // The strategy factory runs lazily so callers can defer heavy allocations until activation.
    TSharedPtr<ITweenValue> Strategy = StrategyFactory();
    if (!Strategy.IsValid())
    {
        bActivated = true;
        return;
    }

    // Pushing into the subsystem hands ownership to the runtime manager.
    if (UNsTweenSubsystem* Subsystem = UNsTweenSubsystem::GetSubsystem())
    {
        Handle = Subsystem->EnqueueSpawn(Spec, Strategy);
    }

    bActivated = true;
}

void FNsTweenBuilder::UpdateWrapMode() const
{
    NSTWEEN_SCOPE_CYCLE_COUNTER("NsTweenBuilder::UpdateWrapMode");
    if (!bHasState)
    {
        return;
    }

    // Mirror the fluent helper flags onto the underlying spec so the runtime can act on them.
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

FNsTweenBuilder& FNsTweenBuilder::SetPingPong(bool bEnable)
{
    NSTWEEN_SCOPE_CYCLE_COUNTER("NsTweenBuilder::SetPingPong");
    if (CanConfigure())
    {
        bPingPong = bEnable;
        UpdateWrapMode();
    }
    return *this;
}

FNsTweenBuilder& FNsTweenBuilder::SetLoops(int32 LoopCount)
{
    NSTWEEN_SCOPE_CYCLE_COUNTER("NsTweenBuilder::SetLoops");
    if (CanConfigure())
    {
        bLooping = (LoopCount != 0);
        Spec.LoopCount = LoopCount < 0 ? 0 : FMath::Max(LoopCount, 0);
        UpdateWrapMode();
    }
    return *this;
}

FNsTweenBuilder& FNsTweenBuilder::SetDelay(float DelaySeconds)
{
    NSTWEEN_SCOPE_CYCLE_COUNTER("NsTweenBuilder::SetDelay");
    if (CanConfigure())
    {
        Spec.DelaySeconds = FMath::Max(0.f, DelaySeconds);
    }
    return *this;
}

FNsTweenBuilder& FNsTweenBuilder::SetTimeScale(float TimeScale)
{
    NSTWEEN_SCOPE_CYCLE_COUNTER("NsTweenBuilder::SetTimeScale");
    if (CanConfigure())
    {
        Spec.TimeScale = FMath::Max(TimeScale, KINDA_SMALL_NUMBER);
    }
    return *this;
}

FNsTweenBuilder& FNsTweenBuilder::SetCurveAsset(UCurveFloat* Curve)
{
    NSTWEEN_SCOPE_CYCLE_COUNTER("NsTweenBuilder::SetCurveAsset");
    if (CanConfigure())
    {
        Spec.CurveAsset = Curve;
        if (Curve)
        {
            Spec.EasingPreset = ENsTweenEase::CurveAsset;
        }
    }
    return *this;
}

FNsTweenBuilder& FNsTweenBuilder::OnComplete(TFunction<void()> Callback)
{
    NSTWEEN_SCOPE_CYCLE_COUNTER("NsTweenBuilder::OnComplete");
    if (bHasState)
    {
        ConfigureCallback(MoveTemp(Callback), CompleteCallback, Spec.OnComplete);
    }
    return *this;
}

FNsTweenBuilder& FNsTweenBuilder::OnLoop(TFunction<void()> Callback)
{
    NSTWEEN_SCOPE_CYCLE_COUNTER("NsTweenBuilder::OnLoop");
    if (bHasState)
    {
        ConfigureCallback(MoveTemp(Callback), LoopCallback, Spec.OnLoop);
    }
    return *this;
}

FNsTweenBuilder& FNsTweenBuilder::OnPingPong(TFunction<void()> Callback)
{
    NSTWEEN_SCOPE_CYCLE_COUNTER("NsTweenBuilder::OnPingPong");
    if (bHasState)
    {
        ConfigureCallback(MoveTemp(Callback), PingPongCallback, Spec.OnPingPong);
    }
    return *this;
}

template <typename DelegateType>
void FNsTweenBuilder::ConfigureCallback(TFunction<void()>&& Callback, TSharedPtr<TFunction<void()>>& Storage, DelegateType& Delegate) const
{
    if (!CanConfigure())
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
        ResetDelegate(Delegate);
    }
}

template void FNsTweenBuilder::ConfigureCallback<FNsTweenOnComplete>(TFunction<void()>&& Callback, TSharedPtr<TFunction<void()>>& Storage, FNsTweenOnComplete& Delegate) const;
template void FNsTweenBuilder::ConfigureCallback<FNsTweenOnLoop>(TFunction<void()>&& Callback, TSharedPtr<TFunction<void()>>& Storage, FNsTweenOnLoop& Delegate) const;
template void FNsTweenBuilder::ConfigureCallback<FNsTweenOnPingPong>(TFunction<void()>&& Callback, TSharedPtr<TFunction<void()>>& Storage, FNsTweenOnPingPong& Delegate) const;

void FNsTweenBuilder::Pause() const
{
    Activate();
    if (bHasState && Handle.IsValid())
    {
        if (UNsTweenSubsystem* Subsystem = UNsTweenSubsystem::GetSubsystem())
        {
            Subsystem->EnqueuePause(Handle);
        }
    }
}

void FNsTweenBuilder::Resume() const
{
    Activate();
    if (bHasState && Handle.IsValid())
    {
        if (UNsTweenSubsystem* Subsystem = UNsTweenSubsystem::GetSubsystem())
        {
            Subsystem->EnqueueResume(Handle);
        }
    }
}

void FNsTweenBuilder::Cancel(bool bApplyFinal) const
{
    Activate();
    if (bHasState && Handle.IsValid())
    {
        if (UNsTweenSubsystem* Subsystem = UNsTweenSubsystem::GetSubsystem())
        {
            Subsystem->EnqueueCancel(Handle, bApplyFinal);
        }
    }
}

bool FNsTweenBuilder::IsActive() const
{
    if (!bHasState)
    {
        return false;
    }

    Activate();
    if (UNsTweenSubsystem* Subsystem = UNsTweenSubsystem::GetSubsystem())
    {
        return Subsystem->IsActive(Handle);
    }

    return false;
}

FNsTweenHandle FNsTweenBuilder::GetHandle() const
{
    Activate();
    if (!bHasState)
    {
        return FNsTweenHandle();
    }
    return Handle;
}

bool FNsTweenBuilder::IsValid() const
{
    Activate();
    return bHasState && Handle.IsValid();
}
