// Copyright (C) 2025 mykaadev. All rights reserved.

#pragma once

#include "CoreMinimal.h"

/**
 * Enum used to represent the Ease type
 */
UENUM(BlueprintType)
enum class ENsTweenEase : uint8
{
    Linear,
    Smoothstep,
    Stepped,
    InSine,
    OutSine,
    InOutSine,
    InQuad,
    OutQuad,
    InOutQuad,
    InCubic,
    OutCubic,
    InOutCubic,
    InQuart,
    OutQuart,
    InOutQuart,
    InQuint,
    OutQuint,
    InOutQuint,
    // Like Quintic but even sharper
    InExpo,
    OutExpo,
    InOutExpo,
    InCirc,
    OutCirc,
    InOutCirc,
    InElastic,
    OutElastic,
    InOutElastic,
    InBounce,
    OutBounce,
    InOutBounce,
    // Anticipation; pull back a little before going forward
    InBack,
    OutBack,
    InOutBack,
};

/**
 * Tween Delay state
 */
UENUM()
enum class ENsTweenDelayState : uint8
{
    None,
    Start,
    Loop,
    PingPong
};

/**
 * Tween Instance
 */
struct NSTWEEN_API FNsTweenInstance
{

// Variables
private:

    /** Duration */
    float DurationSecs;

    /** Ease Type */
    ENsTweenEase EaseType;

    /** Counter */
    float Counter;

    /**  Delay Counter */
    float DelayCounter;

    /** Should auto destroy */
    bool bShouldAutoDestroy;

    /** Is the tween active */
    bool bIsActive;

    /** Is the tween paused */
    bool bIsPaused;

    /** Should PingPong */
    bool bShouldPingPong;

    /** is the tween playing PingPong */
    bool bIsPlayingPingPong;

    /** Can tick during pause */
    bool bCanTickDuringPause;

    /** Should use global time dilation */
    bool bUseGlobalTimeDilation;

    /** Number of loops */
    int NumLoops;

    /** Number of loops completed */
    int NumLoopsCompleted;

    /** Delay seconds */
    float DelaySecs;

    /** Loop delay seconds */
    float LoopDelaySecs;

    /** PingPong delay seconds */
    float PingPongDelaySecs;

    /** Time multiplier */
    float TimeMultiplier;

    /** Ease parameter one */
    float EaseParam1;

    /** Ease parameter two */
    float EaseParam2;

    /** Tween delay state */
    ENsTweenDelayState DelayState;

    /** Function to run on PingPong */
    TFunction<void()> OnPingPongCallback;

    /** Function to run on Loop */
    TFunction<void()> OnLoopCallback;

    /** Function to run on Complete */
    TFunction<void()> OnCompleteCallback;

// Functions
public:

    /** Assign callback for PingPong */
    FNsTweenInstance& OnPingPong(TFunction<void()> Handler);

    /** Assign callback for Loop */
    FNsTweenInstance& OnLoop(TFunction<void()> Handler);

    /** Assign callback for Complete */
    FNsTweenInstance& OnComplete(TFunction<void()> Handler);

    /** Enable/disable pingpong behaviour */
    FNsTweenInstance& SetPingPong(bool bPingPong);

    /** Get is tween active */
    bool IsActive() const;

    /** Set tween active state */
    void SetActive(const bool InState);

    /** Get whether pingpong is enabled */
    bool GetPingPong() const;

    /** Set number of loops */
    FNsTweenInstance& SetLoops(int InLoops);

    /** Get configured loop count */
    int GetLoops() const;

    /** Set delay before tween starts */
    FNsTweenInstance& SetDelay(float InDelay);

    /** Get delay before tween starts */
    float GetDelay() const;

    /** Set delay between loops */
    FNsTweenInstance& SetLoopDelay(float InLoopDelay);

    /** Get delay between loops */
    float GetLoopDelay() const;

    /** Set delay before pingpong */
    FNsTweenInstance& SetPingPongDelay(float InPingPongDelay);

    /** Get delay before pingpong */
    float GetPingPongDelay() const;

    /** Allow ticking while game is paused */
    FNsTweenInstance& SetCanTickDuringPause(bool bInCanTickDuringPause);

    /** Check if tween ticks while game is paused */
    bool GetCanTickDuringPause() const;

    /** Use global time dilation */
    FNsTweenInstance& SetUseGlobalTimeDilation(bool bInUseGlobalTimeDilation);

    /** Check if global time dilation is used */
    bool GetUseGlobalTimeDilation() const;

    /** Set whether tween should auto destroy */
    FNsTweenInstance& SetAutoDestroy(bool bInShouldAutoDestroy);

    /** Get auto destroy flag */
    bool GetAutoDestroy() const;

    /** Set time multiplier */
    FNsTweenInstance& SetTimeMultiplier(float InMultiplier);

    /** Get time multiplier */
    float GetTimeMultiplier() const;

    /** Set easing parameter one */
    FNsTweenInstance& SetEaseParam1(float InEaseParam1);

    /** Get easing parameter one */
    float GetEaseParam1() const;

    /** Set easing parameter two */
    FNsTweenInstance& SetEaseParam2(float InEaseParam2);

    /** Get easing parameter two */
    float GetEaseParam2() const;


