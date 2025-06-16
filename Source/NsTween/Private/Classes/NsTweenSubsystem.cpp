// Copyright (C) 2024 mykaa. All rights reserved.

#include "Classes/NsTweenSubsystem.h"
#include "Classes/NsTweenCore.h"

void UNsTweenSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    LastTickedFrame = GFrameCounter;
    NsTweenCore::Initialize();
#if ENGINE_MAJOR_VERSION < 5
	if(GetWorld() != nullptr)
	{
		LastRealTimeSeconds = GetWorld()->RealTimeSeconds;
	}
#endif

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
}

void UNsTweenSubsystem::Tick(float DeltaTime)
{
    if (LastTickedFrame < GFrameCounter)
    {
        LastTickedFrame = GFrameCounter;

        if (GetWorld() != nullptr)
        {
#if ENGINE_MAJOR_VERSION < 5
			float DeltaRealTimeSeconds = GetWorld()->RealTimeSeconds - LastRealTimeSeconds;
			NsTweenCore::Update(DeltaRealTimeSeconds, GetWorld()->DeltaTimeSeconds, GetWorld()->IsPaused());
			LastRealTimeSeconds = GetWorld()->RealTimeSeconds;
#else
            NsTweenCore::Update(GetWorld()->DeltaRealTimeSeconds, GetWorld()->DeltaTimeSeconds, GetWorld()->IsPaused());
#endif
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
