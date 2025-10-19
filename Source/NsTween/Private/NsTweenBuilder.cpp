// Copyright (C) 2025 nulled.softworks. All rights reserved.

#include "NsTweenBuilder.h"
#include "NsTweenSubsystem.h"
#include "Utils/NsTweenProfiling.h"

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
    // Builders are designed to auto-activate on destruction so temporary helpers still spawn tweens.
    Activate();
}

bool FNsTweenBuilder::CanConfigure() const
{
    NSTWEEN_SCOPE_CYCLE_COUNTER("NsTweenBuilder::CanConfigure");
    // Any configuration step must occur before the builder activates the tween.
    return State.IsValid() && !State->bActivated;
}

void FNsTweenBuilder::Activate() const
{
    NSTWEEN_SCOPE_CYCLE_COUNTER("NsTweenBuilder::Activate");
    if (!State.IsValid() || State->bActivated)
    {
        return;
    }

    // Ensure wrap-mode flags are synchronized before we create the runtime strategy.
    UpdateWrapMode();

    if (!State->StrategyFactory)
    {
        State->bActivated = true;
        return;
    }

    // The strategy factory runs lazily so callers can defer heavy allocations until activation.
    TSharedPtr<ITweenValue> Strategy = State->StrategyFactory();
    if (!Strategy.IsValid())
    {
        State->bActivated = true;
        return;
    }

    // Pushing into the subsystem hands ownership to the runtime manager.
    if (UNsTweenSubsystem* Subsystem = UNsTweenSubsystem::GetSubsystem())
    {
        State->Handle = Subsystem->EnqueueSpawn(State->Spec, Strategy);
    }

    State->bActivated = true;
}

void FNsTweenBuilder::UpdateWrapMode() const
{
    NSTWEEN_SCOPE_CYCLE_COUNTER("NsTweenBuilder::UpdateWrapMode");
    if (!State.IsValid())
    {
        return;
    }

    // Mirror the fluent helper flags onto the underlying spec so the runtime can act on them.
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
    NSTWEEN_SCOPE_CYCLE_COUNTER("NsTweenBuilder::SetPingPong");
    if (CanConfigure())
    {
        State->bPingPong = bEnable;
        UpdateWrapMode();
    }
    return *this;
}

FNsTweenBuilder& FNsTweenBuilder::SetLoops(int32 LoopCount)
{
    NSTWEEN_SCOPE_CYCLE_COUNTER("NsTweenBuilder::SetLoops");
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
    NSTWEEN_SCOPE_CYCLE_COUNTER("NsTweenBuilder::SetDelay");
    if (CanConfigure())
    {
        State->Spec.DelaySeconds = FMath::Max(0.f, DelaySeconds);
    }
    return *this;
}

FNsTweenBuilder& FNsTweenBuilder::SetTimeScale(float TimeScale)
{
    NSTWEEN_SCOPE_CYCLE_COUNTER("NsTweenBuilder::SetTimeScale");
    if (CanConfigure())
    {
        State->Spec.TimeScale = FMath::Max(TimeScale, KINDA_SMALL_NUMBER);
    }
    return *this;
}

FNsTweenBuilder& FNsTweenBuilder::SetCurveAsset(UCurveFloat* Curve)
{
    NSTWEEN_SCOPE_CYCLE_COUNTER("NsTweenBuilder::SetCurveAsset");
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
    NSTWEEN_SCOPE_CYCLE_COUNTER("NsTweenBuilder::OnComplete");
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
    NSTWEEN_SCOPE_CYCLE_COUNTER("NsTweenBuilder::OnLoop");
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
    NSTWEEN_SCOPE_CYCLE_COUNTER("NsTweenBuilder::OnPingPong");
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
        if (UNsTweenSubsystem* Subsystem = UNsTweenSubsystem::GetSubsystem())
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
        if (UNsTweenSubsystem* Subsystem = UNsTweenSubsystem::GetSubsystem())
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
        if (UNsTweenSubsystem* Subsystem = UNsTweenSubsystem::GetSubsystem())
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
    if (UNsTweenSubsystem* Subsystem = UNsTweenSubsystem::GetSubsystem())
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
