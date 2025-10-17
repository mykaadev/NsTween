// Copyright (C) 2024 mykaa. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintAsyncActionBase.h"
#include "NsTween.h"
#include "NsTweenAsyncAction.generated.h"

class UCurveFloat;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FTweenAsyncEvent);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FTweenAsyncUpdateFloat, float, Value);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FTweenAsyncUpdateQuat, FQuat, Value);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FTweenAsyncUpdateRotator, FRotator, Value);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FTweenAsyncUpdateVector, FVector, Value);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FTweenAsyncUpdateVector2D, FVector2D, Value);

UCLASS(Abstract, BlueprintType)
class NSTWEEN_API UNsTweenAsyncAction : public UBlueprintAsyncActionBase
{
    GENERATED_BODY()

public:
    UPROPERTY(BlueprintAssignable, Category = "Tween|Async")
    FTweenAsyncEvent OnComplete;

    UPROPERTY(BlueprintAssignable, Category = "Tween|Async")
    FTweenAsyncEvent OnLoop;

    UPROPERTY(BlueprintAssignable, Category = "Tween|Async")
    FTweenAsyncEvent OnPingPong;

    UFUNCTION(BlueprintCallable, Category = "Tween")
    void Pause();

    UFUNCTION(BlueprintCallable, Category = "Tween")
    void Resume();

    UFUNCTION(BlueprintCallable, Category = "Tween")
    void Restart();

    UFUNCTION(BlueprintCallable, Category = "Tween")
    void Stop();

    UFUNCTION(BlueprintCallable, Category = "Tween")
    void SetTimeMultiplier(float Multiplier);

    virtual void Activate() override;
    virtual void BeginDestroy() override;

protected:
    void InitialiseCommon(UObject* WorldContextObject,
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
        bool bInUseCustomCurve);

    ENsTweenEase GetEffectiveEase() const;
    void ApplyBuilderOptions(FNsTweenBuilder& Builder);
    void HandleCompletedTween();

    virtual void LaunchTween() PURE_VIRTUAL(UNsTweenAsyncAction::LaunchTween, );

protected:
    UPROPERTY()
    TObjectPtr<UObject> WorldContext = nullptr;

    FNsTweenBuilder ActiveTween;
    float DurationSeconds = 0.f;
    float DelaySeconds = 0.f;
    int32 LoopCount = 0;
    float LoopDelaySeconds = 0.f;
    bool bPingPong = false;
    float PingPongDelaySeconds = 0.f;
    bool bCanTickDuringPause = false;
    bool bUseGlobalTimeDilation = true;
    float TimeMultiplier = 1.f;
    ENsTweenEase EaseType = ENsTweenEase::InOutSine;

    UPROPERTY()
    TObjectPtr<UCurveFloat> CurveOverride = nullptr;
    bool bUseCurveOverride = false;

    bool bHasActivated = false;
};

UCLASS(BlueprintType)
class NSTWEEN_API UNsTweenAsyncActionFloat : public UNsTweenAsyncAction
{
    GENERATED_BODY()

public:
    UPROPERTY(BlueprintAssignable, Category = "Tween|Async")
    FTweenAsyncUpdateFloat ApplyEasing;

    UFUNCTION(BlueprintCallable, meta = (BlueprintInternalUseOnly = "true", WorldContext = "WorldContextObject", DefaultToSelf = "WorldContextObject", AdvancedDisplay = "5"), Category = "Tween")
    static UNsTweenAsyncActionFloat* TweenFloat(UObject* WorldContextObject, float Start = 0.0f, float End = 1.0f, float DurationSecs = 1.0f, ENsTweenEase EaseType = ENsTweenEase::InOutQuad, float Delay = 0.f, int32 Loops = 0, float LoopDelay = 0.f, bool bPingPong = false, float PingPongDelay = 0.f, bool bCanTickDuringPause = false, bool bUseGlobalTimeDilation = true);

    UFUNCTION(BlueprintCallable, meta = (BlueprintInternalUseOnly = "true", WorldContext = "WorldContextObject", DefaultToSelf = "WorldContextObject", AdvancedDisplay = "5"), Category = "Tween|Custom Curve")
    static UNsTweenAsyncActionFloat* TweenFloatCustomCurve(UObject* WorldContextObject, float Start = 0.0f, float End = 1.0f, float DurationSecs = 1.0f, UCurveFloat* Curve = nullptr, float Delay = 0.f, int32 Loops = 0, float LoopDelay = 0.f, bool bPingPong = false, float PingPongDelay = 0.f, bool bCanTickDuringPause = false, bool bUseGlobalTimeDilation = true);

protected:
    virtual void LaunchTween() override;

private:
    float StartValue = 0.f;
    float EndValue = 1.f;
};

