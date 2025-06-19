// Copyright (C) 2025 mykaadev. All rights reserved.

#include "Blueprints/NsTweenAsyncAction.h"
#include "Blueprints/NsTweenAsyncActionHelpers.h"
#include "Classes/NsTweenCore.h"

void UNsTweenAsyncAction::Activate()
{
    if (TweenInstance != nullptr)
    {
        // restart the tween
        TweenInstance->Destroy();
        TweenInstance = nullptr;
    }
    if (DurationSecs <= 0)
    {
        FFrame::KismetExecutionMessage(TEXT("Duration must be more than 0"), ELogVerbosity::Error);
        return;
    }
    if (bUseCustomCurve)
    {
        if (CustomCurve != nullptr)
        {
            EaseType = ENsTweenEase::Linear;
            TweenInstance = CreateTweenCustomCurve();
        }
        else
        {
            FFrame::KismetExecutionMessage(TEXT("No Custom Curve defined for custom curve task"), ELogVerbosity::Error);
            return;
        }
    }
    else
    {
        TweenInstance = CreateTween();
    }
    if (TweenInstance == nullptr)
    {
        FFrame::KismetExecutionMessage(TEXT("Tween Instance was not created in child class"), ELogVerbosity::Error);
        return;
    }
    TweenInstance->DelaySecs = Delay;
    TweenInstance->NumLoops = Loops;
    TweenInstance->LoopDelaySecs = LoopDelay;
    TweenInstance->bShouldPingPong = bPingPong;
    TweenInstance->PingPongDelaySecs = PingPongDelay;
    TweenInstance->bCanTickDuringPause = bCanTickDuringPause;
    TweenInstance->bUseGlobalTimeDilation = bUseGlobalTimeDilation;
    // we will tell it when to be destroyed on complete, so that we control when
    // the tween goes invalid and it can't get recycled by doing something unexpected in BPs
    TweenInstance->bShouldAutoDestroy = false;
    TweenInstance->EaseParam1 = EaseParam1;
    TweenInstance->EaseParam2 = EaseParam2;

    if (OnLoop.IsBound())
    {
        TweenInstance->OnLoop([&]() { OnLoop.Broadcast(); });
    }
    if (OnPingPong.IsBound())
    {
        TweenInstance->OnPingPong([&]() { OnPingPong.Broadcast(); });
    }
    if (OnComplete.IsBound())
    {
        TweenInstance->OnComplete([&]()
        {
            OnComplete.Broadcast();
            Stop();
        });
    }
}

FNsTweenInstance* UNsTweenAsyncAction::CreateTween()
{
    // override in specific data type tasks
    return nullptr;
}

FNsTweenInstance* UNsTweenAsyncAction::CreateTweenCustomCurve()
{
    return nullptr;
}

void UNsTweenAsyncAction::SetSharedTweenProperties(float InDurationSecs, float InDelay, int InLoops, float InLoopDelay, bool InbPingPong, float InPingPongDelay, bool bInCanTickDuringPause, bool bInUseGlobalTimeDilation)
{
    TweenInstance = nullptr;
    bUseCustomCurve = false;
    CustomCurve = nullptr;
    DurationSecs = InDurationSecs;
    Delay = InDelay;
    Loops = InLoops;
    LoopDelay = InLoopDelay;
    bPingPong = InbPingPong;
    PingPongDelay = InPingPongDelay;
    bCanTickDuringPause = bInCanTickDuringPause;
    bUseGlobalTimeDilation = bInUseGlobalTimeDilation;
}

void UNsTweenAsyncAction::BeginDestroy()
{
    Super::BeginDestroy();
    if (TweenInstance != nullptr)
    {
        TweenInstance->Destroy();
        TweenInstance = nullptr;
    }
}

void UNsTweenAsyncAction::Pause()
{
    if (TweenInstance)
    {
        TweenInstance->Pause();
    }
}

void UNsTweenAsyncAction::Unpause()
{
    if (TweenInstance)
    {
        TweenInstance->Unpause();
    }
}

