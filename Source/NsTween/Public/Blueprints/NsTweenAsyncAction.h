// Copyright (C) 2025 mykaadev. All rights reserved.

#pragma once

#include "Classes/NsTweenEasing.h"
#include "Kismet/BlueprintAsyncActionBase.h"
#include "NsTweenAsyncAction.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FTweenEventOutputPin);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FTweenUpdateFloatOutputPin, float, Value);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FTweenUpdateQuatOutputPin, FQuat, Value);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FTweenUpdateRotatorOutputPin, FRotator, Value);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FTweenUpdateVectorOutputPin, FVector, Value);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FTweenUpdateVector2DOutputPin, FVector2D, Value);

UCLASS(Abstract, BlueprintType, meta = (ExposedAsyncProxy = AsyncTask))
class NSTWEEN_API UNsTweenAsyncAction : public UBlueprintAsyncActionBase
{
    GENERATED_BODY()

// Functions
public:

    /** Activate */
    virtual void Activate() override;

    /** Create Tween */
    virtual struct FNsTweenInstance* CreateTween();

    /** Create Tween with custom curve */
    virtual struct FNsTweenInstance* CreateTweenCustomCurve();

    /** Set shared tween properties */
    virtual void SetSharedTweenProperties(float InDurationSecs, float InDelay, int InLoops, float InLoopDelay, bool InbPingPong, float InPingPongDelay, bool bInCanTickDuringPause, bool bInUseGlobalTimeDilation);

    /** Begin Destroy */
    virtual void BeginDestroy() override;

    /** Pause */
    UFUNCTION(BlueprintCallable, Category = "Tween")
    void Pause();

    /** Unpause */
    UFUNCTION(BlueprintCallable, Category = "Tween")
    void Unpause();

    /** Restart */
    UFUNCTION(BlueprintCallable, Category = "Tween")
    void Restart();

    /** Stop */
    UFUNCTION(BlueprintCallable, Category = "Tween")
    void Stop();

    /** Set time multiplier */
    UFUNCTION(BlueprintCallable, Category = "Tween")
    void SetTimeMultiplier(float Multiplier);

// Variables
public:

    /** Duration Seconds */
    float DurationSecs;

    /** Ease type */
    ENsTweenEase EaseType;

    /** Delay */
    float Delay;

    /** Loops */
    int Loops;

    /** Loop delay */
    float LoopDelay;

    /** PingPong */
    bool bPingPong;

    /** PingPong delay */
    float PingPongDelay;

    /** Can tick during pause */
    bool bCanTickDuringPause;

    /** Use global time dilation */
    bool bUseGlobalTimeDilation;

    /** Ease Param One */
    float EaseParam1;

    /** Ease Param Two */
    float EaseParam2;

    /** Use Custom curve */
    bool bUseCustomCurve;

    /** Custom Curve */
    UPROPERTY()
    TObjectPtr<class UCurveFloat> CustomCurve;

    /** Tween Instance */
    FNsTweenInstance* TweenInstance = nullptr;

    /** On Loop */
    UPROPERTY(BlueprintAssignable, AdvancedDisplay)
    FTweenEventOutputPin OnLoop;

    /** On PingPong */
    UPROPERTY(BlueprintAssignable, AdvancedDisplay)
    FTweenEventOutputPin OnPingPong;

    /** On Complete */
    UPROPERTY(BlueprintAssignable, AdvancedDisplay)
    FTweenEventOutputPin OnComplete;
};

UCLASS()
class NSTWEEN_API UNsTweenAsyncActionFloat : public UNsTweenAsyncAction
{
    GENERATED_BODY()

// Variables
public:

    /** Start */
    float Start;

    /** End */
    float End;

    /** Triggered every tween update. use "Value" to get the tweened float for this frame */
    UPROPERTY(BlueprintAssignable)
    FTweenUpdateFloatOutputPin ApplyEasing;

