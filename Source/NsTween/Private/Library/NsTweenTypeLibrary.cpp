// Copyright (C) 2025 mykaadev. All rights reserved.

#include "Library/NsTweenTypeLibrary.h"

void FNsTweenInstance::InitializeSharedMembers(const float InDurationSecs, const ENsTweenEase InEaseType)
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

void FNsTweenInstance::Start()
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

void FNsTweenInstance::Restart()
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

void FNsTweenInstance::Destroy()
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

void FNsTweenInstance::Pause()
{
    bIsPaused = true;
}

void FNsTweenInstance::Unpause()
{
    bIsPaused = false;
}

void FNsTweenInstance::Update(float UnscaledDeltaSeconds, float DilatedDeltaSeconds, bool bIsGamePaused)
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

void FNsTweenInstance::CompleteLoop()
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

void FNsTweenInstance::StartNewLoop()
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

void FNsTweenInstance::StartPingPong()
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

void FNsTweenInstanceFloat::Initialize(float InStart, float InEnd, TFunction<void(float)> InOnUpdate, float InDurationSecs, ENsTweenEase InEaseType)
{
    this->StartValue = InStart;
    this->EndValue = InEnd;
    this->OnUpdate = MoveTemp(InOnUpdate);
    this->InitializeSharedMembers(InDurationSecs, InEaseType);
}

void FNsTweenInstanceFloat::ApplyEasing(float EasedPercent)
{
    OnUpdate(FMath::Lerp<float>(StartValue, EndValue, EasedPercent));
}

void FNsTweenInstanceQuat::Initialize(FQuat InStart, FQuat InEnd, TFunction<void(FQuat)> InOnUpdate, float InDurationSecs, ENsTweenEase InEaseType)
{
    this->StartValue = InStart;
    this->EndValue = InEnd;
    this->OnUpdate = MoveTemp(InOnUpdate);
    this->InitializeSharedMembers(InDurationSecs, InEaseType);
}

void FNsTweenInstanceQuat::ApplyEasing(float EasedPercent)
{
    OnUpdate(FQuat::Slerp(StartValue, EndValue, EasedPercent));
}

void FNsTweenInstanceVector::Initialize(FVector InStart, FVector InEnd, TFunction<void(FVector)> InOnUpdate, float InDurationSecs, ENsTweenEase InEaseType)
{
    this->StartValue = InStart;
    this->EndValue = InEnd;
    this->OnUpdate = MoveTemp(InOnUpdate);
    this->InitializeSharedMembers(InDurationSecs, InEaseType);
}

void FNsTweenInstanceVector::ApplyEasing(float EasedPercent)
{
    OnUpdate(FMath::Lerp<FVector>(StartValue, EndValue, EasedPercent));
}

void FNsTweenInstanceVector2D::Initialize(FVector2D InStart, FVector2D InEnd, TFunction<void(FVector2D)> InOnUpdate, float InDurationSecs, ENsTweenEase InEaseType)
{
    this->StartValue = InStart;
    this->EndValue = InEnd;
    this->OnUpdate = MoveTemp(InOnUpdate);
    this->InitializeSharedMembers(InDurationSecs, InEaseType);
}

void FNsTweenInstanceVector2D::ApplyEasing(float EasedPercent)
{
    OnUpdate(FMath::Lerp<FVector2D>(StartValue, EndValue, EasedPercent));
}

void FNsTweenInstanceRotator::Initialize(FRotator InStart, FRotator InEnd, TFunction<void(FRotator)> InOnUpdate, float InDurationSecs, ENsTweenEase InEaseType)
{
    this->StartValue = InStart;
    this->EndValue = InEnd;
    this->OnUpdate = MoveTemp(InOnUpdate);
    this->InitializeSharedMembers(InDurationSecs, InEaseType);
}