void UNsTweenAsyncAction::Restart()
{
    if (TweenInstance)
    {
        TweenInstance->Restart();
    }
}

void UNsTweenAsyncAction::Stop()
{
    if (TweenInstance)
    {
        TweenInstance->Destroy();
        TweenInstance = nullptr;
        SetReadyToDestroy();
#if ENGINE_MAJOR_VERSION < 5
        MarkPendingKill();
#else
        MarkAsGarbage();
#endif
    }
}

void UNsTweenAsyncAction::SetTimeMultiplier(float Multiplier)
{
    if (TweenInstance)
    {
        TweenInstance->TimeMultiplier = FMath::Abs(Multiplier);
    }
}

UNsTweenAsyncActionFloat* UNsTweenAsyncActionFloat::TweenFloat(float Start, float End, float DurationSecs, ENsTweenEase EaseType, float EaseParam1, float EaseParam2, float Delay, int Loops, float LoopDelay, bool bPingPong, float PingPongDelay, bool bCanTickDuringPause, bool bUseGlobalTimeDilation)
{
    UNsTweenAsyncActionFloat* BlueprintNode = NewObject<UNsTweenAsyncActionFloat>();
    BlueprintNode->SetSharedTweenProperties( DurationSecs, Delay, Loops, LoopDelay, bPingPong, PingPongDelay, bCanTickDuringPause, bUseGlobalTimeDilation);
    BlueprintNode->EaseType = EaseType;
    BlueprintNode->Start = Start;
    BlueprintNode->End = End;
    BlueprintNode->EaseParam1 = EaseParam1;
    BlueprintNode->EaseParam2 = EaseParam2;
    return BlueprintNode;
}

UNsTweenAsyncActionFloat* UNsTweenAsyncActionFloat::TweenFloatCustomCurve(float Start, float End, float DurationSecs, UCurveFloat* Curve, float Delay, int Loops, float LoopDelay, bool bPingPong, float PingPongDelay, bool bCanTickDuringPause, bool bUseGlobalTimeDilation)
{
    UNsTweenAsyncActionFloat* BlueprintNode = NewObject<UNsTweenAsyncActionFloat>();
    BlueprintNode->SetSharedTweenProperties( DurationSecs, Delay, Loops, LoopDelay, bPingPong, PingPongDelay, bCanTickDuringPause, bUseGlobalTimeDilation);
    BlueprintNode->CustomCurve = Curve;
    BlueprintNode->bUseCustomCurve = true;
    BlueprintNode->Start = Start;
    BlueprintNode->End = End;
    BlueprintNode->EaseParam1 = 0;
    BlueprintNode->EaseParam2 = 0;
    return BlueprintNode;
}

FNsTweenInstance* UNsTweenAsyncActionFloat::CreateTween()
{
    return NsTweenAsyncActionHelpers::CreateTween(Start, End, DurationSecs, EaseType,
        [&](float T) { ApplyEasing.Broadcast(T); });
}

FNsTweenInstance* UNsTweenAsyncActionFloat::CreateTweenCustomCurve()
{
    return NsTweenAsyncActionHelpers::CreateTweenCustomCurve(Start, End, DurationSecs, CustomCurve, EaseType,
        [&](float V) { ApplyEasing.Broadcast(V); });
}

UNsTweenAsyncActionQuat* UNsTweenAsyncActionQuat::TweenQuat(FQuat Start, FQuat End, float DurationSecs, ENsTweenEase EaseType, float EaseParam1, float EaseParam2, float Delay, int Loops, float LoopDelay, bool bPingPong, float PingPongDelay, bool bCanTickDuringPause, bool bUseGlobalTimeDilation)
{
    UNsTweenAsyncActionQuat* BlueprintNode = NewObject<UNsTweenAsyncActionQuat>();
    BlueprintNode->SetSharedTweenProperties( DurationSecs, Delay, Loops, LoopDelay, bPingPong, PingPongDelay, bCanTickDuringPause, bUseGlobalTimeDilation);
    BlueprintNode->EaseType = EaseType;
    BlueprintNode->Start = Start;
    BlueprintNode->End = End;
    BlueprintNode->EaseParam1 = EaseParam1;
    BlueprintNode->EaseParam2 = EaseParam2;
    return BlueprintNode;
}

