// Copyright (C) 2025 mykaadev. All rights reserved.

#include "Classes/NsTweenInstance.h"
#include "Classes/NsTweenUObject.h"


void NsTweenInstance::InitializeSharedMembers(float InDurationSecs, ENsTweenEase InEaseType)
{
    checkf(InDurationSecs > 0, TEXT("Tween received duration <= 0"));

    if (InDurationSecs <= 0)
    {
        DurationSecs = .001f;
    }
    else
    {
        DurationSecs = InDurationSecs;
    }

    EaseType = InEaseType;
    Counter = 0;
    DelayCounter = 0;
    bShouldAutoDestroy = true;
    bIsActive = true;
    bIsPaused = false;
    bShouldPingPong = false;
    bIsPlayingPingPong = false;
    bCanTickDuringPause = false;
    bUseGlobalTimeDilation = true;

    NumLoops = 1;
    NumLoopsCompleted = 0;
    DelaySecs = 0;
    LoopDelaySecs = 0;
    PingPongDelaySecs = 0;
    EaseParam1 = 0;
    EaseParam2 = 0;
    TimeMultiplier = 1.0f;

    DelayState = ENsTweenDelayState::None;

#if ENGINE_MAJOR_VERSION < 5
    OnPingPongCallback = nullptr;
    OnLoopCallback = nullptr;
    OnCompleteCallback = nullptr;
#else
    OnPingPongCallback.Reset();
    OnLoopCallback.Reset();
    OnCompleteCallback.Reset();
#endif
}

void NsTweenInstance::Start()
{
    DelayCounter = DelaySecs;
    if (DelayCounter > 0)
    {
        DelayState = ENsTweenDelayState::Start;
    }
    else
    {
        DelayState = ENsTweenDelayState::None;
    }
}

void NsTweenInstance::Restart()
{
    if (bIsActive)
    {
        Counter = 0;
        bIsPlayingPingPong = false;
        NumLoopsCompleted = 0;
        Unpause();
        Start();
    }
}

void NsTweenInstance::Destroy()
{
    // Mark for recycling
    bIsActive = false;

#if ENGINE_MAJOR_VERSION < 5
    OnLoopCallback  = nullptr;
    OnPingPongCallback  = nullptr;
    OnCompleteCallback = nullptr;
#else
    OnLoopCallback.Reset();
    OnPingPongCallback.Reset();
    OnCompleteCallback.Reset();
#endif
}

UNsTweenUObject* NsTweenInstance::CreateUObject(UObject* Outer)
{
    UNsTweenUObject* Wrapper = NewObject<UNsTweenUObject>(Outer);
    Wrapper->SetTweenInstance(this);
    return Wrapper;
}

void NsTweenInstance::Pause()
{
    bIsPaused = true;
}

void NsTweenInstance::Unpause()
{
    bIsPaused = false;
}

void NsTweenInstance::Update(float UnscaledDeltaSeconds, float DilatedDeltaSeconds, bool bIsGamePaused)
{
    if ((bIsPaused || !bIsActive || bIsGamePaused) && !bCanTickDuringPause)
    {
        return;
    }

    float DeltaTime = bUseGlobalTimeDilation ? DilatedDeltaSeconds : UnscaledDeltaSeconds;
    DeltaTime *= TimeMultiplier;

    if (DelayCounter > 0)
    {
        DelayCounter -= DeltaTime;
        if (DelayCounter <= 0)
        {
            switch (DelayState)
            {
                case ENsTweenDelayState::Loop:
                    if (OnLoopCallback)
                    {
                        OnLoopCallback();
                    }
                break;

                case ENsTweenDelayState::PingPong:
                    if (OnPingPongCallback)
                    {
                        OnPingPongCallback();
                    }
                break;
            }
        }
    }
    else
    {
        if (bIsPlayingPingPong)
        {
            Counter -= DeltaTime;
        }
        else
        {
            Counter += DeltaTime;
        }

        Counter = FMath::Clamp<float>(Counter, 0, DurationSecs);

        ApplyEasing(NsTweenEasing::EaseWithParams(Counter / DurationSecs, EaseType, EaseParam1, EaseParam2));

        if (bIsPlayingPingPong)
        {
            if (Counter <= 0)
            {
                CompleteLoop();
            }
        }
        else
        {
            if (Counter >= DurationSecs)
            {
                if (bShouldPingPong)
                {
                    StartPingPong();
                }
                else
                {
                    CompleteLoop();
                }
            }
        }
    }
}

void NsTweenInstance::CompleteLoop()
{
    ++NumLoopsCompleted;
    if (NumLoops < 0 || NumLoopsCompleted < NumLoops)
    {
        StartNewLoop();
    }
    else
    {
        if (OnCompleteCallback)
        {
            OnCompleteCallback();
        }
        if (bShouldAutoDestroy)
        {
            Destroy();
        }
        else
        {
            Pause();
        }
    }
}

