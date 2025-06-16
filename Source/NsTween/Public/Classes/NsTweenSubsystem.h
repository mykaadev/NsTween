// Copyright (C) 2024 mykaa. All rights reserved.

#pragma once

#include "Subsystems/GameInstanceSubsystem.h"
#include "Tickable.h"
#include "NsTweenSubsystem.generated.h"

/**
 * Tween Subsystem
 */
UCLASS()
class NSTWEEN_API UNsTweenSubsystem : public UGameInstanceSubsystem, public FTickableGameObject
{
    GENERATED_BODY()

// Functions
public:

    //~ Begin UGameInstanceSubsystem Interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;
    //~ End UGameInstanceSubsystem Interface

    //~ Begin FTickableGameObject Interface
    virtual void Tick(float DeltaTime) override;
    virtual ETickableTickType GetTickableTickType() const override;
    virtual TStatId GetStatId() const override;
    virtual bool IsTickableWhenPaused() const override;
    virtual bool IsTickableInEditor() const override;
    //~ End FTickableGameObject Interface


// Variables
private:

    /** Last Ticked Frame */
    UPROPERTY()
    uint64 LastTickedFrame;

    /** Last Real time Seconds */
    UPROPERTY()
    float LastRealTimeSeconds;
};
