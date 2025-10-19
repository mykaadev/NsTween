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

#if WITH_AUTOMATION_TESTS
struct FNsTweenSubsystemTestAccessor
{
    static bool DequeueCommand(UNsTweenSubsystem& Subsystem, FNsTweenCommand& OutCommand)
    {
        return Subsystem.CommandQueue.Dequeue(OutCommand);
    }
};
#endif

#if WITH_EDITOR
#include "Editor.h"
#endif

UNsTweenSubsystem::UNsTweenSubsystem()
{

}

void UNsTweenSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
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
    // Don’t process anything during teardown/after PIE
    if (!ShouldTick())
    {
        return true;
    }

    // Process commands up front so queue mutations happen before we start ticking tweens.
    ProcessCommands();

    if (DeltaTime < KINDA_SMALL_NUMBER)
    {
        return true;
    }

    struct FTickCandidate
    {
        FNsTween* Instance = nullptr;
        FNsTweenHandle Handle;
    };

    TArray<FTickCandidate, TInlineAllocator<32>> Candidates;
    {
        // Phase 1: Take a read lock and gather the tweens that are still active.
        FReadScopeLock ReadLock(PoolLock);
        Candidates.Reserve(TweenPool.Num());

        for (const TUniquePtr<FNsTween>& Instance : TweenPool)
        {
            if (Instance && Instance->IsActive())
            {
                FTickCandidate& Candidate = Candidates.Emplace_GetRef();
                Candidate.Instance = Instance.Get();
                Candidate.Handle = Instance->GetHandle();
            }
        }
    }

    // Phase 2: Tick outside the lock so Update/OnComplete callbacks can execute without blocking
    // other readers/writers attempting to enqueue commands.
    TArray<FNsTweenHandle, TInlineAllocator<32>> CompletedHandles;
    CompletedHandles.Reserve(Candidates.Num());

    for (const FTickCandidate& Candidate : Candidates)
    {
        if (!Candidate.Instance)
        {
            continue;
        }

        // If tick returns false, it’s finished or invalid — remember the handle for removal.
        if (!Candidate.Instance->Tick(DeltaTime))
        {
            CompletedHandles.Add(Candidate.Handle);
        }
    }

    if (CompletedHandles.Num() > 0)
    {
        // Phase 3: Acquire a write lock only long enough to prune finished tweens. We validate
        // each handle because the tween may have already been removed by a command processed
        // earlier on this frame (e.g. cancel).
        FWriteScopeLock WriteLock(PoolLock);

        for (const FNsTweenHandle& Handle : CompletedHandles)
        {
            for (int32 Index = TweenPool.Num() - 1; Index >= 0; --Index)
            {
                const TUniquePtr<FNsTween>& Instance = TweenPool[Index];
                if (Instance && Instance->GetHandle().Id.Value == Handle.Id.Value)
                {
                    TweenPool.RemoveAtSwap(Index);
                    break;
                }
            }
        }
    }

    return true;
}

void UNsTweenSubsystem::StopAllTweens(bool bApplyFinalOnCancel)
{
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
    FNsTweenCommand Dummy;
    while (CommandQueue.Dequeue(Dummy)) {}
}

void UNsTweenSubsystem::HandleWorldBeginTearDown(UWorld* /*World*/)
{
    StopAllTweens(/*bApplyFinalOnCancel*/ false);
    DrainCommandQueue();
}

void UNsTweenSubsystem::HandleWorldCleanup(UWorld* /*World*/, bool /*bSessionEnded*/, bool /*bCleanupResources*/)
{
    StopAllTweens(/*bApplyFinalOnCancel*/ false);
    DrainCommandQueue();
}

void UNsTweenSubsystem::HandlePreExit()
{
    StopAllTweens(/*bApplyFinalOnCancel*/ false);
    DrainCommandQueue();
}

#if WITH_EDITOR
void UNsTweenSubsystem::HandlePrePIEEnded(const bool /*bIsSimulating*/)
{
    StopAllTweens(/*bApplyFinalOnCancel*/ false);
    DrainCommandQueue();
}
#endif

FNsTweenHandle UNsTweenSubsystem::EnqueueSpawn(const FNsTweenSpec& Spec, const TSharedPtr<ITweenValue>& Strategy)
{
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
    if (!Handle.IsValid()) return;

    FNsTweenCommand Command;
    Command.Type = ENsTweenCommandType::Cancel;
    Command.Handle = Handle;
    Command.bApplyFinalOnCancel = bApplyFinal;
    CommandQueue.Enqueue(Command);
}

void UNsTweenSubsystem::EnqueuePause(const FNsTweenHandle& Handle)
{
    if (!Handle.IsValid()) return;

    FNsTweenCommand Command;
    Command.Type = ENsTweenCommandType::Pause;
    Command.Handle = Handle;
    CommandQueue.Enqueue(Command);
}

void UNsTweenSubsystem::EnqueueResume(const FNsTweenHandle& Handle)
{
    if (!Handle.IsValid()) return;

    FNsTweenCommand Command;
    Command.Type = ENsTweenCommandType::Resume;
    Command.Handle = Handle;
    CommandQueue.Enqueue(Command);
}

bool UNsTweenSubsystem::IsActive(const FNsTweenHandle& Handle) const
{
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
    if (!Command.Strategy.IsValid())
    {
        UE_LOG(LogNsTween, Warning, TEXT("Cannot spawn tween without strategy."));
        return;
    }

    if (!ShouldTick())
    {
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
    switch (Spec.EasingPreset)
    {
        case ENsTweenEase::CustomBezier: return MakeShared<FNsTweenBezierEasing>(Spec.BezierControlPoints);
        case ENsTweenEase::CurveAsset:   return MakeShared<FNsTweenCurveAssetEasingAdapter>(Spec.CurveAsset);
        default:                         return MakeShared<FNsTweenPolynomialEasing>(Spec.EasingPreset);
    }
}
