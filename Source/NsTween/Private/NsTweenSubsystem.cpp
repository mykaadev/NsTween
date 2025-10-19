// Copyright (C) 2025 nulled.softworks. All rights reserved.

#include "NsTweenSubsystem.h"
#include "Containers/Ticker.h"
#include "Easing/NsTweenBezierEasing.h"
#include "Easing/NsTweenCurveAssetEasingAdapter.h"
#include "Easing/NsTweenPolynomialEasing.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "Engine/GameInstance.h"
#include "Interfaces/IEasingCurve.h"
#include "Interfaces/ITweenValue.h"
#include "Misc/CoreDelegates.h"
#include "NsTween.h"
#include "Utils/NsTweenLogging.h"
#include "Utils/NsTweenProfiling.h"

#if WITH_EDITOR
#include "Editor.h"
#endif

UNsTweenSubsystem::UNsTweenSubsystem()
{

}

void UNsTweenSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    NSTWEEN_SCOPE_CYCLE_COUNTER("NsTweenSubsystem::Initialize");
    Super::Initialize(Collection);
    NextTweenId.Set(0);

    // Core ticker
    FTickerDelegate Delegate = FTickerDelegate::CreateUObject(this, &UNsTweenSubsystem::Tick);
    TickerHandle = FTSTicker::GetCoreTicker().AddTicker(Delegate);

    // Runtime teardown
    FWorldDelegates::OnWorldBeginTearDown.AddUObject(this, &UNsTweenSubsystem::HandleWorldBeginTearDown);
    FWorldDelegates::OnWorldCleanup.AddUObject(this, &UNsTweenSubsystem::HandleWorldCleanup);
    FCoreDelegates::OnPreExit.AddUObject(this, &UNsTweenSubsystem::HandlePreExit);

#if WITH_EDITOR
    // PIE teardown (fires before worlds are fully gone)
    PrePIEEndedHandle = FEditorDelegates::PrePIEEnded.AddUObject(this, &UNsTweenSubsystem::HandlePrePIEEnded);
#endif
}

void UNsTweenSubsystem::Deinitialize()
{
    NSTWEEN_SCOPE_CYCLE_COUNTER("NsTweenSubsystem::Deinitialize");
    // Kill ticker first so no more ticks arrive
    if (TickerHandle.IsValid())
    {
        FTSTicker::GetCoreTicker().RemoveTicker(TickerHandle);
        TickerHandle.Reset();
    }

    // Unhook delegates
    FWorldDelegates::OnWorldBeginTearDown.RemoveAll(this);
    FWorldDelegates::OnWorldCleanup.RemoveAll(this);
    FCoreDelegates::OnPreExit.RemoveAll(this);
#if WITH_EDITOR
    if (PrePIEEndedHandle.IsValid())
    {
        FEditorDelegates::PrePIEEnded.Remove(PrePIEEndedHandle);
        PrePIEEndedHandle.Reset();
    }
#endif

    // Stop and clear
    StopAllTweens(/*bApplyFinalOnCancel*/ false);
    DrainCommandQueue();

    Super::Deinitialize();
}

bool UNsTweenSubsystem::ShouldTick() const
{
    NSTWEEN_SCOPE_CYCLE_COUNTER("NsTweenSubsystem::ShouldTick");
    if (IsEngineExitRequested())
    {
        return false;
    }

    if (!GEngine)
    {
        return false;
    }

    bool bHasGameWorld = false;

    for (const FWorldContext& Ctx : GEngine->GetWorldContexts())
    {
        if (const UWorld* const World = Ctx.World())
        {
            const EWorldType::Type WT = Ctx.WorldType;
            const bool bGameLike = (WT == EWorldType::PIE) || (WT == EWorldType::Game) || (WT == EWorldType::GamePreview);
            if (bGameLike && !World->bIsTearingDown)
            {
                bHasGameWorld = true;
                break;
            }
        }
    }

    if (!bHasGameWorld)
    {
        return false;
    }

    return true;
}

bool UNsTweenSubsystem::Tick(float DeltaTime)
{
    NSTWEEN_SCOPE_CYCLE_COUNTER("NsTweenSubsystem::Tick");
    // Don’t process anything during teardown/after PIE
    if (!ShouldTick())
    {
        return true;
    }

    // Process commands up front
    ProcessCommands();

    if (DeltaTime < KINDA_SMALL_NUMBER)
    {
        return true;
    }

    FWriteScopeLock WriteLock(PoolLock);
    for (int32 Index = TweenPool.Num() - 1; Index >= 0; --Index)
    {
        const TUniquePtr<FNsTween>& Instance = TweenPool[Index];
        if (!Instance)
        {
            continue;
        }

        // If tick returns false, it’s finished or invalid — remove it.
        if (!Instance->Tick(DeltaTime))
        {
            TweenPool.RemoveAtSwap(Index);
        }
    }

    return true;
}

