// Copyright (C) 2024 mykaa. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "NsTweenEasing.h"

/**
 * Tween Delay state
 */
UENUM()
enum class ENsTweenDelayState : uint8
{
    None,
    Start,
    Loop,
    YoYo
};

/**
 * Tween Instance
 */
class NSTWEEN_API NsTweenInstance
{

// Variables
public:

    /** Duration */
    float DurationSecs;

    /** Ease Type */
    ENsTweenEase EaseType;

    /** Counter */
    float Counter;

    /**  Delay Counter */
    float DelayCounter;

    /** Should auto destroy */
    uint8 bShouldAutoDestroy;

    /** Is the tween active */
    uint8 bIsActive;

    /** Is the tween paused */
    uint8 bIsPaused;

    /** Should YoYo */
    uint8 bShouldYoYo;

    /** is the tween playing YoYo */
    uint8 bIsPlayingYoYo;

    /** Can tick during pause */
    uint8 bCanTickDuringPause;

    /** Should use global time dilation */
    uint8 bUseGlobalTimeDilation;

    /** Number of loops */
    int NumLoops;

    /** Number of loops completed */
    int NumLoopsCompleted;

    /** Delay seconds */
    float DelaySecs;

    /** Loop delay seconds */
    float LoopDelaySecs;

    /** YoYo delay seconds */
    float YoyoDelaySecs;

    /** Time multiplier */
    float TimeMultiplier;

    /** Ease parameter one */
    float EaseParam1;

    /** Ease parameter two */
    float EaseParam2;

    /** Tween delay state */
    ENsTweenDelayState DelayState;

private:

    /** Function to run on YoYo */
    TFunction<void()> OnYoyo;

    /** Function to run on Loop */
    TFunction<void()> OnLoop;

    /** Function to run on Complete */
    TFunction<void()> OnComplete;

public:

    /** Constructor */
    NsTweenInstance()
        : DurationSecs(0)
        , EaseType()
        , Counter(0)
        , DelayCounter(0)
        , bShouldAutoDestroy(1)
        , bIsActive(1)
        , bIsPaused(1)
        , bShouldYoYo(1)
        , bIsPlayingYoYo(1)
        , bCanTickDuringPause(1)
        , bUseGlobalTimeDilation(1)
        , NumLoops(0)
        , NumLoopsCompleted(0)
        , DelaySecs(0)
        , LoopDelaySecs(0)
        , YoyoDelaySecs(0)
        , TimeMultiplier(0)
        , EaseParam1(0)
        , EaseParam2(0)
        , DelayState() {}

    /** Destructor */
    virtual ~NsTweenInstance()
    {}

    /** Set Delay */
    NsTweenInstance* SetDelay(float InDelaySecs);

    /** How many times to replay the loop (yoyo included). use -1 for infinity */
    NsTweenInstance* SetLoops(int InNumLoops);

    /** Seconds to wait before starting another loop */
    NsTweenInstance* SetLoopDelay(float InLoopDelaySecs);

    /** Interpolate backwards after reaching the end */
    NsTweenInstance* SetYoyo(bool bInShouldYoyo);

    /** Seconds to wait before yoyo-ing backwards */
    NsTweenInstance* SetYoyoDelay(float InYoyoDelaySecs);

    /** Multiply the time delta by this number to speed up or slow down the tween. Only positive numbers allowed */
    NsTweenInstance* SetTimeMultiplier(float InTimeMultiplier);

    /** Set EaseParam1 to fine-tune specific equations. Elastic: Amplitude (1.0) / Back: Overshoot (1.70158) / Stepped: Steps (10) / Smoothstep: x0 (0) */
    NsTweenInstance* SetEaseParam1(float InEaseParam1);

    /** Set EaseParam2 to fine-tune specific equations. Elastic: Period (0.2) / Smoothstep: x1 (1) */
    NsTweenInstance* SetEaseParam2(float InEaseParam2);

    /** Let this tween run while the game is paused */
    NsTweenInstance* SetCanTickDuringPause(bool bInCanTickDuringPause);

    /** Let this tween run while the game is paused */
    NsTweenInstance* SetUseGlobalTimeDilation(bool bInUseGlobalTimeDilation);

    /** Automatically recycles this instance after tween is complete (Stop() is called)*/
    NsTweenInstance* SetAutoDestroy(bool bInShouldAutoDestroy);

    /** Callback on YoYo */
    NsTweenInstance* SetOnYoyo(TFunction<void()> Handler);

    /** Callback on Loop */
    NsTweenInstance* SetOnLoop(TFunction<void()> Handler);

    /** Callback on Complete */
    NsTweenInstance* SetOnComplete(TFunction<void()> Handler);

    /** Reset variables and start a fresh tween */
    void InitializeSharedMembers(float InDurationSecs, ENsTweenEase InEaseType);

    /** Called on the first frame this tween is updated, to set up any options that have been defined */
    void Start();

    /** Takes the existing tween settings and restarts the timer, to play it again from the start */
    void Restart();

    /** Stop tweening and mark this instance for recycling */
    void Destroy();

    /** Get a UObject wrapper for this tween. It will recycle the tween automatically when it's Outer is destroyed */
    class UNsTweenUObject* CreateUObject(UObject* Outer = (UObject*)GetTransientPackage());

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

    /** Start YoYo */
    void StartYoyo();
};

/**
 * Tween Instance - Float
 */
class NSTWEEN_API NsTweenInstanceFloat : public NsTweenInstance
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
class NSTWEEN_API NsTweenInstanceQuat : public NsTweenInstance
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
class NSTWEEN_API NsTweenInstanceVector : public NsTweenInstance
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
class NSTWEEN_API NsTweenInstanceVector2D : public NsTweenInstance
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
class NSTWEEN_API NsTweenInstanceRotator : public NsTweenInstance
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
