#include "NsTweenAsyncAction.h"

#include "Curves/CurveFloat.h"
#include "Engine/Engine.h"
#include "Engine/GameInstance.h"
#include "Engine/World.h"
#include "NsTweenCore.h"
#include "UObject/Stack.h"
#include "Utils/NsTweenLogging.h"

namespace
{
UObject* ResolveWorldContext(UObject* WorldContextObject)
{
    if (WorldContextObject)
    {
        return WorldContextObject;
    }

    if (!GEngine)
    {
        return nullptr;
    }

    for (const FWorldContext& Context : GEngine->GetWorldContexts())
    {
        if (const UWorld* World = Context.World())
        {
            if (UGameInstance* GameInstance = World->GetGameInstance())
            {
                return GameInstance;
            }

            return const_cast<UWorld*>(World);
        }
    }

    return nullptr;
}
}

void UNsTweenAsyncAction::InitialiseCommon(UObject* WorldContextObject,
    float InDuration,
    ENsTweenEase InEase,
    float InDelay,
    int32 InLoops,
    float InLoopDelay,
    bool bInPingPong,
    float InPingPongDelay,
    bool bInCanTickDuringPause,
    bool bInUseGlobalTimeDilation,
    UCurveFloat* InCurve,
    bool bInUseCustomCurve)
{
    WorldContext = WorldContextObject;
    DurationSeconds = FMath::Max(0.f, InDuration);
    DelaySeconds = FMath::Max(0.f, InDelay);
    LoopCount = InLoops;
    LoopDelaySeconds = FMath::Max(0.f, InLoopDelay);
    bPingPong = bInPingPong;
    PingPongDelaySeconds = FMath::Max(0.f, InPingPongDelay);
    bCanTickDuringPause = bInCanTickDuringPause;
    bUseGlobalTimeDilation = bInUseGlobalTimeDilation;
    EaseType = InEase;
    CurveOverride = InCurve;
    bUseCurveOverride = bInUseCustomCurve && InCurve != nullptr;
    TimeMultiplier = 1.f;
    ActiveTween = FNsTweenHandleRef();
    bHasActivated = false;
}

ENsTweenEase UNsTweenAsyncAction::GetEffectiveEase() const
{
    if (bUseCurveOverride && CurveOverride)
    {
        return ENsTweenEase::CurveAsset;
    }

    return EaseType;
}

void UNsTweenAsyncAction::ApplyBuilderOptions(NsTween::FBuilder& Builder)
{
    if (bUseCurveOverride && CurveOverride)
    {
        Builder.SetCurveAsset(CurveOverride);
    }

    if (DelaySeconds > 0.f)
    {
        Builder.SetDelay(DelaySeconds);
    }

    if (LoopCount != 0)
    {
        Builder.SetLoops(LoopCount);
    }

    if (bPingPong)
    {
        Builder.SetPingPong(true);
    }

    if (!FMath::IsNearlyEqual(TimeMultiplier, 1.f))
    {
        Builder.SetTimeScale(TimeMultiplier);
    }

    if (LoopDelaySeconds > 0.f)
    {
        UE_LOG(LogNsTween, Warning, TEXT("LoopDelay is not supported for NsTween async actions and will be ignored."));
    }

    if (PingPongDelaySeconds > 0.f)
    {
        UE_LOG(LogNsTween, Warning, TEXT("PingPongDelay is not supported for NsTween async actions and will be ignored."));
    }

    if (bCanTickDuringPause)
    {
        UE_LOG(LogNsTween, Warning, TEXT("Ticking during pause is not supported for NsTween async actions."));
    }

    if (!bUseGlobalTimeDilation)
    {
        UE_LOG(LogNsTween, Warning, TEXT("Ignoring request to disable global time dilation for NsTween async actions."));
    }

    TWeakObjectPtr<UNsTweenAsyncAction> WeakThis(this);
    Builder.OnLoop([WeakThis]()
    {
        if (UNsTweenAsyncAction* StrongThis = WeakThis.Get())
        {
            StrongThis->OnLoop.Broadcast();
        }
    });

    Builder.OnPingPong([WeakThis]()
    {
        if (UNsTweenAsyncAction* StrongThis = WeakThis.Get())
        {
            StrongThis->OnPingPong.Broadcast();
        }
    });

    Builder.OnComplete([WeakThis]()
    {
        if (UNsTweenAsyncAction* StrongThis = WeakThis.Get())
        {
            StrongThis->OnComplete.Broadcast();
            StrongThis->HandleCompletedTween();
        }
    });
}

