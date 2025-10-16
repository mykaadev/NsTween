// Copyright (C) 2024 mykaa. All rights reserved.

#include "Compatibility/NsTweenLegacyCore.h"

#include "Engine/Engine.h"
#include "TweenManager.h"

namespace
{
UNsTweenManagerSubsystem* GetLegacyManager()
{
    if (!GEngine)
    {
        return nullptr;
    }

    return GEngine->GetEngineSubsystem<UNsTweenManagerSubsystem>();
}
}

FNsTweenInstance::FNsTweenInstance()
    : State(MakeShared<FSharedState>())
{
}

FNsTweenInstance::FNsTweenInstance(const TSharedPtr<FSharedState>& InState)
    : State(InState)
{
    if (!State.IsValid())
    {
        State = MakeShared<FSharedState>();
    }
}

bool FNsTweenInstance::IsValid() const
{
    Activate();
    return State.IsValid() && State->Handle.IsValid();
}

FNsTweenInstance& FNsTweenInstance::SetPingPong(bool bEnable)
{
    if (CanConfigure())
    {
        State->bPingPong = bEnable;
        UpdateLoopSettings();
    }
    return *this;
}

FNsTweenInstance& FNsTweenInstance::SetLoops(int32 LoopCount)
{
    if (CanConfigure())
    {
        State->RequestedLoopCount = LoopCount;
        UpdateLoopSettings();
    }
    return *this;
}

FNsTweenInstance& FNsTweenInstance::SetDelay(float DelaySeconds)
{
    if (CanConfigure())
    {
        State->Spec.DelaySeconds = FMath::Max(0.f, DelaySeconds);
    }
    return *this;
}

FNsTweenInstance& FNsTweenInstance::SetTimeScale(float TimeScale)
{
    if (CanConfigure())
    {
        State->Spec.TimeScale = FMath::Max(TimeScale, KINDA_SMALL_NUMBER);
    }
    return *this;
}

FNsTweenInstance& FNsTweenInstance::OnComplete(TFunction<void()> Callback)
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

FNsTweenInstance& FNsTweenInstance::OnLoop(TFunction<void()> Callback)
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

FNsTweenInstance& FNsTweenInstance::OnPingPong(TFunction<void()> Callback)
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

void FNsTweenInstance::Pause() const
{
    Activate();
    if (UNsTweenManagerSubsystem* Manager = GetLegacyManager())
    {
        Manager->EnqueuePause(State->Handle);
    }
}

void FNsTweenInstance::Resume() const
{
    Activate();
    if (UNsTweenManagerSubsystem* Manager = GetLegacyManager())
    {
        Manager->EnqueueResume(State->Handle);
    }
}

void FNsTweenInstance::Cancel(bool bApplyFinal) const
{
    Activate();
    if (UNsTweenManagerSubsystem* Manager = GetLegacyManager())
    {
        Manager->EnqueueCancel(State->Handle, bApplyFinal);
    }
}

bool FNsTweenInstance::IsActive() const
{
    if (!State.IsValid())
    {
        return false;
    }

    Activate();
    if (UNsTweenManagerSubsystem* Manager = GetLegacyManager())
    {
        return Manager->IsActive(State->Handle);
    }
    return false;
}

FNovaTweenHandle FNsTweenInstance::GetHandle() const
{
    Activate();
    return State.IsValid() ? State->Handle : FNovaTweenHandle();
}

void FNsTweenInstance::Activate() const
{
    if (!State.IsValid() || State->bActivated)
    {
        return;
    }

    if (!State->StrategyFactory)
    {
        return;
    }

    if (UNsTweenManagerSubsystem* Manager = GetLegacyManager())
    {
        TSharedPtr<ITweenValue> Strategy = State->StrategyFactory();
        if (Strategy.IsValid())
        {
            State->Handle = Manager->EnqueueSpawn(State->Spec, Strategy);
            State->bActivated = State->Handle.IsValid();
        }
    }
}