void FNsTweenInstanceRotator::ApplyEasing(float EasedPercent)
{
    FRotator Delta = EndValue - StartValue;

    // Ensure shortest path interpolation for each axis
    Delta.Pitch = FMath::UnwindDegrees(Delta.Pitch);
    Delta.Yaw   = FMath::UnwindDegrees(Delta.Yaw);
    Delta.Roll  = FMath::UnwindDegrees(Delta.Roll);

    const FRotator Interpolated = StartValue + (Delta * EasedPercent);
    OnUpdate(Interpolated.GetNormalized());
}

FNsTweenInstance& FNsTweenInstance::OnPingPong(TFunction<void()> Handler)
{
    OnPingPongCallback = MoveTemp(Handler);
    return *this;
}

FNsTweenInstance& FNsTweenInstance::OnLoop(TFunction<void()> Handler)
{
    OnLoopCallback = MoveTemp(Handler);
    return *this;
}

FNsTweenInstance& FNsTweenInstance::OnComplete(TFunction<void()> Handler)
{
    OnCompleteCallback = MoveTemp(Handler);
    return *this;
}

FNsTweenInstance& FNsTweenInstance::SetPingPong(bool bPingPong)
{
    bShouldPingPong = bPingPong;
    return *this;
}

bool FNsTweenInstance::GetPingPong() const
{
    return bShouldPingPong;
}

FNsTweenInstance& FNsTweenInstance::SetLoops(int InLoops)
{
    NumLoops = InLoops;
    return *this;
}

int FNsTweenInstance::GetLoops() const
{
    return NumLoops;
}

FNsTweenInstance& FNsTweenInstance::SetDelay(float InDelay)
{
    DelaySecs = InDelay;
    return *this;
}

float FNsTweenInstance::GetDelay() const
{
    return DelaySecs;
}

FNsTweenInstance& FNsTweenInstance::SetLoopDelay(float InLoopDelay)
{
    LoopDelaySecs = InLoopDelay;
    return *this;
}

float FNsTweenInstance::GetLoopDelay() const
{
    return LoopDelaySecs;
}

FNsTweenInstance& FNsTweenInstance::SetPingPongDelay(float InPingPongDelay)
{
    PingPongDelaySecs = InPingPongDelay;
    return *this;
}

float FNsTweenInstance::GetPingPongDelay() const
{
    return PingPongDelaySecs;
}

FNsTweenInstance& FNsTweenInstance::SetCanTickDuringPause(bool bInCanTickDuringPause)
{
    bCanTickDuringPause = bInCanTickDuringPause;
    return *this;
}

bool FNsTweenInstance::GetCanTickDuringPause() const
{
    return bCanTickDuringPause;
}

FNsTweenInstance& FNsTweenInstance::SetUseGlobalTimeDilation(bool bInUseGlobalTimeDilation)
{
    bUseGlobalTimeDilation = bInUseGlobalTimeDilation;
    return *this;
}

bool FNsTweenInstance::GetUseGlobalTimeDilation() const
{
    return bUseGlobalTimeDilation;
}

FNsTweenInstance& FNsTweenInstance::SetAutoDestroy(bool bInShouldAutoDestroy)
{
    bShouldAutoDestroy = bInShouldAutoDestroy;
    return *this;
}

bool FNsTweenInstance::GetAutoDestroy() const
{
    return bShouldAutoDestroy;
}

FNsTweenInstance& FNsTweenInstance::SetTimeMultiplier(float InMultiplier)
{
    TimeMultiplier = InMultiplier;
    return *this;
}

float FNsTweenInstance::GetTimeMultiplier() const
{
    return TimeMultiplier;
}

FNsTweenInstance& FNsTweenInstance::SetEaseParam1(float InEaseParam1)
{
    EaseParam1 = InEaseParam1;
    return *this;
}

float FNsTweenInstance::GetEaseParam1() const
{
    return EaseParam1;
}

FNsTweenInstance& FNsTweenInstance::SetEaseParam2(float InEaseParam2)
{
    EaseParam2 = InEaseParam2;
    return *this;
}

float FNsTweenInstance::GetEaseParam2() const
{
    return EaseParam2;
}