void UNsTweenAsyncAction::HandleCompletedTween()
{
    ActiveTween = FNsTweenHandleRef();
    SetReadyToDestroy();
}

void UNsTweenAsyncAction::Activate()
{
    if (bHasActivated)
    {
        return;
    }

    bHasActivated = true;

    if (DurationSeconds <= 0.f)
    {
        FFrame::KismetExecutionMessage(TEXT("Tween duration must be greater than zero."), ELogVerbosity::Error);
        HandleCompletedTween();
        return;
    }

    if (bUseCurveOverride && !CurveOverride)
    {
        FFrame::KismetExecutionMessage(TEXT("Custom curve tween requested without a curve asset."), ELogVerbosity::Error);
        HandleCompletedTween();
        return;
    }

    LaunchTween();
}

void UNsTweenAsyncAction::BeginDestroy()
{
    if (ActiveTween.IsValid())
    {
        ActiveTween.Cancel(false);
        ActiveTween = FNsTweenHandleRef();
    }

    Super::BeginDestroy();
}

void UNsTweenAsyncAction::Pause()
{
    if (ActiveTween.IsValid())
    {
        ActiveTween.Pause();
    }
}

void UNsTweenAsyncAction::Resume()
{
    if (ActiveTween.IsValid())
    {
        ActiveTween.Resume();
    }
}

void UNsTweenAsyncAction::Restart()
{
    if (!bHasActivated)
    {
        Activate();
        return;
    }

    if (ActiveTween.IsValid())
    {
        ActiveTween.Cancel(false);
        ActiveTween = FNsTweenHandleRef();
    }

    LaunchTween();
}

void UNsTweenAsyncAction::Stop()
{
    if (ActiveTween.IsValid())
    {
        ActiveTween.Cancel(false);
        ActiveTween = FNsTweenHandleRef();
    }

    SetReadyToDestroy();
}

void UNsTweenAsyncAction::SetTimeMultiplier(float Multiplier)
{
    const float SafeMultiplier = FMath::Max(FMath::Abs(Multiplier), KINDA_SMALL_NUMBER);
    TimeMultiplier = SafeMultiplier;

    if (ActiveTween.IsValid())
    {
        ActiveTween.SetTimeScale(SafeMultiplier);
    }
}

UNsTweenAsyncActionFloat* UNsTweenAsyncActionFloat::TweenFloat(UObject* WorldContextObject, float Start, float End, float DurationSecs, ENsTweenEase EaseType, float Delay, int32 Loops, float LoopDelay, bool bPingPong, float PingPongDelay, bool bCanTickDuringPause, bool bUseGlobalTimeDilation)
{
    UNsTweenAsyncActionFloat* Node = NewObject<UNsTweenAsyncActionFloat>();
    UObject* ContextObject = ResolveWorldContext(WorldContextObject);
    Node->InitialiseCommon(ContextObject, DurationSecs, EaseType, Delay, Loops, LoopDelay, bPingPong, PingPongDelay, bCanTickDuringPause, bUseGlobalTimeDilation, nullptr, false);
    Node->StartValue = Start;
    Node->EndValue = End;
    if (ContextObject)
    {
        Node->RegisterWithGameInstance(ContextObject);
    }
    return Node;
}

UNsTweenAsyncActionFloat* UNsTweenAsyncActionFloat::TweenFloatCustomCurve(UObject* WorldContextObject, float Start, float End, float DurationSecs, UCurveFloat* Curve, float Delay, int32 Loops, float LoopDelay, bool bPingPong, float PingPongDelay, bool bCanTickDuringPause, bool bUseGlobalTimeDilation)
{
    UNsTweenAsyncActionFloat* Node = NewObject<UNsTweenAsyncActionFloat>();
    UObject* ContextObject = ResolveWorldContext(WorldContextObject);
    Node->InitialiseCommon(ContextObject, DurationSecs, ENsTweenEase::Linear, Delay, Loops, LoopDelay, bPingPong, PingPongDelay, bCanTickDuringPause, bUseGlobalTimeDilation, Curve, true);
    Node->StartValue = Start;
    Node->EndValue = End;
    if (ContextObject)
    {
        Node->RegisterWithGameInstance(ContextObject);
    }
    return Node;
}