    /**
     * Tween a float parameter between the given values
     * @param Start The starting value
     * @param End The ending value
     * @param DurationSecs The seconds to go from start to end
     * @param EaseType The type of easing function to use for interpolation
     * @param EaseParam1 Elastic: Amplitude (1.0) / Back: Overshoot (1.70158) / Stepped: Steps (10) / Smoothstep: x0 (0)
     * @param EaseParam2 Elastic: Period (0.2) / Smoothstep: x1 (1)
     * @param Delay Seconds before the tween starts interpolating, after being created
     * @param Loops The number of loops to play. -1 for infinite
     * @param LoopDelay Seconds to pause before starting each loop
     * @param bPingPong Whether to "pingpong" the tween - once it reaches the end, it starts counting backwards
     * @param PingPongDelay Seconds to pause before starting to pingpong
     * @param bCanTickDuringPause Whether to play this tween while the game is paused. Useful for UI purposes, such as a pause menu
     */
    UFUNCTION(BlueprintCallable, meta = (BlueprintInternalUseOnly = "true", AdvancedDisplay = "4"), Category = "Tween")
    static UNsTweenAsyncActionFloat* TweenFloat(float Start = 0.0f, float End = 1.0f, float DurationSecs = 1.0f, ENsTweenEase EaseType = ENsTweenEase::InOutQuad, float EaseParam1 = 0, float EaseParam2 = 0, float Delay = 0, int Loops = 0, float LoopDelay = 0, bool bPingPong = false, float PingPongDelay = 0, bool bCanTickDuringPause = false, bool bUseGlobalTimeDilation = true);

    /**
     * @brief Tween a float parameter between the given values
     * @param Start The starting value
     * @param End The ending value
     * @param DurationSecs The seconds to go from start to end
     * @param Curve The curve to interpolate with
     * @param Delay Seconds before the tween starts interpolating, after being created
     * @param Loops The number of loops to play. -1 for infinite
     * @param LoopDelay Seconds to pause before starting each loop
     * @param bPingPong Whether to "pingpong" the tween - once it reaches the end, it starts counting backwards
     * @param PingPongDelay Seconds to pause before starting to pingpong
     * @param bCanTickDuringPause Whether to play this tween while the game is paused. Useful for UI purposes, such as a pause menu
     */
    UFUNCTION(BlueprintCallable, meta = (BlueprintInternalUseOnly = "true", AdvancedDisplay = "4"), Category = "Tween|Custom Curve")
    static UNsTweenAsyncActionFloat* TweenFloatCustomCurve(float Start = 0.0f, float End = 1.0f, float DurationSecs = 1.0f, UCurveFloat* Curve = nullptr, float Delay = 0, int Loops = 0, float LoopDelay = 0, bool bPingPong = false, float PingPongDelay = 0, bool bCanTickDuringPause = false, bool bUseGlobalTimeDilation = true);

    //~ Begin UNsTweenAsyncAction Interface
    virtual FNsTweenInstance* CreateTween() override;
    virtual FNsTweenInstance* CreateTweenCustomCurve() override;
    //~ End UNsTweenAsyncAction Interface
};

UCLASS()
class NSTWEEN_API UNsTweenAsyncActionQuat : public UNsTweenAsyncAction
{
    GENERATED_BODY()

// Variables
public:

    /** Start */
    FQuat Start;

    /** End */
    FQuat End;

    /** Triggered every tween update. use "Value" to get the tweened float for this frame */
    UPROPERTY(BlueprintAssignable)
    FTweenUpdateQuatOutputPin ApplyEasing;


// Functions
public:

    /**
     * @brief Tween a Quaternion parameter between the given values
     * @param Start The starting value
     * @param End The ending value
     * @param DurationSecs The seconds to go from start to end
     * @param EaseType The type of easing function to use for interpolation
     * @param EaseParam1 Elastic: Amplitude (1.0) / Back: Overshoot (1.70158) / Stepped: Steps (10) / Smoothstep: x0 (0)
     * @param EaseParam2 Elastic: Period (0.2) / Smoothstep: x1 (1)
     * @param Delay Seconds before the tween starts interpolating, after being created
     * @param Loops The number of loops to play. -1 for infinite
     * @param LoopDelay Seconds to pause before starting each loop
     * @param bPingPong Whether to "pingpong" the tween - once it reaches the end, it starts counting backwards
     * @param PingPongDelay Seconds to pause before starting to pingpong
     * @param bCanTickDuringPause Whether to play this tween while the game is paused. Useful for UI purposes, such as a pause menu
     */
    UFUNCTION(BlueprintCallable, meta = (BlueprintInternalUseOnly = "true", AdvancedDisplay = "4"), Category = "Tween")
    static UNsTweenAsyncActionQuat* TweenQuat(FQuat Start, FQuat End, float DurationSecs = 1.0f, ENsTweenEase EaseType = ENsTweenEase::InOutQuad, float EaseParam1 = 0, float EaseParam2 = 0, float Delay = 0, int Loops = 0, float LoopDelay = 0, bool bPingPong = false, float PingPongDelay = 0, bool bCanTickDuringPause = false, bool bUseGlobalTimeDilation = true);