UNsTweenAsyncActionQuat* UNsTweenAsyncActionQuat::TweenQuatFromRotator(FRotator Start, FRotator End, float DurationSecs, ENsTweenEase EaseType, float EaseParam1, float EaseParam2, float Delay, int Loops, float LoopDelay, bool bPingPong, float PingPongDelay, bool bCanTickDuringPause, bool bUseGlobalTimeDilation)
{
    UNsTweenAsyncActionQuat* BlueprintNode = NewObject<UNsTweenAsyncActionQuat>();
    BlueprintNode->SetSharedTweenProperties
            (
             DurationSecs,
             Delay,
             Loops,
             LoopDelay,
             bPingPong,
             PingPongDelay,
             bCanTickDuringPause,
             bUseGlobalTimeDilation
            );
    BlueprintNode->EaseType = EaseType;
    BlueprintNode->Start = Start.Quaternion();
    BlueprintNode->End = End.Quaternion();
    BlueprintNode->EaseParam1 = EaseParam1;
    BlueprintNode->EaseParam2 = EaseParam2;
    return BlueprintNode;
}

UNsTweenAsyncActionQuat* UNsTweenAsyncActionQuat::TweenQuatCustomCurve(FQuat Start, FQuat End, float DurationSecs, UCurveFloat* Curve, float Delay, int Loops, float LoopDelay, bool bPingPong, float PingPongDelay, bool bCanTickDuringPause, bool bUseGlobalTimeDilation)
{
    UNsTweenAsyncActionQuat* BlueprintNode = NewObject<UNsTweenAsyncActionQuat>();
    BlueprintNode->SetSharedTweenProperties( DurationSecs, Delay, Loops, LoopDelay, bPingPong, PingPongDelay, bCanTickDuringPause, bUseGlobalTimeDilation);
    BlueprintNode->CustomCurve = Curve;
    BlueprintNode->bUseCustomCurve = true;
    BlueprintNode->Start = Start;
    BlueprintNode->End = End;
    BlueprintNode->EaseParam1 = 0;
    BlueprintNode->EaseParam2 = 0;
    return BlueprintNode;
}

UNsTweenAsyncActionQuat* UNsTweenAsyncActionQuat::TweenQuatFromRotatorCustomCurve(FRotator Start, FRotator End, float DurationSecs, UCurveFloat* Curve, float Delay, int Loops, float LoopDelay, bool bPingPong, float PingPongDelay, bool bCanTickDuringPause, bool bUseGlobalTimeDilation)
{
    UNsTweenAsyncActionQuat* BlueprintNode = NewObject<UNsTweenAsyncActionQuat>();
    BlueprintNode->SetSharedTweenProperties( DurationSecs, Delay, Loops, LoopDelay, bPingPong, PingPongDelay, bCanTickDuringPause, bUseGlobalTimeDilation);
    BlueprintNode->CustomCurve = Curve;
    BlueprintNode->bUseCustomCurve = true;
    BlueprintNode->Start = Start.Quaternion();
    BlueprintNode->End = End.Quaternion();
    BlueprintNode->EaseParam1 = 0;
    BlueprintNode->EaseParam2 = 0;
    return BlueprintNode;
}

FNsTweenInstance* UNsTweenAsyncActionQuat::CreateTween()
{
    return NsTweenAsyncActionHelpers::CreateTween(Start, End, DurationSecs, EaseType,
        [&](const FQuat& Q) { ApplyEasing.Broadcast(Q); });
}

FNsTweenInstance* UNsTweenAsyncActionQuat::CreateTweenCustomCurve()
{
    return NsTweenAsyncActionHelpers::CreateTweenCustomCurve(Start, End, DurationSecs, CustomCurve, EaseType,
        [&](const FQuat& Q) { ApplyEasing.Broadcast(Q); });
}

