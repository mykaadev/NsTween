// Copyright (C) 2025 nulled.softworks. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "Containers/Queue.h"
#include "Containers/Ticker.h"
#include "HAL/ThreadSafeCounter.h"
#include "NsTweenTypeLibrary.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "NsTweenSubsystem.generated.h"

class ITweenValue;
class IEasingCurve;
struct FNsTweenSpec;
struct FNsTweenHandle;
struct FNsTweenId;
struct FNsTweenBezierDefinition;
class FNsTweenBezierEasing;
class FNsTweenPolynomialEasing;
class FNsTweenCurveAssetEasingAdapter;
struct FNsTween;

UCLASS()
class UNsTweenSubsystem : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UNsTweenSubsystem();

    //~ Begin UEngineSubsystem Interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;
    //~ End UEngineSubsystem Interface

    /** Queue API */
    FNsTweenHandle EnqueueSpawn(const FNsTweenSpec& Spec, const TSharedPtr<ITweenValue>& Strategy);
    void EnqueueCancel(const FNsTweenHandle& Handle, bool bApplyFinal);
    void EnqueuePause(const FNsTweenHandle& Handle);
    void EnqueueResume(const FNsTweenHandle& Handle);
    bool IsActive(const FNsTweenHandle& Handle) const;

    /** Stop everything now (used by teardown/PIE end) */
    void StopAllTweens(bool bApplyFinalOnCancel);

    /** Get Subsystem */
    static UNsTweenSubsystem* GetSubsystem();

private:

    /** Ticker callback */
    bool Tick(float DeltaTime);

    /** Command pump */
    void ProcessCommands();
    void SpawnTween(FNsTweenCommand& Command);
    void CancelTween(const FNsTweenCommand& Command);
    void PauseTween(const FNsTweenCommand& Command);
    void ResumeTween(const FNsTweenCommand& Command);

    /** Easing factory */
    TSharedPtr<IEasingCurve> CreateEasing(const FNsTweenSpec& Spec) const;

    /** Are we allowed to tick right now? */
    bool ShouldTick() const;

    /** Clear any queued-but-unprocessed commands */
    void DrainCommandQueue();

    /** World / editor teardown hooks */
    void HandleWorldBeginTearDown(UWorld* World);
    void HandleWorldCleanup(UWorld* World, bool bSessionEnded, bool bCleanupResources);
    void HandlePreExit();

#if WITH_EDITOR
    void HandlePrePIEEnded(const bool bIsSimulating);
#endif

// Components
private:

    /** Live tweens */
    TArray<TUniquePtr<FNsTween>> TweenPool;

    /** Command queue (game thread) */
    TQueue<FNsTweenCommand, EQueueMode::Mpsc> CommandQueue;

    /** Pool lock */
    mutable FRWLock PoolLock;

    /** Main ticker handle */
    FTSTicker::FDelegateHandle TickerHandle;

    /** ID allocator */
    FThreadSafeCounter NextTweenId;

#if WITH_EDITOR
    /** Editor delegate handles (optional to store if you want to remove individually) */
    FDelegateHandle PrePIEEndedHandle;
#endif
};

