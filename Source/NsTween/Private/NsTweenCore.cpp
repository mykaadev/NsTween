// Copyright (C) 2024 mykaa. All rights reserved.

#include "NsTweenCore.h"

#include "Engine/Engine.h"
#include "NsTweenSubsystem.h"

namespace
{
UNsTweenSubsystem* GetSubsystem()
{
    if (!GEngine)
    {
        return nullptr;
    }

    return GEngine->GetEngineSubsystem<UNsTweenSubsystem>();
}
}

FNsTweenHandleRef::FNsTweenHandleRef()
    : State(MakeShared<FSharedState>())
{
}

FNsTweenHandleRef::FNsTweenHandleRef(const TSharedPtr<FSharedState>& InState)
    : State(InState)
{
    if (!State.IsValid())
    {
        State = MakeShared<FSharedState>();
    }
}

bool FNsTweenHandleRef::IsValid() const
{
    Activate();
    return State.IsValid() && State->Handle.IsValid();
}

FNsTweenHandleRef& FNsTweenHandleRef::SetPingPong(bool bEnable)
{
    if (CanConfigure())
    {
        State->bPingPong = bEnable;
        UpdateLoopSettings();
    }
    return *this;
}

FNsTweenHandleRef& FNsTweenHandleRef::SetLoops(int32 LoopCount)
{
    if (CanConfigure())
    {
        State->RequestedLoopCount = LoopCount;
        UpdateLoopSettings();
    }
    return *this;
}

FNsTweenHandleRef& FNsTweenHandleRef::SetDelay(float DelaySeconds)
{
    if (CanConfigure())
    {
        State->Spec.DelaySeconds = FMath::Max(0.f, DelaySeconds);
    }
    return *this;
}

FNsTweenHandleRef& FNsTweenHandleRef::SetTimeScale(float TimeScale)
{
    if (CanConfigure())
    {
        State->Spec.TimeScale = FMath::Max(TimeScale, KINDA_SMALL_NUMBER);
    }
    return *this;
}

FNsTweenHandleRef& FNsTweenHandleRef::OnComplete(TFunction<void()> Callback)
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

FNsTweenHandleRef& FNsTweenHandleRef::OnLoop(TFunction<void()> Callback)
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

FNsTweenHandleRef& FNsTweenHandleRef::OnPingPong(TFunction<void()> Callback)
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

void FNsTweenHandleRef::Pause() const
{
    Activate();
    if (UNsTweenSubsystem* Manager = GetSubsystem())
    {
        Manager->EnqueuePause(State->Handle);
    }
}

void FNsTweenHandleRef::Resume() const
{
    Activate();
    if (UNsTweenSubsystem* Manager = GetSubsystem())
    {
        Manager->EnqueueResume(State->Handle);
    }
}

void FNsTweenHandleRef::Cancel(bool bApplyFinal) const
{
    Activate();
    if (UNsTweenSubsystem* Manager = GetSubsystem())
    {
        Manager->EnqueueCancel(State->Handle, bApplyFinal);
    }
}

bool FNsTweenHandleRef::IsActive() const
{
    if (!State.IsValid())
    {
        return false;
    }

    Activate();
    if (UNsTweenSubsystem* Manager = GetSubsystem())
    {
        return Manager->IsActive(State->Handle);
    }
    return false;
}

FNsTweenHandle FNsTweenHandleRef::GetHandle() const
{
    Activate();
    return State.IsValid() ? State->Handle : FNsTweenHandle();
}

void FNsTweenHandleRef::Activate() const
{
    if (!State.IsValid() || State->bActivated)
    {
        return;
    }

    if (!State->StrategyFactory)
    {
        return;
    }

    if (UNsTweenSubsystem* Manager = GetSubsystem())
    {
        TSharedPtr<ITweenValue> Strategy = State->StrategyFactory();
        if (Strategy.IsValid())
        {
            State->Handle = Manager->EnqueueSpawn(State->Spec, Strategy);
            State->bActivated = State->Handle.IsValid();
        }
    }
}

bool FNsTweenHandleRef::CanConfigure() const
{
    return State.IsValid() && !State->bActivated;
}

void FNsTweenHandleRef::UpdateLoopSettings() const
{
    if (!State.IsValid())
    {
        return;
    }

    if (State->bPingPong)
    {
        State->Spec.WrapMode = ENsTweenWrapMode::PingPong;
    }
    else if (State->RequestedLoopCount != 0)
    {
        State->Spec.WrapMode = ENsTweenWrapMode::Loop;
    }
    else
    {
        State->Spec.WrapMode = ENsTweenWrapMode::Once;
    }

    if (State->Spec.WrapMode == ENsTweenWrapMode::Once)
    {
        State->Spec.LoopCount = 0;
    }
    else
    {
        State->Spec.LoopCount = State->RequestedLoopCount < 0 ? 0 : State->RequestedLoopCount;
    }
}

namespace NsTween
{

FBuilder Play(float StartValue, float EndValue, float DurationSeconds, ENsTweenEase Ease, TFunction<void(const float&)> Update)
{
    return FBuilder::Create(StartValue, EndValue, DurationSeconds, Ease, MoveTemp(Update));
}

FBuilder Play(const FVector& StartValue, const FVector& EndValue, float DurationSeconds, ENsTweenEase Ease, TFunction<void(const FVector&)> Update)
{
    return FBuilder::Create(StartValue, EndValue, DurationSeconds, Ease, MoveTemp(Update));
}

FBuilder Play(const FRotator& StartValue, const FRotator& EndValue, float DurationSeconds, ENsTweenEase Ease, TFunction<void(const FRotator&)> Update)
{
    return FBuilder::Create(StartValue, EndValue, DurationSeconds, Ease, MoveTemp(Update));
}

FBuilder Play(const FTransform& StartValue, const FTransform& EndValue, float DurationSeconds, ENsTweenEase Ease, TFunction<void(const FTransform&)> Update)
{
    return FBuilder::Create(StartValue, EndValue, DurationSeconds, Ease, MoveTemp(Update));
}

FBuilder Play(const FLinearColor& StartValue, const FLinearColor& EndValue, float DurationSeconds, ENsTweenEase Ease, TFunction<void(const FLinearColor&)> Update)
{
    return FBuilder::Create(StartValue, EndValue, DurationSeconds, Ease, MoveTemp(Update));
}

} // namespace NsTween