    /**
     * @brief Tweens a quaternion, but you can enter in yaw/pitch/roll as the input
     * @param Start The starting value
     * @param End The ending value
     * @param DurationSecs The seconds to go from start to end
     * @param EaseType The type of easing function to use for interpolation
     * @param EaseParam1 Elastic: Amplitude (1.0) / Back: Overshoot (1.70158) / Stepped: Steps (10) / Smoothstep: x0 (0)
     * @param EaseParam2 Elastic: Period (0.2) / Smoothstep: x1 (1)
     * @param Delay Seconds before the tween starts interpolating, after being created
     * @param Loops The number of loops to play. -1 for infinite
     * @param LoopDelay Seconds to pause before starting each loop
     * @param bPingPong Whether to "pingpong" the tween - once it reaches the end, it starts counting backwards
     * @param PingPongDelay Seconds to pause before starting to pingpong
     * @param bCanTickDuringPause Whether to play this tween while the game is paused. Useful for UI purposes, such as a pause menu
     */
    UFUNCTION(BlueprintCallable, meta = (BlueprintInternalUseOnly = "true", AdvancedDisplay = "4"), Category = "Tween")
    static UNsTweenAsyncActionQuat* TweenQuatFromRotator(FRotator Start = FRotator::ZeroRotator, FRotator End = FRotator::ZeroRotator, float DurationSecs = 1.0f, ENsTweenEase EaseType = ENsTweenEase::InOutQuad, float EaseParam1 = 0, float EaseParam2 = 0, float Delay = 0, int Loops = 0, float LoopDelay = 0, bool bPingPong = false, float PingPongDelay = 0, bool bCanTickDuringPause = false, bool bUseGlobalTimeDilation = true);

    /**
     * @brief Tween a float parameter between the given values
     * @param Start The starting value
     * @param End The ending value
     * @param DurationSecs The seconds to go from start to end
     * @param Curve The curve to interpolate with
     * @param Delay Seconds before the tween starts interpolating, after being created
     * @param Loops The number of loops to play. -1 for infinite
     * @param LoopDelay Seconds to pause before starting each loop
     * @param bPingPong Whether to "pingpong" the tween - once it reaches the end, it starts counting backwards
     * @param PingPongDelay Seconds to pause before starting to pingpong
     * @param bCanTickDuringPause Whether to play this tween while the game is paused. Useful for UI purposes, such as a pause menu
     */
    UFUNCTION(BlueprintCallable, meta = (BlueprintInternalUseOnly = "true", AdvancedDisplay = "4"), Category = "Tween|Custom Curve")
    static UNsTweenAsyncActionQuat* TweenQuatCustomCurve(FQuat Start, FQuat End, float DurationSecs = 1.0f, UCurveFloat* Curve = nullptr, float Delay = 0, int Loops = 0, float LoopDelay = 0, bool bPingPong = false, float PingPongDelay = 0, bool bCanTickDuringPause = false, bool bUseGlobalTimeDilation = true);

