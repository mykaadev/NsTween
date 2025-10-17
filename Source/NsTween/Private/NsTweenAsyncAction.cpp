// Copyright (C) 2025 nulled.softworks. All rights reserved.

#include "NsTweenAsyncAction.h"

#include "Curves/CurveFloat.h"
#include "Engine/Engine.h"
#include "Engine/GameInstance.h"
#include "Engine/World.h"
#include "NsTween.h"
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

template <typename TAction, typename TInitializer>
TAction* CreateAsyncNode(UObject* WorldContextObject,
    float DurationSecs,
    ENsTweenEase EaseType,
    float Delay,
    int32 Loops,
    float LoopDelay,
    bool bPingPong,
    float PingPongDelay,
    bool bCanTickDuringPause,
    bool bUseGlobalTimeDilation,
    UCurveFloat* Curve,
    bool bUseCustomCurve,
    TInitializer&& Initializer)
{
    TAction* Node = NewObject<TAction>();
    UObject* ContextObject = ResolveWorldContext(WorldContextObject);
    Node->InitialiseCommon(ContextObject, DurationSecs, EaseType, Delay, Loops, LoopDelay, bPingPong, PingPongDelay, bCanTickDuringPause, bUseGlobalTimeDilation, Curve, bUseCustomCurve);
    Forward<TInitializer>(Initializer)(*Node);

    if (ContextObject)
    {
        Node->RegisterWithGameInstance(ContextObject);
    }

    return Node;
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
    ActiveTween = FNsTweenBuilder();
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

void UNsTweenAsyncAction::ApplyBuilderOptions(FNsTweenBuilder& Builder)
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
    ActiveTween = FNsTweenBuilder();
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
    }

    ActiveTween = FNsTweenBuilder();

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
        ActiveTween = FNsTweenBuilder();
    }

    LaunchTween();
}