void UNsTweenAsyncActionFloat::LaunchTween()
{
    TWeakObjectPtr<UNsTweenAsyncActionFloat> WeakThis(this);
    NsTween::FBuilder Builder = NsTween::Play(StartValue, EndValue, DurationSeconds, GetEffectiveEase(), [WeakThis](const float& Value)
    {
        if (UNsTweenAsyncActionFloat* StrongThis = WeakThis.Get())
        {
            StrongThis->ApplyEasing.Broadcast(Value);
        }
    });

    ApplyBuilderOptions(Builder);
    ActiveTween = MoveTemp(Builder);
}

UNsTweenAsyncActionQuat* UNsTweenAsyncActionQuat::TweenQuat(UObject* WorldContextObject, FQuat Start, FQuat End, float DurationSecs, ENsTweenEase EaseType, float Delay, int32 Loops, float LoopDelay, bool bPingPong, float PingPongDelay, bool bCanTickDuringPause, bool bUseGlobalTimeDilation)
{
    UNsTweenAsyncActionQuat* Node = NewObject<UNsTweenAsyncActionQuat>();
    UObject* ContextObject = ResolveWorldContext(WorldContextObject);
    Node->InitialiseCommon(ContextObject, DurationSecs, EaseType, Delay, Loops, LoopDelay, bPingPong, PingPongDelay, bCanTickDuringPause, bUseGlobalTimeDilation, nullptr, false);
    Node->StartValue = Start.GetNormalized();
    Node->EndValue = End.GetNormalized();
    if (ContextObject)
    {
        Node->RegisterWithGameInstance(ContextObject);
    }
    return Node;
}

UNsTweenAsyncActionQuat* UNsTweenAsyncActionQuat::TweenQuatFromRotator(UObject* WorldContextObject, FRotator Start, FRotator End, float DurationSecs, ENsTweenEase EaseType, float Delay, int32 Loops, float LoopDelay, bool bPingPong, float PingPongDelay, bool bCanTickDuringPause, bool bUseGlobalTimeDilation)
{
    return TweenQuat(WorldContextObject, Start.Quaternion(), End.Quaternion(), DurationSecs, EaseType, Delay, Loops, LoopDelay, bPingPong, PingPongDelay, bCanTickDuringPause, bUseGlobalTimeDilation);
}

UNsTweenAsyncActionQuat* UNsTweenAsyncActionQuat::TweenQuatCustomCurve(UObject* WorldContextObject, FQuat Start, FQuat End, float DurationSecs, UCurveFloat* Curve, float Delay, int32 Loops, float LoopDelay, bool bPingPong, float PingPongDelay, bool bCanTickDuringPause, bool bUseGlobalTimeDilation)
{
    UNsTweenAsyncActionQuat* Node = NewObject<UNsTweenAsyncActionQuat>();
    UObject* ContextObject = ResolveWorldContext(WorldContextObject);
    Node->InitialiseCommon(ContextObject, DurationSecs, ENsTweenEase::Linear, Delay, Loops, LoopDelay, bPingPong, PingPongDelay, bCanTickDuringPause, bUseGlobalTimeDilation, Curve, true);
    Node->StartValue = Start.GetNormalized();
    Node->EndValue = End.GetNormalized();
    if (ContextObject)
    {
        Node->RegisterWithGameInstance(ContextObject);
    }
    return Node;
}

UNsTweenAsyncActionQuat* UNsTweenAsyncActionQuat::TweenQuatFromRotatorCustomCurve(UObject* WorldContextObject, FRotator Start, FRotator End, float DurationSecs, UCurveFloat* Curve, float Delay, int32 Loops, float LoopDelay, bool bPingPong, float PingPongDelay, bool bCanTickDuringPause, bool bUseGlobalTimeDilation)
{
    return TweenQuatCustomCurve(WorldContextObject, Start.Quaternion(), End.Quaternion(), DurationSecs, Curve, Delay, Loops, LoopDelay, bPingPong, PingPongDelay, bCanTickDuringPause, bUseGlobalTimeDilation);
}