    /**
     * @brief Tween a float parameter between the given values
     * @param Start The starting value
     * @param End The ending value
     * @param DurationSecs The seconds to go from start to end
     * @param Curve The curve to interpolate with
     * @param Delay Seconds before the tween starts interpolating, after being created
     * @param Loops The number of loops to play. -1 for infinite
     * @param LoopDelay Seconds to pause before starting each loop
     * @param bPingPong Whether to "pingpong" the tween - once it reaches the end, it starts counting backwards
     * @param PingPongDelay Seconds to pause before starting to pingpong
     * @param bCanTickDuringPause Whether to play this tween while the game is paused. Useful for UI purposes, such as a pause menu
     */
    UFUNCTION(BlueprintCallable, meta = (BlueprintInternalUseOnly = "true", AdvancedDisplay = "4"), Category = "Tween|Custom Curve")
    static UNsTweenAsyncActionQuat* TweenQuatFromRotatorCustomCurve(FRotator Start = FRotator::ZeroRotator, FRotator End = FRotator::ZeroRotator, float DurationSecs = 1.0f, UCurveFloat* Curve = nullptr, float Delay = 0, int Loops = 0, float LoopDelay = 0, bool bPingPong = false, float PingPongDelay = 0, bool bCanTickDuringPause = false, bool bUseGlobalTimeDilation = true);

    //~ Begin UNsTweenAsyncAction Interface
    virtual FNsTweenInstance* CreateTween() override;
    virtual FNsTweenInstance* CreateTweenCustomCurve() override;
    //~ End UNsTweenAsyncAction Interface
};

UCLASS()
class NSTWEEN_API UNsTweenAsyncActionRotator : public UNsTweenAsyncAction
{
    GENERATED_BODY()

// Variables
public:

    /** Start */
    FQuat Start;

    /** End */
    FQuat End;

    /** Triggered every tween update. use "Value" to get the tweened float for this frame */
    UPROPERTY(BlueprintAssignable)
    FTweenUpdateRotatorOutputPin ApplyEasing;

    /**
     * @brief Under the hood this just tweens a quaternion, but for convenience the input/output are converted to Rotators
     * @param Start The starting value
     * @param End The ending value
     * @param DurationSecs The seconds to go from start to end
     * @param EaseType The type of easing function to use for interpolation
     * @param EaseParam1 Elastic: Amplitude (1.0) / Back: Overshoot (1.70158) / Stepped: Steps (10) / Smoothstep: x0 (0)
     * @param EaseParam2 Elastic: Period (0.2) / Smoothstep: x1 (1)
     * @param Delay Seconds before the tween starts interpolating, after being created
     * @param Loops The number of loops to play. -1 for infinite
     * @param LoopDelay Seconds to pause before starting each loop
     * @param bPingPong Whether to "pingpong" the tween - once it reaches the end, it starts counting backwards
     * @param PingPongDelay Seconds to pause before starting to pingpong
     * @param bCanTickDuringPause Whether to play this tween while the game is paused. Useful for UI purposes, such as a pause menu
     */
    UFUNCTION(BlueprintCallable, meta = (BlueprintInternalUseOnly = "true", AdvancedDisplay = "4"), Category = "Tween")
    static UNsTweenAsyncActionRotator* TweenRotator(FRotator Start = FRotator::ZeroRotator, FRotator End = FRotator::ZeroRotator, float DurationSecs = 1.0f, ENsTweenEase EaseType = ENsTweenEase::InOutQuad, float EaseParam1 = 0, float EaseParam2 = 0, float Delay = 0, int Loops = 0, float LoopDelay = 0, bool bPingPong = false, float PingPongDelay = 0, bool bCanTickDuringPause = false, bool bUseGlobalTimeDilation = true);
    /**
     * @brief Tween a float parameter between the given values
     * @param Start The starting value
     * @param End The ending value
     * @param DurationSecs The seconds to go from start to end
     * @param Curve The curve to interpolate with
     * @param Delay Seconds before the tween starts interpolating, after being created
     * @param Loops The number of loops to play. -1 for infinite
     * @param LoopDelay Seconds to pause before starting each loop
     * @param bPingPong Whether to "pingpong" the tween - once it reaches the end, it starts counting backwards
     * @param PingPongDelay Seconds to pause before starting to pingpong
     * @param bCanTickDuringPause Whether to play this tween while the game is paused. Useful for UI purposes, such as a pause menu
     */
    UFUNCTION(BlueprintCallable, meta = (BlueprintInternalUseOnly = "true", AdvancedDisplay = "4"), Category = "Tween|Custom Curve")
    static UNsTweenAsyncActionRotator* TweenRotatorCustomCurve(FRotator Start = FRotator::ZeroRotator, FRotator End = FRotator::ZeroRotator, float DurationSecs = 1.0f, UCurveFloat* Curve = nullptr, float Delay = 0, int Loops = 0, float LoopDelay = 0, bool bPingPong = false, float PingPongDelay = 0, bool bCanTickDuringPause = false, bool bUseGlobalTimeDilation = true);