bool FNsTweenInstance::CanConfigure() const
{
    return State.IsValid() && !State->bActivated;
}

void FNsTweenInstance::UpdateLoopSettings() const
{
    if (!State.IsValid())
    {
        return;
    }

    if (State->bPingPong)
    {
        State->Spec.WrapMode = ENovaTweenWrapMode::PingPong;
    }
    else if (State->RequestedLoopCount != 0)
    {
        State->Spec.WrapMode = ENovaTweenWrapMode::Loop;
    }
    else
    {
        State->Spec.WrapMode = ENovaTweenWrapMode::Once;
    }

    if (State->Spec.WrapMode == ENovaTweenWrapMode::Once)
    {
        State->Spec.LoopCount = 0;
    }
    else
    {
        State->Spec.LoopCount = State->RequestedLoopCount < 0 ? 0 : State->RequestedLoopCount;
    }
}

ENovaEasingPreset NsTweenCore::Internal::ConvertEase(ENsTweenEase Ease)
{
    switch (Ease)
    {
    default:
    case ENsTweenEase::Linear: return ENovaEasingPreset::Linear;
    case ENsTweenEase::InSine: return ENovaEasingPreset::EaseInSine;
    case ENsTweenEase::OutSine: return ENovaEasingPreset::EaseOutSine;
    case ENsTweenEase::InOutSine: return ENovaEasingPreset::EaseInOutSine;
    case ENsTweenEase::InQuad: return ENovaEasingPreset::EaseInQuad;
    case ENsTweenEase::OutQuad: return ENovaEasingPreset::EaseOutQuad;
    case ENsTweenEase::InOutQuad: return ENovaEasingPreset::EaseInOutQuad;
    case ENsTweenEase::InCubic: return ENovaEasingPreset::EaseInCubic;
    case ENsTweenEase::OutCubic: return ENovaEasingPreset::EaseOutCubic;
    case ENsTweenEase::InOutCubic: return ENovaEasingPreset::EaseInOutCubic;
    case ENsTweenEase::InExpo: return ENovaEasingPreset::EaseInExpo;
    case ENsTweenEase::OutExpo: return ENovaEasingPreset::EaseOutExpo;
    case ENsTweenEase::InOutExpo: return ENovaEasingPreset::EaseInOutExpo;
    }
}

namespace NsTweenCore
{

TTweenBuilder<float> Play(float StartValue, float EndValue, float DurationSeconds, ENsTweenEase Ease, TFunction<void(float)> Update)
{
    return TTweenBuilder<float>(StartValue, EndValue, DurationSeconds, Ease, MoveTemp(Update));
}

TTweenBuilder<FVector> Play(const FVector& StartValue, const FVector& EndValue, float DurationSeconds, ENsTweenEase Ease, TFunction<void(const FVector&)> Update)
{
    return TTweenBuilder<FVector>(StartValue, EndValue, DurationSeconds, Ease, MoveTemp(Update));
}

TTweenBuilder<FRotator> Play(const FRotator& StartValue, const FRotator& EndValue, float DurationSeconds, ENsTweenEase Ease, TFunction<void(const FRotator&)> Update)
{
    return TTweenBuilder<FRotator>(StartValue, EndValue, DurationSeconds, Ease, MoveTemp(Update));
}

TTweenBuilder<FTransform> Play(const FTransform& StartValue, const FTransform& EndValue, float DurationSeconds, ENsTweenEase Ease, TFunction<void(const FTransform&)> Update)
{
    return TTweenBuilder<FTransform>(StartValue, EndValue, DurationSeconds, Ease, MoveTemp(Update));
}

TTweenBuilder<FLinearColor> Play(const FLinearColor& StartValue, const FLinearColor& EndValue, float DurationSeconds, ENsTweenEase Ease, TFunction<void(const FLinearColor&)> Update)
{
    return TTweenBuilder<FLinearColor>(StartValue, EndValue, DurationSeconds, Ease, MoveTemp(Update));
}

} // namespace NsTweenCore