UNsTweenAsyncActionRotator* UNsTweenAsyncActionRotator::TweenRotator(FRotator Start, FRotator End, float DurationSecs, ENsTweenEase EaseType, float EaseParam1, float EaseParam2, float Delay, int Loops, float LoopDelay, bool bPingPong, float PingPongDelay, bool bCanTickDuringPause, bool bUseGlobalTimeDilation)
{
    UNsTweenAsyncActionRotator* BlueprintNode = NewObject<UNsTweenAsyncActionRotator>();
    BlueprintNode->SetSharedTweenProperties (  DurationSecs,  Delay,  Loops,  LoopDelay,  bPingPong,  PingPongDelay,  bCanTickDuringPause,  bUseGlobalTimeDilation );
    BlueprintNode->EaseType = EaseType;
    BlueprintNode->Start = Start.Quaternion();
    BlueprintNode->End = End.Quaternion();
    BlueprintNode->EaseParam1 = EaseParam1;
    BlueprintNode->EaseParam2 = EaseParam2;
    return BlueprintNode;
}

UNsTweenAsyncActionRotator* UNsTweenAsyncActionRotator::TweenRotatorCustomCurve(FRotator Start, FRotator End, float DurationSecs, UCurveFloat* Curve, float Delay, int Loops, float LoopDelay, bool bPingPong, float PingPongDelay, bool bCanTickDuringPause, bool bUseGlobalTimeDilation)
{
    UNsTweenAsyncActionRotator* BlueprintNode = NewObject<UNsTweenAsyncActionRotator>();
    BlueprintNode->SetSharedTweenProperties( DurationSecs, Delay, Loops, LoopDelay, bPingPong, PingPongDelay, bCanTickDuringPause, bUseGlobalTimeDilation);
    BlueprintNode->CustomCurve = Curve;
    BlueprintNode->bUseCustomCurve = true;
    BlueprintNode->Start = Start.Quaternion();
    BlueprintNode->End = End.Quaternion();
    BlueprintNode->EaseParam1 = 0;
    BlueprintNode->EaseParam2 = 0;
    return BlueprintNode;
}

FNsTweenInstance* UNsTweenAsyncActionRotator::CreateTween()
{
    return NsTweenAsyncActionHelpers::CreateTween(Start, End, DurationSecs, EaseType,
        [&](const FQuat& Q)
        {
            ApplyEasing.Broadcast(Q.Rotator());
        });
}

FNsTweenInstance* UNsTweenAsyncActionRotator::CreateTweenCustomCurve()
{
    return NsTweenAsyncActionHelpers::CreateTweenCustomCurve(Start, End, DurationSecs, CustomCurve, EaseType,
        [&](const FQuat& Q)
        {
            ApplyEasing.Broadcast(Q.Rotator());
        });
}

UNsTweenAsyncActionVector* UNsTweenAsyncActionVector::TweenVector(FVector Start, FVector End, float DurationSecs, ENsTweenEase EaseType, float EaseParam1, float EaseParam2, float Delay, int Loops, float LoopDelay, bool bPingPong, float PingPongDelay, bool bCanTickDuringPause, bool bUseGlobalTimeDilation)
{
    UNsTweenAsyncActionVector* BlueprintNode = NewObject<UNsTweenAsyncActionVector>();
    BlueprintNode->SetSharedTweenProperties( DurationSecs, Delay, Loops, LoopDelay, bPingPong, PingPongDelay, bCanTickDuringPause, bUseGlobalTimeDilation);
    BlueprintNode->EaseType = EaseType;
    BlueprintNode->Start = Start;
    BlueprintNode->End = End;
    BlueprintNode->EaseParam1 = EaseParam1;
    BlueprintNode->EaseParam2 = EaseParam2;
    return BlueprintNode;
}