    //~ Begin UNsTweenAsyncAction Interface
    virtual FNsTweenInstance* CreateTween() override;
    virtual FNsTweenInstance* CreateTweenCustomCurve() override;
    //~ End UNsTweenAsyncAction Interface
};

UCLASS()
class NSTWEEN_API UNsTweenAsyncActionVector : public UNsTweenAsyncAction
{
    GENERATED_BODY()

// Variables
public:

    /** Start */
    FVector Start;

    /** End */
    FVector End;

    /** Triggered every tween update. use "Value" to get the tweened float for this frame */
    UPROPERTY(BlueprintAssignable)
    FTweenUpdateVectorOutputPin ApplyEasing;

// Functions
public:

    /**
     * @brief Tween a Vector parameter between the given values
     * @param Start The starting value
     * @param End The ending value
     * @param DurationSecs The seconds to go from start to end
     * @param EaseType The type of easing function to use for interpolation
     * @param EaseParam1 Elastic: Amplitude (1.0) / Back: Overshoot (1.70158) / Stepped: Steps (10) / Smoothstep: x0 (0)
     * @param EaseParam2 Elastic: Period (0.2) / Smoothstep: x1 (1)
     * @param Delay Seconds before the tween starts interpolating, after being created
     * @param Loops The number of loops to play. -1 for infinite
     * @param LoopDelay Seconds to pause before starting each loop
     * @param bPingPong Whether to "pingpong" the tween - once it reaches the end, it starts counting backwards
     * @param PingPongDelay Seconds to pause before starting to pingpong
     * @param bCanTickDuringPause Whether to play this tween while the game is paused. Useful for UI purposes, such as a pause menu
     */
    UFUNCTION(BlueprintCallable, meta = (BlueprintInternalUseOnly = "true", AdvancedDisplay = "4"), Category = "Tween")
    static UNsTweenAsyncActionVector* TweenVector(FVector Start = FVector::ZeroVector, FVector End = FVector::ZeroVector, float DurationSecs = 1.0f, ENsTweenEase EaseType = ENsTweenEase::InOutQuad, float EaseParam1 = 0, float EaseParam2 = 0, float Delay = 0, int Loops = 0, float LoopDelay = 0, bool bPingPong = false, float PingPongDelay = 0, bool bCanTickDuringPause = false, bool bUseGlobalTimeDilation = true);

    /**
     * @brief Tween a float parameter between the given values
     * @param Start The starting value
     * @param End The ending value
     * @param DurationSecs The seconds to go from start to end
     * @param Curve The curve to interpolate with
     * @param Delay Seconds before the tween starts interpolating, after being created
     * @param Loops The number of loops to play. -1 for infinite
     * @param LoopDelay Seconds to pause before starting each loop
     * @param bPingPong Whether to "pingpong" the tween - once it reaches the end, it starts counting backwards
     * @param PingPongDelay Seconds to pause before starting to pingpong
     * @param bCanTickDuringPause Whether to play this tween while the game is paused. Useful for UI purposes, such as a pause menu
     */
    UFUNCTION(BlueprintCallable, meta = (BlueprintInternalUseOnly = "true", AdvancedDisplay = "4"), Category = "Tween|Custom Curve")
    static UNsTweenAsyncActionVector* TweenVectorCustomCurve(FVector Start = FVector::ZeroVector, FVector End = FVector::ZeroVector, float DurationSecs = 1.0f, UCurveFloat* Curve = nullptr, float Delay = 0, int Loops = 0, float LoopDelay = 0, bool bPingPong = false, float PingPongDelay = 0, bool bCanTickDuringPause = false, bool bUseGlobalTimeDilation = true);

