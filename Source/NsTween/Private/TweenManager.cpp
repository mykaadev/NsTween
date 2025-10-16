// Copyright (C) 2024 mykaa. All rights reserved.

#include "TweenManager.h"
#include "NsTweenModule.h"
#include "TweenInstance.h"
#include "Easing/BezierEasing.h"
#include "Easing/PolynomialEasing.h"
#include "Easing/CurveAssetEasingAdapter.h"
#include "Interfaces/ITweenValue.h"
#include "Interfaces/IEasingCurve.h"
#include "Utils/NsTweenLogging.h"

#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Curves/CurveFloat.h"
#include "Misc/ScopeLock.h"
#include "HAL/PlatformTime.h"
#include "Math/UnrealMathUtility.h"
#include "HAL/PlatformProcess.h"
#include "HAL/RWLock.h"
#include "Containers/Ticker.h"

DEFINE_LOG_CATEGORY(LogNsTween);

namespace
{
    constexpr float KINDA_SMALL = 1.e-6f;
}

UNsTweenManagerSubsystem::UNsTweenManagerSubsystem()
{
}

void UNsTweenManagerSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    NextTweenId.Set(0);

    FTickerDelegate Delegate = FTickerDelegate::CreateUObject(this, &UNsTweenManagerSubsystem::Tick);
    TickerHandle = FTSTicker::GetCoreTicker().AddTicker(Delegate);
}

void UNsTweenManagerSubsystem::Deinitialize()
{
    if (TickerHandle.IsValid())
    {
        FTSTicker::GetCoreTicker().RemoveTicker(TickerHandle);
        TickerHandle.Reset();
    }

    FWriteScopeLock WriteLock(PoolLock);
    TweenPool.Empty();
    Super::Deinitialize();
}

FNovaTweenHandle UNsTweenManagerSubsystem::EnqueueSpawn(const FNovaTweenSpec& Spec, const TSharedPtr<ITweenValue>& Strategy)
{
    FNovaTweenCommand Command;
    Command.Type = ENsTweenCommandType::Spawn;
    Command.Spec = Spec;
    Command.Strategy = Strategy;

    FNovaTweenHandle Handle;
    Handle.Id.Value = static_cast<uint32>(NextTweenId.Increment());
    Command.Handle = Handle;

    CommandQueue.Enqueue(Command);
    return Handle;
}

void UNsTweenManagerSubsystem::EnqueueCancel(const FNovaTweenHandle& Handle, bool bApplyFinal)
{
    FNovaTweenCommand Command;
    Command.Type = ENsTweenCommandType::Cancel;
    Command.Handle = Handle;
    Command.bApplyFinalOnCancel = bApplyFinal;
    if (Handle.IsValid())
    {
        CommandQueue.Enqueue(Command);
    }
}

void UNsTweenManagerSubsystem::EnqueuePause(const FNovaTweenHandle& Handle)
{
    FNovaTweenCommand Command;
    Command.Type = ENsTweenCommandType::Pause;
    Command.Handle = Handle;
    if (Handle.IsValid())
    {
        CommandQueue.Enqueue(Command);
    }
}

void UNsTweenManagerSubsystem::EnqueueResume(const FNovaTweenHandle& Handle)
{
    FNovaTweenCommand Command;
    Command.Type = ENsTweenCommandType::Resume;
    Command.Handle = Handle;
    if (Handle.IsValid())
    {
        CommandQueue.Enqueue(Command);
    }
}

bool UNsTweenManagerSubsystem::IsActive(const FNovaTweenHandle& Handle) const
{
    FReadScopeLock ReadLock(PoolLock);
    for (const TUniquePtr<FTweenInstance>& Instance : TweenPool)
    {
        if (Instance && Instance->IsActive() && Instance->GetHandle().Id.Value == Handle.Id.Value)
        {
            return true;
        }
    }
    return false;
}