UCLASS(BlueprintType)
class NSTWEEN_API UNsTweenAsyncActionQuat : public UNsTweenAsyncAction
{
    GENERATED_BODY()

public:
    UPROPERTY(BlueprintAssignable, Category = "Tween|Async")
    FTweenAsyncUpdateQuat ApplyEasing;

    UFUNCTION(BlueprintCallable, meta = (BlueprintInternalUseOnly = "true", WorldContext = "WorldContextObject", DefaultToSelf = "WorldContextObject", AdvancedDisplay = "5"), Category = "Tween")
    static UNsTweenAsyncActionQuat* TweenQuat(UObject* WorldContextObject, FQuat Start, FQuat End, float DurationSecs = 1.0f, ENsTweenEase EaseType = ENsTweenEase::InOutQuad, float Delay = 0.f, int32 Loops = 0, float LoopDelay = 0.f, bool bPingPong = false, float PingPongDelay = 0.f, bool bCanTickDuringPause = false, bool bUseGlobalTimeDilation = true);

    UFUNCTION(BlueprintCallable, meta = (BlueprintInternalUseOnly = "true", WorldContext = "WorldContextObject", DefaultToSelf = "WorldContextObject", AdvancedDisplay = "5"), Category = "Tween")
    static UNsTweenAsyncActionQuat* TweenQuatFromRotator(UObject* WorldContextObject, FRotator Start = FRotator::ZeroRotator, FRotator End = FRotator::ZeroRotator, float DurationSecs = 1.0f, ENsTweenEase EaseType = ENsTweenEase::InOutQuad, float Delay = 0.f, int32 Loops = 0, float LoopDelay = 0.f, bool bPingPong = false, float PingPongDelay = 0.f, bool bCanTickDuringPause = false, bool bUseGlobalTimeDilation = true);

    UFUNCTION(BlueprintCallable, meta = (BlueprintInternalUseOnly = "true", WorldContext = "WorldContextObject", DefaultToSelf = "WorldContextObject", AdvancedDisplay = "5"), Category = "Tween|Custom Curve")
    static UNsTweenAsyncActionQuat* TweenQuatCustomCurve(UObject* WorldContextObject, FQuat Start, FQuat End, float DurationSecs = 1.0f, UCurveFloat* Curve = nullptr, float Delay = 0.f, int32 Loops = 0, float LoopDelay = 0.f, bool bPingPong = false, float PingPongDelay = 0.f, bool bCanTickDuringPause = false, bool bUseGlobalTimeDilation = true);

    UFUNCTION(BlueprintCallable, meta = (BlueprintInternalUseOnly = "true", WorldContext = "WorldContextObject", DefaultToSelf = "WorldContextObject", AdvancedDisplay = "5"), Category = "Tween|Custom Curve")
    static UNsTweenAsyncActionQuat* TweenQuatFromRotatorCustomCurve(UObject* WorldContextObject, FRotator Start = FRotator::ZeroRotator, FRotator End = FRotator::ZeroRotator, float DurationSecs = 1.0f, UCurveFloat* Curve = nullptr, float Delay = 0.f, int32 Loops = 0, float LoopDelay = 0.f, bool bPingPong = false, float PingPongDelay = 0.f, bool bCanTickDuringPause = false, bool bUseGlobalTimeDilation = true);

protected:
    virtual void LaunchTween() override;

private:
    FQuat StartValue = FQuat::Identity;
    FQuat EndValue = FQuat::Identity;
};

UCLASS(BlueprintType)
class NSTWEEN_API UNsTweenAsyncActionRotator : public UNsTweenAsyncAction
{
    GENERATED_BODY()

public:
    UPROPERTY(BlueprintAssignable, Category = "Tween|Async")
    FTweenAsyncUpdateRotator ApplyEasing;

    UFUNCTION(BlueprintCallable, meta = (BlueprintInternalUseOnly = "true", WorldContext = "WorldContextObject", DefaultToSelf = "WorldContextObject", AdvancedDisplay = "5"), Category = "Tween")
    static UNsTweenAsyncActionRotator* TweenRotator(UObject* WorldContextObject, FRotator Start = FRotator::ZeroRotator, FRotator End = FRotator::ZeroRotator, float DurationSecs = 1.0f, ENsTweenEase EaseType = ENsTweenEase::InOutQuad, float Delay = 0.f, int32 Loops = 0, float LoopDelay = 0.f, bool bPingPong = false, float PingPongDelay = 0.f, bool bCanTickDuringPause = false, bool bUseGlobalTimeDilation = true);