    //~ Begin UNsTweenAsyncAction Interface
    virtual FNsTweenInstance* CreateTween() override;
    virtual FNsTweenInstance* CreateTweenCustomCurve() override;
    //~ End UNsTweenAsyncAction Interface
};

/**  */
UCLASS()
class NSTWEEN_API UNsTweenAsyncActionVector2D : public UNsTweenAsyncAction
{
    GENERATED_BODY()

// Variables
public:

    /** Start */
    FVector2D Start;

    /** End */
    FVector2D End;

    /** Triggered every tween update. use "Value" to get the tweened float for this frame */
    UPROPERTY(BlueprintAssignable)
    FTweenUpdateVector2DOutputPin ApplyEasing;

// Functions
public:

    /**
     * @brief Tween a Vector parameter between the given values
     * @param Start The starting value
     * @param End The ending value
     * @param DurationSecs The seconds to go from start to end
     * @param EaseType The type of easing function to use for interpolation
     * @param EaseParam1 Elastic: Amplitude (1.0) / Back: Overshoot (1.70158) / Stepped: Steps (10) / Smoothstep: x0 (0)
     * @param EaseParam2 Elastic: Period (0.2) / Smoothstep: x1 (1)
     * @param Delay Seconds before the tween starts interpolating, after being created
     * @param Loops The number of loops to play. -1 for infinite
     * @param LoopDelay Seconds to pause before starting each loop
     * @param bPingPong Whether to "pingpong" the tween - once it reaches the end, it starts counting backwards
     * @param PingPongDelay Seconds to pause before starting to pingpong
     * @param bCanTickDuringPause Whether to play this tween while the game is paused. Useful for UI purposes, such as a pause menu
     */
    UFUNCTION(BlueprintCallable, meta = (BlueprintInternalUseOnly = "true", AdvancedDisplay = "4"), Category = "Tween")
    static UNsTweenAsyncActionVector2D* TweenVector2D(FVector2D Start = FVector2D::ZeroVector, FVector2D End = FVector2D::ZeroVector, float DurationSecs = 1.0f, ENsTweenEase EaseType = ENsTweenEase::InOutQuad, float EaseParam1 = 0, float EaseParam2 = 0, float Delay = 0, int Loops = 0, float LoopDelay = 0, bool bPingPong = false, float PingPongDelay = 0, bool bCanTickDuringPause = false, bool bUseGlobalTimeDilation = true);

    /**
     * @brief Tween a float parameter between the given values
     * @param Start The starting value
     * @param End The ending value
     * @param DurationSecs The seconds to go from start to end
     * @param Curve The curve to interpolate with
     * @param Delay Seconds before the tween starts interpolating, after being created
     * @param Loops The number of loops to play. -1 for infinite
     * @param LoopDelay Seconds to pause before starting each loop
     * @param bPingPong Whether to "pingpong" the tween - once it reaches the end, it starts counting backwards
     * @param PingPongDelay Seconds to pause before starting to pingpong
     * @param bCanTickDuringPause Whether to play this tween while the game is paused. Useful for UI purposes, such as a pause menu
     */
    UFUNCTION(BlueprintCallable, meta = (BlueprintInternalUseOnly = "true", AdvancedDisplay = "4", DisplayName = "Tween Vector 2D Custom Curve"), Category = "Tween|Custom Curve")
    static UNsTweenAsyncActionVector2D* TweenVector2DCustomCurve(FVector2D Start = FVector2D::ZeroVector, FVector2D End = FVector2D::ZeroVector, float DurationSecs = 1.0f, UCurveFloat* Curve = nullptr, float Delay = 0, int Loops = 0, float LoopDelay = 0, bool bPingPong = false, float PingPongDelay = 0, bool bCanTickDuringPause = false, bool bUseGlobalTimeDilation = true);

    //~ Begin UNsTweenAsyncAction Interface
    virtual FNsTweenInstance* CreateTween() override;
    virtual FNsTweenInstance* CreateTweenCustomCurve() override;
    //~ End UNsTweenAsyncAction Interface
};