bool UNsTweenManagerSubsystem::Tick(float DeltaTime)
{
    ProcessCommands();

    if (DeltaTime < KINDA_SMALL)
    {
        return true;
    }

    FWriteScopeLock WriteLock(PoolLock);
    for (int32 Index = TweenPool.Num() - 1; Index >= 0; --Index)
    {
        TUniquePtr<FTweenInstance>& Instance = TweenPool[Index];
        if (!Instance)
        {
            continue;
        }

        if (!Instance->Tick(DeltaTime))
        {
            TweenPool.RemoveAtSwap(Index);
        }
    }

    return true;
}

void UNsTweenManagerSubsystem::ProcessCommands()
{
    FNovaTweenCommand Command;
    while (CommandQueue.Dequeue(Command))
    {
        switch (Command.Type)
        {
        case ENsTweenCommandType::Spawn:
            SpawnTween(Command);
            break;
        case ENsTweenCommandType::Cancel:
            CancelTween(Command);
            break;
        case ENsTweenCommandType::Pause:
            PauseTween(Command);
            break;
        case ENsTweenCommandType::Resume:
            ResumeTween(Command);
            break;
        default:
            break;
        }
    }
}

void UNsTweenManagerSubsystem::SpawnTween(const FNovaTweenCommand& Command)
{
    if (!Command.Strategy.IsValid())
    {
        UE_LOG(LogNsTween, Warning, TEXT("Cannot spawn tween without strategy."));
        return;
    }

    TSharedPtr<IEasingCurve> Easing = CreateEasing(Command.Spec);
    if (!Easing.IsValid())
    {
        UE_LOG(LogNsTween, Warning, TEXT("Failed to create easing curve."));
        return;
    }

    TUniquePtr<FTweenInstance> Instance = MakeUnique<FTweenInstance>(Command.Handle, MoveTemp(Command.Spec), Command.Strategy, Easing);

    FWriteScopeLock WriteLock(PoolLock);
    TweenPool.Add(MoveTemp(Instance));
}

void UNsTweenManagerSubsystem::CancelTween(const FNovaTweenCommand& Command)
{
    FWriteScopeLock WriteLock(PoolLock);
    for (int32 Index = 0; Index < TweenPool.Num(); ++Index)
    {
        TUniquePtr<FTweenInstance>& Instance = TweenPool[Index];
        if (Instance && Instance->GetHandle().Id.Value == Command.Handle.Id.Value)
        {
            Instance->Cancel(Command.bApplyFinalOnCancel);
            TweenPool.RemoveAtSwap(Index);
            break;
        }
    }
}

void UNsTweenManagerSubsystem::PauseTween(const FNovaTweenCommand& Command)
{
    FWriteScopeLock WriteLock(PoolLock);
    for (const TUniquePtr<FTweenInstance>& Instance : TweenPool)
    {
        if (Instance && Instance->GetHandle().Id.Value == Command.Handle.Id.Value)
        {
            Instance->SetPaused(true);
            break;
        }
    }
}

void UNsTweenManagerSubsystem::ResumeTween(const FNovaTweenCommand& Command)
{
    FWriteScopeLock WriteLock(PoolLock);
    for (const TUniquePtr<FTweenInstance>& Instance : TweenPool)
    {
        if (Instance && Instance->GetHandle().Id.Value == Command.Handle.Id.Value)
        {
            Instance->SetPaused(false);
            break;
        }
    }
}

TSharedPtr<IEasingCurve> UNsTweenManagerSubsystem::CreateEasing(const FNovaTweenSpec& Spec) const
{
    switch (Spec.EasingPreset)
    {
    case ENovaEasingPreset::CustomBezier:
        return MakeShared<FBezierEasing>(Spec.BezierControlPoints);
    case ENovaEasingPreset::CurveAsset:
        return MakeShared<FCurveAssetEasingAdapter>(Spec.CurveAsset);
    default:
        return MakeShared<FPolynomialEasing>(Spec.EasingPreset);
    }
}