UNsTweenAsyncActionVector* UNsTweenAsyncActionVector::TweenVectorCustomCurve(FVector Start, FVector End, float DurationSecs, UCurveFloat* Curve, float Delay, int Loops, float LoopDelay, bool bPingPong, float PingPongDelay, bool bCanTickDuringPause, bool bUseGlobalTimeDilation)
{
    UNsTweenAsyncActionVector* BlueprintNode = NewObject<UNsTweenAsyncActionVector>();
    BlueprintNode->SetSharedTweenProperties(  DurationSecs,  Delay,  Loops,  LoopDelay,  bPingPong,  PingPongDelay,  bCanTickDuringPause,  bUseGlobalTimeDilation );
    BlueprintNode->CustomCurve = Curve;
    BlueprintNode->bUseCustomCurve = true;
    BlueprintNode->Start = Start;
    BlueprintNode->End = End;
    BlueprintNode->EaseParam1 = 0;
    BlueprintNode->EaseParam2 = 0;
    return BlueprintNode;
}

FNsTweenInstance* UNsTweenAsyncActionVector::CreateTween()
{
    return NsTweenAsyncActionHelpers::CreateTween(Start, End, DurationSecs, EaseType,
        [&](const FVector& V) { ApplyEasing.Broadcast(V); });
}

FNsTweenInstance* UNsTweenAsyncActionVector::CreateTweenCustomCurve()
{
    return NsTweenAsyncActionHelpers::CreateTweenCustomCurve(Start, End, DurationSecs, CustomCurve, EaseType,
        [&](const FVector& V) { ApplyEasing.Broadcast(V); });
}

UNsTweenAsyncActionVector2D* UNsTweenAsyncActionVector2D::TweenVector2D(FVector2D Start, FVector2D End, float DurationSecs, ENsTweenEase EaseType, float EaseParam1, float EaseParam2, float Delay, int Loops, float LoopDelay, bool bPingPong, float PingPongDelay, bool bCanTickDuringPause, bool bUseGlobalTimeDilation)
{
    UNsTweenAsyncActionVector2D* BlueprintNode = NewObject<UNsTweenAsyncActionVector2D>();
    BlueprintNode->SetSharedTweenProperties( DurationSecs, Delay, Loops, LoopDelay, bPingPong, PingPongDelay, bCanTickDuringPause, bUseGlobalTimeDilation);
    BlueprintNode->EaseType = EaseType;
    BlueprintNode->Start = Start;
    BlueprintNode->End = End;
    BlueprintNode->EaseParam1 = EaseParam1;
    BlueprintNode->EaseParam2 = EaseParam2;
    return BlueprintNode;
}

UNsTweenAsyncActionVector2D* UNsTweenAsyncActionVector2D::TweenVector2DCustomCurve(FVector2D Start, FVector2D End, float DurationSecs, UCurveFloat* Curve, float Delay, int Loops, float LoopDelay, bool bPingPong, float PingPongDelay, bool bCanTickDuringPause, bool bUseGlobalTimeDilation)
{
    UNsTweenAsyncActionVector2D* BlueprintNode = NewObject<UNsTweenAsyncActionVector2D>();
    BlueprintNode->SetSharedTweenProperties( DurationSecs, Delay, Loops, LoopDelay, bPingPong, PingPongDelay, bCanTickDuringPause, bUseGlobalTimeDilation);
    BlueprintNode->CustomCurve = Curve;
    BlueprintNode->bUseCustomCurve = true;
    BlueprintNode->Start = Start;
    BlueprintNode->End = End;
    BlueprintNode->EaseParam1 = 0;
    BlueprintNode->EaseParam2 = 0;
    return BlueprintNode;
}

FNsTweenInstance* UNsTweenAsyncActionVector2D::CreateTween()
{
    return NsTweenAsyncActionHelpers::CreateTween(Start, End, DurationSecs, EaseType,
        [&](const FVector2D V) { ApplyEasing.Broadcast(V); });
}

FNsTweenInstance* UNsTweenAsyncActionVector2D::CreateTweenCustomCurve()
{
    return NsTweenAsyncActionHelpers::CreateTweenCustomCurve(Start, End, DurationSecs, CustomCurve, EaseType,
        [&](const FVector2D V)
        {
            ApplyEasing.Broadcast(V);
        });
}
