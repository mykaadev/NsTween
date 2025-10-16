// Copyright (C) 2024 mykaa. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/EngineSubsystem.h"
#include "Containers/Queue.h"
#include "Containers/Ticker.h"
#include "HAL/RWLock.h"
#include "HAL/ThreadSafeCounter.h"
#include "TweenHandle.h"
#include "TweenSpec.h"

class ITweenValue;
class IEasingCurve;
struct FNovaTweenSpec;
struct FNovaTweenHandle;
struct FNovaTweenId;
struct FNovaBezierDefinition;
class FBezierEasing;
class FPolynomialEasing;
class FCurveAssetEasingAdapter;
struct FTweenInstance;

enum class ENsTweenCommandType : uint8
{
    Spawn,
    Cancel,
    Pause,
    Resume
};

struct FNovaTweenCommand
{
    ENsTweenCommandType Type = ENsTweenCommandType::Spawn;
    FNovaTweenHandle Handle;
    FNovaTweenSpec Spec;
    TSharedPtr<ITweenValue> Strategy;
    bool bApplyFinalOnCancel = true;
};

UCLASS()
class UNsTweenManagerSubsystem : public UEngineSubsystem
{
    GENERATED_BODY()

public:
    UNsTweenManagerSubsystem();

    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    FNovaTweenHandle EnqueueSpawn(const FNovaTweenSpec& Spec, const TSharedPtr<ITweenValue>& Strategy);
    void EnqueueCancel(const FNovaTweenHandle& Handle, bool bApplyFinal);
    void EnqueuePause(const FNovaTweenHandle& Handle);
    void EnqueueResume(const FNovaTweenHandle& Handle);
    bool IsActive(const FNovaTweenHandle& Handle) const;

private:
    bool Tick(float DeltaTime);
    void ProcessCommands();
    void SpawnTween(const FNovaTweenCommand& Command);
    void CancelTween(const FNovaTweenCommand& Command);
    void PauseTween(const FNovaTweenCommand& Command);
    void ResumeTween(const FNovaTweenCommand& Command);

    TSharedPtr<IEasingCurve> CreateEasing(const FNovaTweenSpec& Spec) const;

private:
    TArray<TUniquePtr<FTweenInstance>> TweenPool;
    TQueue<FNovaTweenCommand, EQueueMode::Mpsc> CommandQueue;
    mutable FRWLock PoolLock;
    FTSTicker::FDelegateHandle TickerHandle;
    FThreadSafeCounter NextTweenId;
};