void UNsTweenSubsystem::StopAllTweens(bool bApplyFinalOnCancel)
{
    NSTWEEN_SCOPE_CYCLE_COUNTER("NsTweenSubsystem::StopAllTweens");
    FWriteScopeLock WriteLock(PoolLock);

    for (int32 Index = TweenPool.Num() - 1; Index >= 0; --Index)
    {
        if (const TUniquePtr<FNsTween>& Instance = TweenPool[Index])
        {
            Instance->Cancel(bApplyFinalOnCancel);
        }
    }
    TweenPool.Reset();
}

UNsTweenSubsystem* UNsTweenSubsystem::GetSubsystem()
{
    NSTWEEN_SCOPE_CYCLE_COUNTER("NsTweenSubsystem::GetSubsystem");
    UNsTweenSubsystem* ToReturn = nullptr;

    if (GEngine != nullptr)
    {
        // Iterate through World Contexts until a Game World is found
        for (const FWorldContext& Context : GEngine->GetWorldContexts())
        {
            if (const UWorld* const World = Context.World())
            {
                if (World->IsGameWorld() || World->IsPlayInEditor())
                {
                    if (UNsTweenSubsystem* const Subsystem = UGameInstance::GetSubsystem<UNsTweenSubsystem>(World->GetGameInstance()))
                    {
                        ToReturn = Subsystem;
                        break;
                    }
                }
            }
        }
    }

    return ToReturn;
}

void UNsTweenSubsystem::DrainCommandQueue()
{
    NSTWEEN_SCOPE_CYCLE_COUNTER("NsTweenSubsystem::DrainCommandQueue");
    FNsTweenCommand Dummy;
    while (CommandQueue.Dequeue(Dummy)) {}
}

void UNsTweenSubsystem::HandleWorldBeginTearDown(UWorld* /*World*/)
{
    NSTWEEN_SCOPE_CYCLE_COUNTER("NsTweenSubsystem::HandleWorldBeginTearDown");
    StopAllTweens(/*bApplyFinalOnCancel*/ false);
    DrainCommandQueue();
}

void UNsTweenSubsystem::HandleWorldCleanup(UWorld* /*World*/, bool /*bSessionEnded*/, bool /*bCleanupResources*/)
{
    NSTWEEN_SCOPE_CYCLE_COUNTER("NsTweenSubsystem::HandleWorldCleanup");
    StopAllTweens(/*bApplyFinalOnCancel*/ false);
    DrainCommandQueue();
}

void UNsTweenSubsystem::HandlePreExit()
{
    NSTWEEN_SCOPE_CYCLE_COUNTER("NsTweenSubsystem::HandlePreExit");
    StopAllTweens(/*bApplyFinalOnCancel*/ false);
    DrainCommandQueue();
}

#if WITH_EDITOR
void UNsTweenSubsystem::HandlePrePIEEnded(const bool /*bIsSimulating*/)
{
    NSTWEEN_SCOPE_CYCLE_COUNTER("NsTweenSubsystem::HandlePrePIEEnded");
    StopAllTweens(/*bApplyFinalOnCancel*/ false);
    DrainCommandQueue();
}
#endif

FNsTweenHandle UNsTweenSubsystem::EnqueueSpawn(const FNsTweenSpec& Spec, const TSharedPtr<ITweenValue>& Strategy)
{
    NSTWEEN_SCOPE_CYCLE_COUNTER("NsTweenSubsystem::EnqueueSpawn");
    FNsTweenCommand Command;
    Command.Type = ENsTweenCommandType::Spawn;
    Command.Spec = Spec;
    Command.Strategy = Strategy;

    FNsTweenHandle Handle;
    Handle.Id.Value = static_cast<uint32>(NextTweenId.Increment());
    Command.Handle = Handle;

    CommandQueue.Enqueue(Command);
    return Handle;
}

void UNsTweenSubsystem::EnqueueCancel(const FNsTweenHandle& Handle, bool bApplyFinal)
{
    NSTWEEN_SCOPE_CYCLE_COUNTER("NsTweenSubsystem::EnqueueCancel");
    if (!Handle.IsValid()) return;

    FNsTweenCommand Command;
    Command.Type = ENsTweenCommandType::Cancel;
    Command.Handle = Handle;
    Command.bApplyFinalOnCancel = bApplyFinal;
    CommandQueue.Enqueue(Command);
}

