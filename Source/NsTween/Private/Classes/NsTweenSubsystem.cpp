// Copyright (C) 2025 nulled.softworks. All rights reserved.

#include "Classes/NsTweenSubsystem.h"
#include "Classes/NsTweenCore.h"
#include "Engine/World.h"

void UNsTweenSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    LastTickedFrame = GFrameCounter;
    NsTweenCore::Initialize();
    if(GetWorld() != nullptr)
    {
        LastRealTimeSeconds = GetWorld()->RealTimeSeconds;
    }

#if WITH_EDITOR
    NsTweenCore::ClearActiveTweens();
#endif
}

void UNsTweenSubsystem::Deinitialize()
{
    Super::Deinitialize();
#if WITH_EDITOR
    NsTweenCore::CheckTweenCapacity();
    NsTweenCore::ClearActiveTweens();
#endif
    NsTweenCore::Deinitialize();
}

void UNsTweenSubsystem::Tick(float DeltaTime)
{
    if (LastTickedFrame < GFrameCounter)
    {
        LastTickedFrame = GFrameCounter;

        if (GetWorld() != nullptr)
        {
            NsTweenCore::Update(GetWorld()->DeltaRealTimeSeconds, GetWorld()->DeltaTimeSeconds, GetWorld()->IsPaused());
        }
    }
}

ETickableTickType UNsTweenSubsystem::GetTickableTickType() const
{
    return ETickableTickType::Always;
}

TStatId UNsTweenSubsystem::GetStatId() const
{
    RETURN_QUICK_DECLARE_CYCLE_STAT(NsTween, STATGROUP_Tickables);
}

bool UNsTweenSubsystem::IsTickableWhenPaused() const
{
    return true;
}

bool UNsTweenSubsystem::IsTickableInEditor() const
{
    return false;
}