void UNsTweenAsyncAction::Stop()
{
    if (ActiveTween.IsValid())
    {
        ActiveTween.Cancel(false);
        ActiveTween = FNsTweenBuilder();
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
    return CreateAsyncNode<UNsTweenAsyncActionFloat>(WorldContextObject, DurationSecs, EaseType, Delay, Loops, LoopDelay, bPingPong, PingPongDelay, bCanTickDuringPause, bUseGlobalTimeDilation, nullptr, false, [Start, End](UNsTweenAsyncActionFloat& Node)
    {
        Node.StartValue = Start;
        Node.EndValue = End;
    });
}

UNsTweenAsyncActionFloat* UNsTweenAsyncActionFloat::TweenFloatCustomCurve(UObject* WorldContextObject, float Start, float End, float DurationSecs, UCurveFloat* Curve, float Delay, int32 Loops, float LoopDelay, bool bPingPong, float PingPongDelay, bool bCanTickDuringPause, bool bUseGlobalTimeDilation)
{
    return CreateAsyncNode<UNsTweenAsyncActionFloat>(WorldContextObject, DurationSecs, ENsTweenEase::Linear, Delay, Loops, LoopDelay, bPingPong, PingPongDelay, bCanTickDuringPause, bUseGlobalTimeDilation, Curve, true, [Start, End](UNsTweenAsyncActionFloat& Node)
    {
        Node.StartValue = Start;
        Node.EndValue = End;
    });
}

void UNsTweenAsyncActionFloat::LaunchTween()
{
    StartTypedTween(this, StartValue, EndValue, [](UNsTweenAsyncActionFloat& Action, const float& Value)
    {
        Action.ApplyEasing.Broadcast(Value);
    });
}

UNsTweenAsyncActionQuat* UNsTweenAsyncActionQuat::TweenQuat(UObject* WorldContextObject, FQuat Start, FQuat End, float DurationSecs, ENsTweenEase EaseType, float Delay, int32 Loops, float LoopDelay, bool bPingPong, float PingPongDelay, bool bCanTickDuringPause, bool bUseGlobalTimeDilation)
{
    return CreateAsyncNode<UNsTweenAsyncActionQuat>(WorldContextObject, DurationSecs, EaseType, Delay, Loops, LoopDelay, bPingPong, PingPongDelay, bCanTickDuringPause, bUseGlobalTimeDilation, nullptr, false, [Start, End](UNsTweenAsyncActionQuat& Node)
    {
        Node.StartValue = Start.GetNormalized();
        Node.EndValue = End.GetNormalized();
    });
}

UNsTweenAsyncActionQuat* UNsTweenAsyncActionQuat::TweenQuatFromRotator(UObject* WorldContextObject, FRotator Start, FRotator End, float DurationSecs, ENsTweenEase EaseType, float Delay, int32 Loops, float LoopDelay, bool bPingPong, float PingPongDelay, bool bCanTickDuringPause, bool bUseGlobalTimeDilation)
{
    return TweenQuat(WorldContextObject, Start.Quaternion(), End.Quaternion(), DurationSecs, EaseType, Delay, Loops, LoopDelay, bPingPong, PingPongDelay, bCanTickDuringPause, bUseGlobalTimeDilation);
}

UNsTweenAsyncActionQuat* UNsTweenAsyncActionQuat::TweenQuatCustomCurve(UObject* WorldContextObject, FQuat Start, FQuat End, float DurationSecs, UCurveFloat* Curve, float Delay, int32 Loops, float LoopDelay, bool bPingPong, float PingPongDelay, bool bCanTickDuringPause, bool bUseGlobalTimeDilation)
{
    return CreateAsyncNode<UNsTweenAsyncActionQuat>(WorldContextObject, DurationSecs, ENsTweenEase::Linear, Delay, Loops, LoopDelay, bPingPong, PingPongDelay, bCanTickDuringPause, bUseGlobalTimeDilation, Curve, true, [Start, End](UNsTweenAsyncActionQuat& Node)
    {
        Node.StartValue = Start.GetNormalized();
        Node.EndValue = End.GetNormalized();
    });
}

UNsTweenAsyncActionQuat* UNsTweenAsyncActionQuat::TweenQuatFromRotatorCustomCurve(UObject* WorldContextObject, FRotator Start, FRotator End, float DurationSecs, UCurveFloat* Curve, float Delay, int32 Loops, float LoopDelay, bool bPingPong, float PingPongDelay, bool bCanTickDuringPause, bool bUseGlobalTimeDilation)
{
    return TweenQuatCustomCurve(WorldContextObject, Start.Quaternion(), End.Quaternion(), DurationSecs, Curve, Delay, Loops, LoopDelay, bPingPong, PingPongDelay, bCanTickDuringPause, bUseGlobalTimeDilation);
}

void UNsTweenAsyncActionQuat::LaunchTween()
{
    StartTypedTween(this, StartValue, EndValue, [](UNsTweenAsyncActionQuat& Action, const FQuat& Value)
    {
        Action.ApplyEasing.Broadcast(Value);
    });
}

UNsTweenAsyncActionRotator* UNsTweenAsyncActionRotator::TweenRotator(UObject* WorldContextObject, FRotator Start, FRotator End, float DurationSecs, ENsTweenEase EaseType, float Delay, int32 Loops, float LoopDelay, bool bPingPong, float PingPongDelay, bool bCanTickDuringPause, bool bUseGlobalTimeDilation)
{
    return CreateAsyncNode<UNsTweenAsyncActionRotator>(WorldContextObject, DurationSecs, EaseType, Delay, Loops, LoopDelay, bPingPong, PingPongDelay, bCanTickDuringPause, bUseGlobalTimeDilation, nullptr, false, [Start, End](UNsTweenAsyncActionRotator& Node)
    {
        Node.StartQuat = Start.Quaternion();
        Node.EndQuat = End.Quaternion();
    });
}

UNsTweenAsyncActionRotator* UNsTweenAsyncActionRotator::TweenRotatorCustomCurve(UObject* WorldContextObject, FRotator Start, FRotator End, float DurationSecs, UCurveFloat* Curve, float Delay, int32 Loops, float LoopDelay, bool bPingPong, float PingPongDelay, bool bCanTickDuringPause, bool bUseGlobalTimeDilation)
{
    return CreateAsyncNode<UNsTweenAsyncActionRotator>(WorldContextObject, DurationSecs, ENsTweenEase::Linear, Delay, Loops, LoopDelay, bPingPong, PingPongDelay, bCanTickDuringPause, bUseGlobalTimeDilation, Curve, true, [Start, End](UNsTweenAsyncActionRotator& Node)
    {
        Node.StartQuat = Start.Quaternion();
        Node.EndQuat = End.Quaternion();
    });
}

void UNsTweenAsyncActionRotator::LaunchTween()
{
    StartTypedTween(this, StartQuat, EndQuat, [](UNsTweenAsyncActionRotator& Action, const FQuat& Value)
    {
        Action.ApplyEasing.Broadcast(Value.Rotator());
    });
}

UNsTweenAsyncActionVector* UNsTweenAsyncActionVector::TweenVector(UObject* WorldContextObject, FVector Start, FVector End, float DurationSecs, ENsTweenEase EaseType, float Delay, int32 Loops, float LoopDelay, bool bPingPong, float PingPongDelay, bool bCanTickDuringPause, bool bUseGlobalTimeDilation)
{
    return CreateAsyncNode<UNsTweenAsyncActionVector>(WorldContextObject, DurationSecs, EaseType, Delay, Loops, LoopDelay, bPingPong, PingPongDelay, bCanTickDuringPause, bUseGlobalTimeDilation, nullptr, false, [Start, End](UNsTweenAsyncActionVector& Node)
    {
        Node.StartValue = Start;
        Node.EndValue = End;
    });
}

UNsTweenAsyncActionVector* UNsTweenAsyncActionVector::TweenVectorCustomCurve(UObject* WorldContextObject, FVector Start, FVector End, float DurationSecs, UCurveFloat* Curve, float Delay, int32 Loops, float LoopDelay, bool bPingPong, float PingPongDelay, bool bCanTickDuringPause, bool bUseGlobalTimeDilation)
{
    return CreateAsyncNode<UNsTweenAsyncActionVector>(WorldContextObject, DurationSecs, ENsTweenEase::Linear, Delay, Loops, LoopDelay, bPingPong, PingPongDelay, bCanTickDuringPause, bUseGlobalTimeDilation, Curve, true, [Start, End](UNsTweenAsyncActionVector& Node)
    {
        Node.StartValue = Start;
        Node.EndValue = End;
    });
}

void UNsTweenAsyncActionVector::LaunchTween()
{
    StartTypedTween(this, StartValue, EndValue, [](UNsTweenAsyncActionVector& Action, const FVector& Value)
    {
        Action.ApplyEasing.Broadcast(Value);
    });
}

UNsTweenAsyncActionVector2D* UNsTweenAsyncActionVector2D::TweenVector2D(UObject* WorldContextObject, FVector2D Start, FVector2D End, float DurationSecs, ENsTweenEase EaseType, float Delay, int32 Loops, float LoopDelay, bool bPingPong, float PingPongDelay, bool bCanTickDuringPause, bool bUseGlobalTimeDilation)
{
    return CreateAsyncNode<UNsTweenAsyncActionVector2D>(WorldContextObject, DurationSecs, EaseType, Delay, Loops, LoopDelay, bPingPong, PingPongDelay, bCanTickDuringPause, bUseGlobalTimeDilation, nullptr, false, [Start, End](UNsTweenAsyncActionVector2D& Node)
    {
        Node.StartValue = Start;
        Node.EndValue = End;
    });
}

UNsTweenAsyncActionVector2D* UNsTweenAsyncActionVector2D::TweenVector2DCustomCurve(UObject* WorldContextObject, FVector2D Start, FVector2D End, float DurationSecs, UCurveFloat* Curve, float Delay, int32 Loops, float LoopDelay, bool bPingPong, float PingPongDelay, bool bCanTickDuringPause, bool bUseGlobalTimeDilation)
{
    return CreateAsyncNode<UNsTweenAsyncActionVector2D>(WorldContextObject, DurationSecs, ENsTweenEase::Linear, Delay, Loops, LoopDelay, bPingPong, PingPongDelay, bCanTickDuringPause, bUseGlobalTimeDilation, Curve, true, [Start, End](UNsTweenAsyncActionVector2D& Node)
    {
        Node.StartValue = Start;
        Node.EndValue = End;
    });
}

void UNsTweenAsyncActionVector2D::LaunchTween()
{
    StartTypedTween(this, StartValue, EndValue, [](UNsTweenAsyncActionVector2D& Action, const FVector2D& Value)
    {
        Action.ApplyEasing.Broadcast(Value);
    });
}

UNsTweenAsyncActionTransform* UNsTweenAsyncActionTransform::TweenTransform(UObject* WorldContextObject, const FTransform& Start, const FTransform& End, float DurationSecs, ENsTweenEase EaseType, float Delay, int32 Loops, float LoopDelay, bool bPingPong, float PingPongDelay, bool bCanTickDuringPause, bool bUseGlobalTimeDilation)
{
    return CreateAsyncNode<UNsTweenAsyncActionTransform>(WorldContextObject, DurationSecs, EaseType, Delay, Loops, LoopDelay, bPingPong, PingPongDelay, bCanTickDuringPause, bUseGlobalTimeDilation, nullptr, false, [Start, End](UNsTweenAsyncActionTransform& Node)
    {
        Node.StartValue = Start;
        Node.EndValue = End;
    });
}

UNsTweenAsyncActionTransform* UNsTweenAsyncActionTransform::TweenTransformCustomCurve(UObject* WorldContextObject, const FTransform& Start, const FTransform& End, float DurationSecs, UCurveFloat* Curve, float Delay, int32 Loops, float LoopDelay, bool bPingPong, float PingPongDelay, bool bCanTickDuringPause, bool bUseGlobalTimeDilation)
{
    return CreateAsyncNode<UNsTweenAsyncActionTransform>(WorldContextObject, DurationSecs, ENsTweenEase::Linear, Delay, Loops, LoopDelay, bPingPong, PingPongDelay, bCanTickDuringPause, bUseGlobalTimeDilation, Curve, true, [Start, End](UNsTweenAsyncActionTransform& Node)
    {
        Node.StartValue = Start;
        Node.EndValue = End;
    });
}

void UNsTweenAsyncActionTransform::LaunchTween()
{
    StartTypedTween(this, StartValue, EndValue, [](UNsTweenAsyncActionTransform& Action, const FTransform& Value)
    {
        Action.ApplyEasing.Broadcast(Value);
    });
}

UNsTweenAsyncActionLinearColor* UNsTweenAsyncActionLinearColor::TweenLinearColor(UObject* WorldContextObject, FLinearColor Start, FLinearColor End, float DurationSecs, ENsTweenEase EaseType, float Delay, int32 Loops, float LoopDelay, bool bPingPong, float PingPongDelay, bool bCanTickDuringPause, bool bUseGlobalTimeDilation)
{
    return CreateAsyncNode<UNsTweenAsyncActionLinearColor>(WorldContextObject, DurationSecs, EaseType, Delay, Loops, LoopDelay, bPingPong, PingPongDelay, bCanTickDuringPause, bUseGlobalTimeDilation, nullptr, false, [Start, End](UNsTweenAsyncActionLinearColor& Node)
    {
        Node.StartValue = Start;
        Node.EndValue = End;
    });
}

UNsTweenAsyncActionLinearColor* UNsTweenAsyncActionLinearColor::TweenLinearColorCustomCurve(UObject* WorldContextObject, FLinearColor Start, FLinearColor End, float DurationSecs, UCurveFloat* Curve, float Delay, int32 Loops, float LoopDelay, bool bPingPong, float PingPongDelay, bool bCanTickDuringPause, bool bUseGlobalTimeDilation)
{
    return CreateAsyncNode<UNsTweenAsyncActionLinearColor>(WorldContextObject, DurationSecs, ENsTweenEase::Linear, Delay, Loops, LoopDelay, bPingPong, PingPongDelay, bCanTickDuringPause, bUseGlobalTimeDilation, Curve, true, [Start, End](UNsTweenAsyncActionLinearColor& Node)
    {
        Node.StartValue = Start;
        Node.EndValue = End;
    });
}

void UNsTweenAsyncActionLinearColor::LaunchTween()
{
    StartTypedTween(this, StartValue, EndValue, [](UNsTweenAsyncActionLinearColor& Action, const FLinearColor& Value)
    {
        Action.ApplyEasing.Broadcast(Value);
    });
}