void NsTweenInstance::StartNewLoop()
{
    DelayCounter = LoopDelaySecs;
    Counter = 0;
    bIsPlayingPingPong = false;
    if (DelayCounter > 0)
    {
        DelayState = ENsTweenDelayState::Loop;
    }
    else
    {
        if (OnLoopCallback)
        {
            OnLoopCallback();
        }
    }
}

void NsTweenInstance::StartPingPong()
{
    bIsPlayingPingPong = true;
    DelayCounter = PingPongDelaySecs;
    if (DelayCounter > 0)
    {
        DelayState = ENsTweenDelayState::PingPong;
    }
    else
    {
        if (OnPingPongCallback)
        {
            OnPingPongCallback();
        }
    }
}

void NsTweenInstanceFloat::Initialize(float InStart, float InEnd, TFunction<void(float)> InOnUpdate, float InDurationSecs, ENsTweenEase InEaseType)
{
    this->StartValue = InStart;
    this->EndValue = InEnd;
    this->OnUpdate = MoveTemp(InOnUpdate);
    this->InitializeSharedMembers(InDurationSecs, InEaseType);
}

void NsTweenInstanceFloat::ApplyEasing(float EasedPercent)
{
    OnUpdate(FMath::Lerp<float>(StartValue, EndValue, EasedPercent));
}

void NsTweenInstanceQuat::Initialize(FQuat InStart, FQuat InEnd, TFunction<void(FQuat)> InOnUpdate, float InDurationSecs, ENsTweenEase InEaseType)
{
    this->StartValue = InStart;
    this->EndValue = InEnd;
    this->OnUpdate = MoveTemp(InOnUpdate);
    this->InitializeSharedMembers(InDurationSecs, InEaseType);
}

void NsTweenInstanceQuat::ApplyEasing(float EasedPercent)
{
    OnUpdate(FQuat::Slerp(StartValue, EndValue, EasedPercent));
}

void NsTweenInstanceVector::Initialize(FVector InStart, FVector InEnd, TFunction<void(FVector)> InOnUpdate, float InDurationSecs, ENsTweenEase InEaseType)
{
    this->StartValue = InStart;
    this->EndValue = InEnd;
    this->OnUpdate = MoveTemp(InOnUpdate);
    this->InitializeSharedMembers(InDurationSecs, InEaseType);
}

void NsTweenInstanceVector::ApplyEasing(float EasedPercent)
{
    OnUpdate(FMath::Lerp<FVector>(StartValue, EndValue, EasedPercent));
}

void NsTweenInstanceVector2D::Initialize(FVector2D InStart, FVector2D InEnd, TFunction<void(FVector2D)> InOnUpdate, float InDurationSecs, ENsTweenEase InEaseType)
{
    this->StartValue = InStart;
    this->EndValue = InEnd;
    this->OnUpdate = MoveTemp(InOnUpdate);
    this->InitializeSharedMembers(InDurationSecs, InEaseType);
}

void NsTweenInstanceVector2D::ApplyEasing(float EasedPercent)
{
    OnUpdate(FMath::Lerp<FVector2D>(StartValue, EndValue, EasedPercent));
}

void NsTweenInstanceRotator::Initialize(FRotator InStart, FRotator InEnd, TFunction<void(FRotator)> InOnUpdate, float InDurationSecs, ENsTweenEase InEaseType)
{
    this->StartValue = InStart;
    this->EndValue = InEnd;
    this->OnUpdate = MoveTemp(InOnUpdate);
    this->InitializeSharedMembers(InDurationSecs, InEaseType);
}

void NsTweenInstanceRotator::ApplyEasing(float EasedPercent)
{
    FRotator Delta = EndValue - StartValue;

    // Ensure shortest path interpolation for each axis
    Delta.Pitch = FMath::UnwindDegrees(Delta.Pitch);
    Delta.Yaw   = FMath::UnwindDegrees(Delta.Yaw);
    Delta.Roll  = FMath::UnwindDegrees(Delta.Roll);

    const FRotator Interpolated = StartValue + (Delta * EasedPercent);
    OnUpdate(Interpolated.GetNormalized());
}

NsTweenInstance& NsTweenInstance::OnPingPong(TFunction<void()> Handler)
{
    OnPingPongCallback = MoveTemp(Handler);
    return *this;
}

NsTweenInstance& NsTweenInstance::OnLoop(TFunction<void()> Handler)
{
    OnLoopCallback = MoveTemp(Handler);
    return *this;
}

NsTweenInstance& NsTweenInstance::OnComplete(TFunction<void()> Handler)
{
    OnCompleteCallback = MoveTemp(Handler);
    return *this;
}
