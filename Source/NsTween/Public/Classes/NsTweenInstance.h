// Copyright (C) 2025 mykaadev. All rights reserved.

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
    PingPong
};

/**
 * Tween Instance
 */
struct NSTWEEN_API NsTweenInstance
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

public:

    /** Function to run on PingPong */
    TFunction<void()> OnPingPongCallback;

    /** Function to run on Loop */
    TFunction<void()> OnLoopCallback;

    /** Function to run on Complete */
    TFunction<void()> OnCompleteCallback;

    /** Assign callback for PingPong */
    NsTweenInstance& OnPingPong(TFunction<void()> Handler);

    /** Assign callback for Loop */
    NsTweenInstance& OnLoop(TFunction<void()> Handler);

    /** Assign callback for Complete */
    NsTweenInstance& OnComplete(TFunction<void()> Handler);


    /** Constructor */
    NsTweenInstance()
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
        , DelayState() {}

    /** Destructor */
    virtual ~NsTweenInstance()
    {}


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

    /** Start PingPong */
    void StartPingPong();
};

/**
 * Tween Instance - Float
 */
struct NSTWEEN_API NsTweenInstanceFloat : public NsTweenInstance
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
struct NSTWEEN_API NsTweenInstanceQuat : public NsTweenInstance
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
struct NSTWEEN_API NsTweenInstanceVector : public NsTweenInstance
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
struct NSTWEEN_API NsTweenInstanceVector2D : public NsTweenInstance
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
struct NSTWEEN_API NsTweenInstanceRotator : public NsTweenInstance
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