    /** Constructor */
    FNsTweenInstance()
        : DurationSecs(0)
        , EaseType()
        , Counter(0)
        , DelayCounter(0)
        , bShouldAutoDestroy(true)
        , bIsActive(true)
        , bIsPaused(false)
        , bShouldPingPong(false)
        , bIsPlayingPingPong(false)
        , bCanTickDuringPause(false)
        , bUseGlobalTimeDilation(true)
        , NumLoops(0)
        , NumLoopsCompleted(0)
        , DelaySecs(0)
        , LoopDelaySecs(0)
        , PingPongDelaySecs(0)
        , TimeMultiplier(0)
        , EaseParam1(0)
        , EaseParam2(0)
        , DelayState()
    {}

    /** Destructor */
    virtual ~FNsTweenInstance()
    {}

    /** Reset variables and start a fresh tween */
    void InitializeSharedMembers(const float InDurationSecs, const ENsTweenEase InEaseType);

    /** Called on the first frame this tween is updated, to set up any options that have been defined */
    void Start();

    /** Takes the existing tween settings and restarts the timer, to play it again from the start */
    void Restart();

    /** Stop tweening and mark this instance for recycling */
    void Destroy();

    /** Pause */
    void Pause();

    /** Unpause */
    void Unpause();

    /** Update  */
    void Update(float UnscaledDeltaSeconds, float DilatedDeltaSeconds, bool bIsGamePaused = false);

protected:

    /** Apply Easing */
    virtual void ApplyEasing(float EasedPercent) = 0;

private:

    /** Complete Loop */
    void CompleteLoop();

    /** Start New Loop */
    void StartNewLoop();

    /** Start PingPong */
    void StartPingPong();
};

/**
 * Tween Instance - Float
 */
struct NSTWEEN_API FNsTweenInstanceFloat : public FNsTweenInstance
{
// Functions
public:

    /** Initialize */
    void Initialize(float InStart, float InEnd, TFunction<void(float)> InOnUpdate, float InDurationSecs, ENsTweenEase InEaseType);

    //~ Begin NsTweenInstance Interface
    virtual void ApplyEasing(float EasedPercent) override;
    //~ End NsTweenInstance Interface

// Variables
public:

    /** Start Value */
    float StartValue;

    /** End Value */
    float EndValue;

    /** On Update callback */
    TFunction<void(float)> OnUpdate;
};

/**
 * Tween Instance - Quat
 */
struct NSTWEEN_API FNsTweenInstanceQuat : public FNsTweenInstance
{
// Functions
public:

    /** Initialize */
    void Initialize(FQuat InStart, FQuat InEnd, TFunction<void(FQuat)> InOnUpdate, float InDurationSecs, ENsTweenEase InEaseType);

    //~ Begin NsTweenInstance Interface
    virtual void ApplyEasing(float EasedPercent) override;
    //~ End NsTweenInstance Interface

// Variables
public:

    /** Start Value */
    FQuat StartValue;

    /** End Value */
    FQuat EndValue;

    /** On Update callback */
    TFunction<void(FQuat)> OnUpdate;
};

/**
 * Tween Instance - Vector
 */
struct NSTWEEN_API FNsTweenInstanceVector : public FNsTweenInstance
{
// Functions
public:

    /** Initialize */
    void Initialize(FVector InStart, FVector InEnd, TFunction<void(FVector)> InOnUpdate, float InDurationSecs, ENsTweenEase InEaseType);

    //~ Begin NsTweenInstance Interface
    virtual void ApplyEasing(float EasedPercent) override;
    //~ End NsTweenInstance Interface

// Variables
public:

    /** Start Value */
    FVector StartValue;

    /** End Value */
    FVector EndValue;

    /** On Update callback */
    TFunction<void(FVector)> OnUpdate;
};

/**
 * Tween Instance - Vector2D
 */
struct NSTWEEN_API FNsTweenInstanceVector2D : public FNsTweenInstance
{
// Functions
public:

    /** Initialize */
    void Initialize(FVector2D InStart, FVector2D InEnd, TFunction<void(FVector2D)> InOnUpdate, float InDurationSecs, ENsTweenEase InEaseType);

    //~ Begin NsTweenInstance Interface
    virtual void ApplyEasing(float EasedPercent) override;
    //~ End NsTweenInstance Interface

// Variables
public:

    /** Start Value */
    FVector2D StartValue;

    /** End Value */
    FVector2D EndValue;

    /** On Update callback */
    TFunction<void(FVector2D)> OnUpdate;
};

/**
 * Tween Instance - Rotator
 */
struct NSTWEEN_API FNsTweenInstanceRotator : public FNsTweenInstance
{
// Functions
public:

    /** Initialize */
    void Initialize(FRotator InStart, FRotator InEnd, TFunction<void(FRotator)> InOnUpdate, float InDurationSecs, ENsTweenEase InEaseType);

    //~ Begin NsTweenInstance Interface
    virtual void ApplyEasing(float EasedPercent) override;
    //~ End NsTweenInstance Interface

// Variables
public:

    /** Start Value */
    FRotator StartValue;

    /** End Value */
    FRotator EndValue;

    /** On Update callback */
    TFunction<void(FRotator)> OnUpdate;
};