void UNsTweenAsyncActionQuat::LaunchTween()
{
    TWeakObjectPtr<UNsTweenAsyncActionQuat> WeakThis(this);
    NsTween::FBuilder Builder = NsTween::Play(StartValue, EndValue, DurationSeconds, GetEffectiveEase(), [WeakThis](const FQuat& Value)
    {
        if (UNsTweenAsyncActionQuat* StrongThis = WeakThis.Get())
        {
            StrongThis->ApplyEasing.Broadcast(Value);
        }
    });

    ApplyBuilderOptions(Builder);
    ActiveTween = MoveTemp(Builder);
}

UNsTweenAsyncActionRotator* UNsTweenAsyncActionRotator::TweenRotator(UObject* WorldContextObject, FRotator Start, FRotator End, float DurationSecs, ENsTweenEase EaseType, float Delay, int32 Loops, float LoopDelay, bool bPingPong, float PingPongDelay, bool bCanTickDuringPause, bool bUseGlobalTimeDilation)
{
    UNsTweenAsyncActionRotator* Node = NewObject<UNsTweenAsyncActionRotator>();
    UObject* ContextObject = ResolveWorldContext(WorldContextObject);
    Node->InitialiseCommon(ContextObject, DurationSecs, EaseType, Delay, Loops, LoopDelay, bPingPong, PingPongDelay, bCanTickDuringPause, bUseGlobalTimeDilation, nullptr, false);
    Node->StartQuat = Start.Quaternion();
    Node->EndQuat = End.Quaternion();
    if (ContextObject)
    {
        Node->RegisterWithGameInstance(ContextObject);
    }
    return Node;
}

UNsTweenAsyncActionRotator* UNsTweenAsyncActionRotator::TweenRotatorCustomCurve(UObject* WorldContextObject, FRotator Start, FRotator End, float DurationSecs, UCurveFloat* Curve, float Delay, int32 Loops, float LoopDelay, bool bPingPong, float PingPongDelay, bool bCanTickDuringPause, bool bUseGlobalTimeDilation)
{
    UNsTweenAsyncActionRotator* Node = NewObject<UNsTweenAsyncActionRotator>();
    UObject* ContextObject = ResolveWorldContext(WorldContextObject);
    Node->InitialiseCommon(ContextObject, DurationSecs, ENsTweenEase::Linear, Delay, Loops, LoopDelay, bPingPong, PingPongDelay, bCanTickDuringPause, bUseGlobalTimeDilation, Curve, true);
    Node->StartQuat = Start.Quaternion();
    Node->EndQuat = End.Quaternion();
    if (ContextObject)
    {
        Node->RegisterWithGameInstance(ContextObject);
    }
    return Node;
}

void UNsTweenAsyncActionRotator::LaunchTween()
{
    TWeakObjectPtr<UNsTweenAsyncActionRotator> WeakThis(this);
    NsTween::FBuilder Builder = NsTween::Play(StartQuat, EndQuat, DurationSeconds, GetEffectiveEase(), [WeakThis](const FQuat& Value)
    {
        if (UNsTweenAsyncActionRotator* StrongThis = WeakThis.Get())
        {
            StrongThis->ApplyEasing.Broadcast(Value.Rotator());
        }
    });

    ApplyBuilderOptions(Builder);
    ActiveTween = MoveTemp(Builder);
}

UNsTweenAsyncActionVector* UNsTweenAsyncActionVector::TweenVector(UObject* WorldContextObject, FVector Start, FVector End, float DurationSecs, ENsTweenEase EaseType, float Delay, int32 Loops, float LoopDelay, bool bPingPong, float PingPongDelay, bool bCanTickDuringPause, bool bUseGlobalTimeDilation)
{
    UNsTweenAsyncActionVector* Node = NewObject<UNsTweenAsyncActionVector>();
    UObject* ContextObject = ResolveWorldContext(WorldContextObject);
    Node->InitialiseCommon(ContextObject, DurationSecs, EaseType, Delay, Loops, LoopDelay, bPingPong, PingPongDelay, bCanTickDuringPause, bUseGlobalTimeDilation, nullptr, false);
    Node->StartValue = Start;
    Node->EndValue = End;
    if (ContextObject)
    {
        Node->RegisterWithGameInstance(ContextObject);
    }
    return Node;
}