    UFUNCTION(BlueprintCallable, meta = (BlueprintInternalUseOnly = "true", WorldContext = "WorldContextObject", DefaultToSelf = "WorldContextObject", AdvancedDisplay = "5"), Category = "Tween|Custom Curve")
    static UNsTweenAsyncActionRotator* TweenRotatorCustomCurve(UObject* WorldContextObject, FRotator Start = FRotator::ZeroRotator, FRotator End = FRotator::ZeroRotator, float DurationSecs = 1.0f, UCurveFloat* Curve = nullptr, float Delay = 0.f, int32 Loops = 0, float LoopDelay = 0.f, bool bPingPong = false, float PingPongDelay = 0.f, bool bCanTickDuringPause = false, bool bUseGlobalTimeDilation = true);

protected:
    virtual void LaunchTween() override;

private:
    FQuat StartQuat = FQuat::Identity;
    FQuat EndQuat = FQuat::Identity;
};

UCLASS(BlueprintType)
class NSTWEEN_API UNsTweenAsyncActionVector : public UNsTweenAsyncAction
{
    GENERATED_BODY()

public:
    UPROPERTY(BlueprintAssignable, Category = "Tween|Async")
    FTweenAsyncUpdateVector ApplyEasing;

    UFUNCTION(BlueprintCallable, meta = (BlueprintInternalUseOnly = "true", WorldContext = "WorldContextObject", DefaultToSelf = "WorldContextObject", AdvancedDisplay = "5"), Category = "Tween")
    static UNsTweenAsyncActionVector* TweenVector(UObject* WorldContextObject, FVector Start = FVector::ZeroVector, FVector End = FVector::ZeroVector, float DurationSecs = 1.0f, ENsTweenEase EaseType = ENsTweenEase::InOutQuad, float Delay = 0.f, int32 Loops = 0, float LoopDelay = 0.f, bool bPingPong = false, float PingPongDelay = 0.f, bool bCanTickDuringPause = false, bool bUseGlobalTimeDilation = true);

    UFUNCTION(BlueprintCallable, meta = (BlueprintInternalUseOnly = "true", WorldContext = "WorldContextObject", DefaultToSelf = "WorldContextObject", AdvancedDisplay = "5"), Category = "Tween|Custom Curve")
    static UNsTweenAsyncActionVector* TweenVectorCustomCurve(UObject* WorldContextObject, FVector Start = FVector::ZeroVector, FVector End = FVector::ZeroVector, float DurationSecs = 1.0f, UCurveFloat* Curve = nullptr, float Delay = 0.f, int32 Loops = 0, float LoopDelay = 0.f, bool bPingPong = false, float PingPongDelay = 0.f, bool bCanTickDuringPause = false, bool bUseGlobalTimeDilation = true);

protected:
    virtual void LaunchTween() override;

private:
    FVector StartValue = FVector::ZeroVector;
    FVector EndValue = FVector::ZeroVector;
};

UCLASS(BlueprintType)
class NSTWEEN_API UNsTweenAsyncActionVector2D : public UNsTweenAsyncAction
{
    GENERATED_BODY()

public:
    UPROPERTY(BlueprintAssignable, Category = "Tween|Async")
    FTweenAsyncUpdateVector2D ApplyEasing;

    UFUNCTION(BlueprintCallable, meta = (BlueprintInternalUseOnly = "true", WorldContext = "WorldContextObject", DefaultToSelf = "WorldContextObject", AdvancedDisplay = "5"), Category = "Tween")
    static UNsTweenAsyncActionVector2D* TweenVector2D(UObject* WorldContextObject, FVector2D Start = FVector2D::ZeroVector, FVector2D End = FVector2D::ZeroVector, float DurationSecs = 1.0f, ENsTweenEase EaseType = ENsTweenEase::InOutQuad, float Delay = 0.f, int32 Loops = 0, float LoopDelay = 0.f, bool bPingPong = false, float PingPongDelay = 0.f, bool bCanTickDuringPause = false, bool bUseGlobalTimeDilation = true);

    UFUNCTION(BlueprintCallable, meta = (BlueprintInternalUseOnly = "true", WorldContext = "WorldContextObject", DefaultToSelf = "WorldContextObject", AdvancedDisplay = "5", DisplayName = "Tween Vector 2D Custom Curve"), Category = "Tween|Custom Curve")
    static UNsTweenAsyncActionVector2D* TweenVector2DCustomCurve(UObject* WorldContextObject, FVector2D Start = FVector2D::ZeroVector, FVector2D End = FVector2D::ZeroVector, float DurationSecs = 1.0f, UCurveFloat* Curve = nullptr, float Delay = 0.f, int32 Loops = 0, float LoopDelay = 0.f, bool bPingPong = false, float PingPongDelay = 0.f, bool bCanTickDuringPause = false, bool bUseGlobalTimeDilation = true);

protected:
    virtual void LaunchTween() override;

private:
    FVector2D StartValue = FVector2D::ZeroVector;
    FVector2D EndValue = FVector2D::ZeroVector;
};

