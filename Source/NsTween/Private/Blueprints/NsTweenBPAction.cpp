#include "Blueprints/NsTweenBPAction.h"
#include "Classes/NsTweenCore.h"

void UNsTweenBPAction::Activate()
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
    TweenInstance->SetDelay(Delay)
                 ->SetLoops(Loops)
                 ->SetLoopDelay(LoopDelay)
                 ->SetYoyo(bYoyo)
                 ->SetYoyoDelay(YoyoDelay)
                 ->SetCanTickDuringPause(bCanTickDuringPause)
                 ->SetUseGlobalTimeDilation(bUseGlobalTimeDilation)
                 // we will tell it when to be destroyed on complete, so that we control when
                 // the tween goes invalid and it can't get recycled by doing something unexpected in BPs
                 ->SetAutoDestroy(false)
                 ->SetEaseParam1(EaseParam1)
                 ->SetEaseParam2(EaseParam2);

    if (OnLoop.IsBound())
    {
        TweenInstance->SetOnLoop([&]() { OnLoop.Broadcast(); });
    }
    if (OnYoyo.IsBound())
    {
        TweenInstance->SetOnYoyo([&]() { OnYoyo.Broadcast(); });
    }
    if (OnComplete.IsBound())
    {
        TweenInstance->SetOnComplete([&]()
        {
            OnComplete.Broadcast();
            Stop();
        });
    }
}

NsTweenInstance* UNsTweenBPAction::CreateTween()
{
    // override in specific data type tasks
    return nullptr;
}

NsTweenInstance* UNsTweenBPAction::CreateTweenCustomCurve()
{
    return nullptr;
}

void UNsTweenBPAction::SetSharedTweenProperties(float InDurationSecs, float InDelay, int InLoops, float InLoopDelay, bool InbYoyo, float InYoyoDelay, bool bInCanTickDuringPause, bool bInUseGlobalTimeDilation)
{
    TweenInstance = nullptr;
    bUseCustomCurve = false;
    CustomCurve = nullptr;
    DurationSecs = InDurationSecs;
    Delay = InDelay;
    Loops = InLoops;
    LoopDelay = InLoopDelay;
    bYoyo = InbYoyo;
    YoyoDelay = InYoyoDelay;
    bCanTickDuringPause = bInCanTickDuringPause;
    bUseGlobalTimeDilation = bInUseGlobalTimeDilation;
}

void UNsTweenBPAction::BeginDestroy()
{
    Super::BeginDestroy();
    if (TweenInstance != nullptr)
    {
        TweenInstance->Destroy();
        TweenInstance = nullptr;
    }
}

void UNsTweenBPAction::Pause()
{
    if (TweenInstance)
    {
        TweenInstance->Pause();
    }
}

void UNsTweenBPAction::Unpause()
{
    if (TweenInstance)
    {
        TweenInstance->Unpause();
    }
}

void UNsTweenBPAction::Restart()
{
    if (TweenInstance)
    {
        TweenInstance->Restart();
    }
}

void UNsTweenBPAction::Stop()
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

void UNsTweenBPAction::SetTimeMultiplier(float Multiplier)
{
    if (TweenInstance)
    {
        TweenInstance->SetTimeMultiplier(Multiplier);
    }
}

UNsTweenBPActionFloat* UNsTweenBPActionFloat::TweenFloat(float Start, float End, float DurationSecs, ENsTweenEase EaseType, float EaseParam1, float EaseParam2, float Delay, int Loops, float LoopDelay, bool bYoyo, float YoyoDelay, bool bCanTickDuringPause, bool bUseGlobalTimeDilation)
{
    UNsTweenBPActionFloat* BlueprintNode = NewObject<UNsTweenBPActionFloat>();
    BlueprintNode->SetSharedTweenProperties( DurationSecs, Delay, Loops, LoopDelay, bYoyo, YoyoDelay, bCanTickDuringPause, bUseGlobalTimeDilation);
    BlueprintNode->EaseType = EaseType;
    BlueprintNode->Start = Start;
    BlueprintNode->End = End;
    BlueprintNode->EaseParam1 = EaseParam1;
    BlueprintNode->EaseParam2 = EaseParam2;
    return BlueprintNode;
}