UNsTweenAsyncActionVector* UNsTweenAsyncActionVector::TweenVectorCustomCurve(UObject* WorldContextObject, FVector Start, FVector End, float DurationSecs, UCurveFloat* Curve, float Delay, int32 Loops, float LoopDelay, bool bPingPong, float PingPongDelay, bool bCanTickDuringPause, bool bUseGlobalTimeDilation)
{
    UNsTweenAsyncActionVector* Node = NewObject<UNsTweenAsyncActionVector>();
    UObject* ContextObject = ResolveWorldContext(WorldContextObject);
    Node->InitialiseCommon(ContextObject, DurationSecs, ENsTweenEase::Linear, Delay, Loops, LoopDelay, bPingPong, PingPongDelay, bCanTickDuringPause, bUseGlobalTimeDilation, Curve, true);
    Node->StartValue = Start;
    Node->EndValue = End;
    if (ContextObject)
    {
        Node->RegisterWithGameInstance(ContextObject);
    }
    return Node;
}

void UNsTweenAsyncActionVector::LaunchTween()
{
    TWeakObjectPtr<UNsTweenAsyncActionVector> WeakThis(this);
    NsTween::FBuilder Builder = NsTween::Play(StartValue, EndValue, DurationSeconds, GetEffectiveEase(), [WeakThis](const FVector& Value)
    {
        if (UNsTweenAsyncActionVector* StrongThis = WeakThis.Get())
        {
            StrongThis->ApplyEasing.Broadcast(Value);
        }
    });

    ApplyBuilderOptions(Builder);
    ActiveTween = MoveTemp(Builder);
}

UNsTweenAsyncActionVector2D* UNsTweenAsyncActionVector2D::TweenVector2D(UObject* WorldContextObject, FVector2D Start, FVector2D End, float DurationSecs, ENsTweenEase EaseType, float Delay, int32 Loops, float LoopDelay, bool bPingPong, float PingPongDelay, bool bCanTickDuringPause, bool bUseGlobalTimeDilation)
{
    UNsTweenAsyncActionVector2D* Node = NewObject<UNsTweenAsyncActionVector2D>();
    UObject* ContextObject = ResolveWorldContext(WorldContextObject);
    Node->InitialiseCommon(ContextObject, DurationSecs, EaseType, Delay, Loops, LoopDelay, bPingPong, PingPongDelay, bCanTickDuringPause, bUseGlobalTimeDilation, nullptr, false);
    Node->StartValue = Start;
    Node->EndValue = End;
    if (ContextObject)
    {
        Node->RegisterWithGameInstance(ContextObject);
    }
    return Node;
}

UNsTweenAsyncActionVector2D* UNsTweenAsyncActionVector2D::TweenVector2DCustomCurve(UObject* WorldContextObject, FVector2D Start, FVector2D End, float DurationSecs, UCurveFloat* Curve, float Delay, int32 Loops, float LoopDelay, bool bPingPong, float PingPongDelay, bool bCanTickDuringPause, bool bUseGlobalTimeDilation)
{
    UNsTweenAsyncActionVector2D* Node = NewObject<UNsTweenAsyncActionVector2D>();
    UObject* ContextObject = ResolveWorldContext(WorldContextObject);
    Node->InitialiseCommon(ContextObject, DurationSecs, ENsTweenEase::Linear, Delay, Loops, LoopDelay, bPingPong, PingPongDelay, bCanTickDuringPause, bUseGlobalTimeDilation, Curve, true);
    Node->StartValue = Start;
    Node->EndValue = End;
    if (ContextObject)
    {
        Node->RegisterWithGameInstance(ContextObject);
    }
    return Node;
}

void UNsTweenAsyncActionVector2D::LaunchTween()
{
    TWeakObjectPtr<UNsTweenAsyncActionVector2D> WeakThis(this);
    NsTween::FBuilder Builder = NsTween::Play(StartValue, EndValue, DurationSeconds, GetEffectiveEase(), [WeakThis](const FVector2D& Value)
    {
        if (UNsTweenAsyncActionVector2D* StrongThis = WeakThis.Get())
        {
            StrongThis->ApplyEasing.Broadcast(Value);
        }
    });

    ApplyBuilderOptions(Builder);
    ActiveTween = MoveTemp(Builder);
}