void UNsTweenSubsystem::EnqueuePause(const FNsTweenHandle& Handle)
{
    NSTWEEN_SCOPE_CYCLE_COUNTER("NsTweenSubsystem::EnqueuePause");
    if (!Handle.IsValid()) return;

    FNsTweenCommand Command;
    Command.Type = ENsTweenCommandType::Pause;
    Command.Handle = Handle;
    CommandQueue.Enqueue(Command);
}

void UNsTweenSubsystem::EnqueueResume(const FNsTweenHandle& Handle)
{
    NSTWEEN_SCOPE_CYCLE_COUNTER("NsTweenSubsystem::EnqueueResume");
    if (!Handle.IsValid()) return;

    FNsTweenCommand Command;
    Command.Type = ENsTweenCommandType::Resume;
    Command.Handle = Handle;
    CommandQueue.Enqueue(Command);
}

bool UNsTweenSubsystem::IsActive(const FNsTweenHandle& Handle) const
{
    NSTWEEN_SCOPE_CYCLE_COUNTER("NsTweenSubsystem::IsActive");
    FReadScopeLock ReadLock(PoolLock);
    for (const TUniquePtr<FNsTween>& Instance : TweenPool)
    {
        if (Instance && Instance->IsActive() && Instance->GetHandle().Id.Value == Handle.Id.Value)
        {
            return true;
        }
    }
    return false;
}

void UNsTweenSubsystem::ProcessCommands()
{
    NSTWEEN_SCOPE_CYCLE_COUNTER("NsTweenSubsystem::ProcessCommands");
    FNsTweenCommand Command;
    while (CommandQueue.Dequeue(Command))
    {
        switch (Command.Type)
        {
            case ENsTweenCommandType::Spawn:  SpawnTween(Command);  break;
            case ENsTweenCommandType::Cancel: CancelTween(Command);  break;
            case ENsTweenCommandType::Pause:  PauseTween(Command);   break;
            case ENsTweenCommandType::Resume: ResumeTween(Command);  break;
            default: break;
        }
    }
}

void UNsTweenSubsystem::SpawnTween(FNsTweenCommand& Command)
{
    NSTWEEN_SCOPE_CYCLE_COUNTER("NsTweenSubsystem::SpawnTween");
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

    TUniquePtr<FNsTween> Instance = MakeUnique<FNsTween>(Command.Handle, MoveTemp(Command.Spec), Command.Strategy, Easing);

    FWriteScopeLock WriteLock(PoolLock);
    TweenPool.Add(MoveTemp(Instance));
}

void UNsTweenSubsystem::CancelTween(const FNsTweenCommand& Command)
{
    NSTWEEN_SCOPE_CYCLE_COUNTER("NsTweenSubsystem::CancelTween");
    FWriteScopeLock WriteLock(PoolLock);
    for (int32 Index = 0; Index < TweenPool.Num(); ++Index)
    {
        TUniquePtr<FNsTween>& Instance = TweenPool[Index];
        if (Instance && Instance->GetHandle().Id.Value == Command.Handle.Id.Value)
        {
            Instance->Cancel(Command.bApplyFinalOnCancel);
            TweenPool.RemoveAtSwap(Index);
            break;
        }
    }
}

void UNsTweenSubsystem::PauseTween(const FNsTweenCommand& Command)
{
    NSTWEEN_SCOPE_CYCLE_COUNTER("NsTweenSubsystem::PauseTween");
    FWriteScopeLock WriteLock(PoolLock);
    for (const TUniquePtr<FNsTween>& Instance : TweenPool)
    {
        if (Instance && Instance->GetHandle().Id.Value == Command.Handle.Id.Value)
        {
            Instance->SetPaused(true);
            break;
        }
    }
}

void UNsTweenSubsystem::ResumeTween(const FNsTweenCommand& Command)
{
    NSTWEEN_SCOPE_CYCLE_COUNTER("NsTweenSubsystem::ResumeTween");
    FWriteScopeLock WriteLock(PoolLock);
    for (const TUniquePtr<FNsTween>& Instance : TweenPool)
    {
        if (Instance && Instance->GetHandle().Id.Value == Command.Handle.Id.Value)
        {
            Instance->SetPaused(false);
            break;
        }
    }
}

TSharedPtr<IEasingCurve> UNsTweenSubsystem::CreateEasing(const FNsTweenSpec& Spec) const
{
    NSTWEEN_SCOPE_CYCLE_COUNTER("NsTweenSubsystem::CreateEasing");
    switch (Spec.EasingPreset)
    {
        case ENsTweenEase::CustomBezier: return MakeShared<FNsTweenBezierEasing>(Spec.BezierControlPoints);
        case ENsTweenEase::CurveAsset:   return MakeShared<FNsTweenCurveAssetEasingAdapter>(Spec.CurveAsset);
        default:                         return MakeShared<FNsTweenPolynomialEasing>(Spec.EasingPreset);
    }
}