UNsTweenBPActionFloat* UNsTweenBPActionFloat::TweenFloatCustomCurve(float Start, float End, float DurationSecs, UCurveFloat* Curve, float Delay, int Loops, float LoopDelay, bool bYoyo, float YoyoDelay, bool bCanTickDuringPause, bool bUseGlobalTimeDilation)
{
    UNsTweenBPActionFloat* BlueprintNode = NewObject<UNsTweenBPActionFloat>();
    BlueprintNode->SetSharedTweenProperties( DurationSecs, Delay, Loops, LoopDelay, bYoyo, YoyoDelay, bCanTickDuringPause, bUseGlobalTimeDilation);
    BlueprintNode->CustomCurve = Curve;
    BlueprintNode->bUseCustomCurve = true;
    BlueprintNode->Start = Start;
    BlueprintNode->End = End;
    BlueprintNode->EaseParam1 = 0;
    BlueprintNode->EaseParam2 = 0;
    return BlueprintNode;
}

NsTweenInstance* UNsTweenBPActionFloat::CreateTween()
{
    return NsTweenCore::Play(Start, End, DurationSecs, EaseType, [&](const float T) { ApplyEasing.Broadcast(T); });
}

NsTweenInstance* UNsTweenBPActionFloat::CreateTweenCustomCurve()
{
    return NsTweenCore::Play (0,  1,  DurationSecs,  EaseType,  [&](const float T)
    {
        const float EasedTime = CustomCurve->GetFloatValue(T);
        const float EasedValue = FMath::Lerp(Start, End, EasedTime);
        ApplyEasing.Broadcast(EasedValue);
    });
}

UNsTweenBPActionQuat* UNsTweenBPActionQuat::TweenQuat(FQuat Start, FQuat End, float DurationSecs, ENsTweenEase EaseType, float EaseParam1, float EaseParam2, float Delay, int Loops, float LoopDelay, bool bYoyo, float YoyoDelay, bool bCanTickDuringPause, bool bUseGlobalTimeDilation)
{
    UNsTweenBPActionQuat* BlueprintNode = NewObject<UNsTweenBPActionQuat>();
    BlueprintNode->SetSharedTweenProperties( DurationSecs, Delay, Loops, LoopDelay, bYoyo, YoyoDelay, bCanTickDuringPause, bUseGlobalTimeDilation);
    BlueprintNode->EaseType = EaseType;
    BlueprintNode->Start = Start;
    BlueprintNode->End = End;
    BlueprintNode->EaseParam1 = EaseParam1;
    BlueprintNode->EaseParam2 = EaseParam2;
    return BlueprintNode;
}

