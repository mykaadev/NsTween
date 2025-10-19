// Copyright (C) 2025 nulled.softworks. All rights reserved.

#include "NsTweenBuilder.h"
#include "NsTweenSubsystem.h"
#include "Utils/NsTweenProfiling.h"

FNsTweenBuilder::FNsTweenBuilder() = default;

FNsTweenBuilder::FNsTweenBuilder(FNsTweenSpec&& InSpec, TFunction<TSharedPtr<ITweenValue>()>&& InStrategyFactory)
    : Spec(MoveTemp(InSpec))
    , StrategyFactory(MoveTemp(InStrategyFactory))
    , bHasSpec(true)
{
    bLooping = (Spec.WrapMode == ENsTweenWrapMode::Loop) || (Spec.LoopCount != 0);
    bPingPong = (Spec.WrapMode == ENsTweenWrapMode::PingPong);
    UpdateWrapMode();
}

FNsTweenBuilder::FNsTweenBuilder(FNsTweenBuilder&& Other) noexcept = default;

FNsTweenBuilder& FNsTweenBuilder::operator=(FNsTweenBuilder&& Other) noexcept = default;

FNsTweenBuilder::~FNsTweenBuilder()
{
    // Builders are designed to auto-activate on destruction so temporary helpers still spawn tweens.
    Activate();
}

bool FNsTweenBuilder::CanConfigure() const
{
    return bHasSpec && !bActivated;
}

void FNsTweenBuilder::Activate() const
{
    NSTWEEN_SCOPE_CYCLE_COUNTER("NsTweenBuilder::Activate");

    if (!bHasSpec || bActivated)
    {
        return;
    }

    UpdateWrapMode();

    if (!StrategyFactory)
    {
        bActivated = true;
        return;
    }

    TSharedPtr<ITweenValue> Strategy = StrategyFactory();
    StrategyFactory = {};
    if (!Strategy.IsValid())
    {
        bActivated = true;
        return;
    }

    if (UNsTweenSubsystem* Subsystem = UNsTweenSubsystem::GetSubsystem())
    {
        Handle = Subsystem->EnqueueSpawn(Spec, Strategy);
    }

    bActivated = true;
}

void FNsTweenBuilder::UpdateWrapMode() const
{
    NSTWEEN_SCOPE_CYCLE_COUNTER("NsTweenBuilder::UpdateWrapMode");

    if (!bHasSpec)
    {
        return;
    }

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
        const int32 ClampedLoops = FMath::Max(LoopCount, 0);
        Spec.LoopCount = ClampedLoops;
        bLooping = (ClampedLoops != 0);
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

    ConfigureComplete(MoveTemp(Callback));
    return *this;
}

FNsTweenBuilder& FNsTweenBuilder::OnLoop(TFunction<void()> Callback)
{
    NSTWEEN_SCOPE_CYCLE_COUNTER("NsTweenBuilder::OnLoop");

    ConfigureLoop(MoveTemp(Callback));
    return *this;
}

FNsTweenBuilder& FNsTweenBuilder::OnPingPong(TFunction<void()> Callback)
{
    NSTWEEN_SCOPE_CYCLE_COUNTER("NsTweenBuilder::OnPingPong");

    ConfigurePingPong(MoveTemp(Callback));
    return *this;
}

void FNsTweenBuilder::ConfigureComplete(TFunction<void()>&& Callback) const
{
    if (!CanConfigure())
    {
        return;
    }

    if (Callback)
    {
        CompleteCallback = MakeShared<TFunction<void()>>(MoveTemp(Callback));
        Spec.OnComplete.Unbind();
        Spec.OnComplete.BindLambda([CallbackPtr = CompleteCallback]()
        {
            if (CallbackPtr && *CallbackPtr)
            {
                (*CallbackPtr)();
            }
        });
    }
    else
    {
        CompleteCallback.Reset();
        Spec.OnComplete.Unbind();
    }
}

void FNsTweenBuilder::ConfigureLoop(TFunction<void()>&& Callback) const
{
    if (!CanConfigure())
    {
        return;
    }

    if (Callback)
    {
        LoopCallback = MakeShared<TFunction<void()>>(MoveTemp(Callback));
        Spec.OnLoop.Unbind();
        Spec.OnLoop.BindLambda([CallbackPtr = LoopCallback]()
        {
            if (CallbackPtr && *CallbackPtr)
            {
                (*CallbackPtr)();
            }
        });
    }
    else
    {
        LoopCallback.Reset();
        Spec.OnLoop.Unbind();
    }
}

void FNsTweenBuilder::ConfigurePingPong(TFunction<void()>&& Callback) const
{
    if (!CanConfigure())
    {
        return;
    }

    if (Callback)
    {
        PingPongCallback = MakeShared<TFunction<void()>>(MoveTemp(Callback));
        Spec.OnPingPong.Unbind();
        Spec.OnPingPong.BindLambda([CallbackPtr = PingPongCallback]()
        {
            if (CallbackPtr && *CallbackPtr)
            {
                (*CallbackPtr)();
            }
        });
    }
    else
    {
        PingPongCallback.Reset();
        Spec.OnPingPong.Unbind();
    }
}

void FNsTweenBuilder::Pause() const
{
    Activate();

    if (!Handle.IsValid())
    {
        return;
    }

    if (UNsTweenSubsystem* Subsystem = UNsTweenSubsystem::GetSubsystem())
    {
        Subsystem->EnqueuePause(Handle);
    }
}

void FNsTweenBuilder::Resume() const
{
    Activate();

    if (!Handle.IsValid())
    {
        return;
    }

    if (UNsTweenSubsystem* Subsystem = UNsTweenSubsystem::GetSubsystem())
    {
        Subsystem->EnqueueResume(Handle);
    }
}

void FNsTweenBuilder::Cancel(bool bApplyFinal) const
{
    Activate();

    if (!Handle.IsValid())
    {
        return;
    }

    if (UNsTweenSubsystem* Subsystem = UNsTweenSubsystem::GetSubsystem())
    {
        Subsystem->EnqueueCancel(Handle, bApplyFinal);
    }
}

bool FNsTweenBuilder::IsActive() const
{
    Activate();

    if (!Handle.IsValid())
    {
        return false;
    }

    if (UNsTweenSubsystem* Subsystem = UNsTweenSubsystem::GetSubsystem())
    {
        return Subsystem->IsActive(Handle);
    }

    return false;
}

FNsTweenHandle FNsTweenBuilder::GetHandle() const
{
    Activate();
    return Handle;
}

bool FNsTweenBuilder::IsValid() const
{
    Activate();
    return Handle.IsValid();
}