UNsTweenBPActionQuat* UNsTweenBPActionQuat::TweenQuatFromRotator(FRotator Start, FRotator End, float DurationSecs, ENsTweenEase EaseType, float EaseParam1, float EaseParam2, float Delay, int Loops, float LoopDelay, bool bYoyo, float YoyoDelay, bool bCanTickDuringPause, bool bUseGlobalTimeDilation)
{
    UNsTweenBPActionQuat* BlueprintNode = NewObject<UNsTweenBPActionQuat>();
    BlueprintNode->SetSharedTweenProperties
            (
             DurationSecs,
             Delay,
             Loops,
             LoopDelay,
             bYoyo,
             YoyoDelay,
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

UNsTweenBPActionQuat* UNsTweenBPActionQuat::TweenQuatCustomCurve(FQuat Start, FQuat End, float DurationSecs, UCurveFloat* Curve, float Delay, int Loops, float LoopDelay, bool bYoyo, float YoyoDelay, bool bCanTickDuringPause, bool bUseGlobalTimeDilation)
{
    UNsTweenBPActionQuat* BlueprintNode = NewObject<UNsTweenBPActionQuat>();
    BlueprintNode->SetSharedTweenProperties( DurationSecs, Delay, Loops, LoopDelay, bYoyo, YoyoDelay, bCanTickDuringPause, bUseGlobalTimeDilation);
    BlueprintNode->CustomCurve = Curve;
    BlueprintNode->bUseCustomCurve = true;
    BlueprintNode->Start = Start;
    BlueprintNode->End = End;
    BlueprintNode->EaseParam1 = 0;
    BlueprintNode->EaseParam2 = 0;
    return BlueprintNode;
}

UNsTweenBPActionQuat* UNsTweenBPActionQuat::TweenQuatFromRotatorCustomCurve(FRotator Start, FRotator End, float DurationSecs, UCurveFloat* Curve, float Delay, int Loops, float LoopDelay, bool bYoyo, float YoyoDelay, bool bCanTickDuringPause, bool bUseGlobalTimeDilation)
{
    UNsTweenBPActionQuat* BlueprintNode = NewObject<UNsTweenBPActionQuat>();
    BlueprintNode->SetSharedTweenProperties( DurationSecs, Delay, Loops, LoopDelay, bYoyo, YoyoDelay, bCanTickDuringPause, bUseGlobalTimeDilation);
    BlueprintNode->CustomCurve = Curve;
    BlueprintNode->bUseCustomCurve = true;
    BlueprintNode->Start = Start.Quaternion();
    BlueprintNode->End = End.Quaternion();
    BlueprintNode->EaseParam1 = 0;
    BlueprintNode->EaseParam2 = 0;
    return BlueprintNode;
}

NsTweenInstance* UNsTweenBPActionQuat::CreateTween()
{
    return NsTweenCore::Play(Start, End, DurationSecs, EaseType, [&](const FQuat& T) { ApplyEasing.Broadcast(T); });
}

NsTweenInstance* UNsTweenBPActionQuat::CreateTweenCustomCurve()
{
    return NsTweenCore::Play (0,  1,  DurationSecs,  EaseType,  [&](const float T)
    {
     const float EasedTime = CustomCurve->GetFloatValue(T);
     const FQuat EasedValue = FMath::Lerp(Start, End, EasedTime);
     ApplyEasing.Broadcast(EasedValue);
    });
}

UNsTweenBPActionRotator* UNsTweenBPActionRotator::TweenRotator(FRotator Start, FRotator End, float DurationSecs, ENsTweenEase EaseType, float EaseParam1, float EaseParam2, float Delay, int Loops, float LoopDelay, bool bYoyo, float YoyoDelay, bool bCanTickDuringPause, bool bUseGlobalTimeDilation)
{
    UNsTweenBPActionRotator* BlueprintNode = NewObject<UNsTweenBPActionRotator>();
    BlueprintNode->SetSharedTweenProperties (  DurationSecs,  Delay,  Loops,  LoopDelay,  bYoyo,  YoyoDelay,  bCanTickDuringPause,  bUseGlobalTimeDilation );
    BlueprintNode->EaseType = EaseType;
    BlueprintNode->Start = Start.Quaternion();
    BlueprintNode->End = End.Quaternion();
    BlueprintNode->EaseParam1 = EaseParam1;
    BlueprintNode->EaseParam2 = EaseParam2;
    return BlueprintNode;
}

UNsTweenBPActionRotator* UNsTweenBPActionRotator::TweenRotatorCustomCurve(FRotator Start, FRotator End, float DurationSecs, UCurveFloat* Curve, float Delay, int Loops, float LoopDelay, bool bYoyo, float YoyoDelay, bool bCanTickDuringPause, bool bUseGlobalTimeDilation)
{
    UNsTweenBPActionRotator* BlueprintNode = NewObject<UNsTweenBPActionRotator>();
    BlueprintNode->SetSharedTweenProperties( DurationSecs, Delay, Loops, LoopDelay, bYoyo, YoyoDelay, bCanTickDuringPause, bUseGlobalTimeDilation);
    BlueprintNode->CustomCurve = Curve;
    BlueprintNode->bUseCustomCurve = true;
    BlueprintNode->Start = Start.Quaternion();
    BlueprintNode->End = End.Quaternion();
    BlueprintNode->EaseParam1 = 0;
    BlueprintNode->EaseParam2 = 0;
    return BlueprintNode;
}

NsTweenInstance* UNsTweenBPActionRotator::CreateTween()
{
    return NsTweenCore::Play( Start, End, DurationSecs, EaseType, [&](const FQuat& T)
    {
        ApplyEasing.Broadcast(T.Rotator());
    });
}

NsTweenInstance* UNsTweenBPActionRotator::CreateTweenCustomCurve()
{
    return NsTweenCore::Play(0, 1, DurationSecs, EaseType, [&](float t)
    {
        const float EasedTime = CustomCurve->GetFloatValue(t);
        const FQuat EasedValue = FMath::Lerp(Start, End, EasedTime);
        ApplyEasing.Broadcast(EasedValue.Rotator());
    });
}

UNsTweenBPActionVector* UNsTweenBPActionVector::TweenVector(FVector Start, FVector End, float DurationSecs, ENsTweenEase EaseType, float EaseParam1, float EaseParam2, float Delay, int Loops, float LoopDelay, bool bYoyo, float YoyoDelay, bool bCanTickDuringPause, bool bUseGlobalTimeDilation)
{
    UNsTweenBPActionVector* BlueprintNode = NewObject<UNsTweenBPActionVector>();
    BlueprintNode->SetSharedTweenProperties( DurationSecs, Delay, Loops, LoopDelay, bYoyo, YoyoDelay, bCanTickDuringPause, bUseGlobalTimeDilation);
    BlueprintNode->EaseType = EaseType;
    BlueprintNode->Start = Start;
    BlueprintNode->End = End;
    BlueprintNode->EaseParam1 = EaseParam1;
    BlueprintNode->EaseParam2 = EaseParam2;
    return BlueprintNode;
}

UNsTweenBPActionVector* UNsTweenBPActionVector::TweenVectorCustomCurve(FVector Start, FVector End, float DurationSecs, UCurveFloat* Curve, float Delay, int Loops, float LoopDelay, bool bYoyo, float YoyoDelay, bool bCanTickDuringPause, bool bUseGlobalTimeDilation)
{
    UNsTweenBPActionVector* BlueprintNode = NewObject<UNsTweenBPActionVector>();
    BlueprintNode->SetSharedTweenProperties(  DurationSecs,  Delay,  Loops,  LoopDelay,  bYoyo,  YoyoDelay,  bCanTickDuringPause,  bUseGlobalTimeDilation );
    BlueprintNode->CustomCurve = Curve;
    BlueprintNode->bUseCustomCurve = true;
    BlueprintNode->Start = Start;
    BlueprintNode->End = End;
    BlueprintNode->EaseParam1 = 0;
    BlueprintNode->EaseParam2 = 0;
    return BlueprintNode;
}

NsTweenInstance* UNsTweenBPActionVector::CreateTween()
{
    return NsTweenCore::Play(Start, End, DurationSecs, EaseType, [&](const FVector& T) { ApplyEasing.Broadcast(T); });
}

NsTweenInstance* UNsTweenBPActionVector::CreateTweenCustomCurve()
{
    return NsTweenCore::Play(0, 1, DurationSecs, EaseType, [&](const float T)
    {
        const float EasedTime = CustomCurve->GetFloatValue(T);
        const FVector EasedValue = FMath::Lerp(Start, End, EasedTime);
        ApplyEasing.Broadcast(EasedValue);
    });
}

UNsTweenBPActionVector2D* UNsTweenBPActionVector2D::TweenVector2D(FVector2D Start, FVector2D End, float DurationSecs, ENsTweenEase EaseType, float EaseParam1, float EaseParam2, float Delay, int Loops, float LoopDelay, bool bYoyo, float YoyoDelay, bool bCanTickDuringPause, bool bUseGlobalTimeDilation)
{
    UNsTweenBPActionVector2D* BlueprintNode = NewObject<UNsTweenBPActionVector2D>();
    BlueprintNode->SetSharedTweenProperties( DurationSecs, Delay, Loops, LoopDelay, bYoyo, YoyoDelay, bCanTickDuringPause, bUseGlobalTimeDilation);
    BlueprintNode->EaseType = EaseType;
    BlueprintNode->Start = Start;
    BlueprintNode->End = End;
    BlueprintNode->EaseParam1 = EaseParam1;
    BlueprintNode->EaseParam2 = EaseParam2;
    return BlueprintNode;
}

UNsTweenBPActionVector2D* UNsTweenBPActionVector2D::TweenVector2DCustomCurve(FVector2D Start, FVector2D End, float DurationSecs, UCurveFloat* Curve, float Delay, int Loops, float LoopDelay, bool bYoyo, float YoyoDelay, bool bCanTickDuringPause, bool bUseGlobalTimeDilation)
{
    UNsTweenBPActionVector2D* BlueprintNode = NewObject<UNsTweenBPActionVector2D>();
    BlueprintNode->SetSharedTweenProperties( DurationSecs, Delay, Loops, LoopDelay, bYoyo, YoyoDelay, bCanTickDuringPause, bUseGlobalTimeDilation);
    BlueprintNode->CustomCurve = Curve;
    BlueprintNode->bUseCustomCurve = true;
    BlueprintNode->Start = Start;
    BlueprintNode->End = End;
    BlueprintNode->EaseParam1 = 0;
    BlueprintNode->EaseParam2 = 0;
    return BlueprintNode;
}

NsTweenInstance* UNsTweenBPActionVector2D::CreateTween()
{
    return NsTweenCore::Play(Start, End, DurationSecs, EaseType, [&](const FVector2D T) { ApplyEasing.Broadcast(T); });
}

NsTweenInstance* UNsTweenBPActionVector2D::CreateTweenCustomCurve()
{
    return NsTweenCore::Play(0, 1, DurationSecs, EaseType,[&](const float T)
    {
         const float EasedTime = CustomCurve->GetFloatValue(T);
         const FVector2D EasedValue = FMath::Lerp(Start, End, EasedTime);
         ApplyEasing.Broadcast(EasedValue);
    });
}
