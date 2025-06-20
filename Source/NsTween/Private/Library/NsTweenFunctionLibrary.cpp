// Copyright (C) 2025 mykaadev. All rights reserved.

#include "Library/NsTweenFunctionLibrary.h"
#include "Classes/NsTweenCore.h"
#include "Engine/World.h"
#include "Engine/Engine.h"

float UNsTweenFunctionLibrary::Ease(float t, ENsTweenEase EaseType)

{
    return NsTweenEasing::Ease(t, EaseType);
}

float UNsTweenFunctionLibrary::EaseWithParams(float t, ENsTweenEase EaseType, float Param1, float Param2)
{
    return NsTweenEasing::EaseWithParams(t, EaseType, Param1, Param2);
}

void UNsTweenFunctionLibrary::EnsureTweenCapacity(int NumFloatTweens, int NumVectorTweens, int NumVector2DTweens, int NumQuatTweens)
{
    NsTweenCore::EnsureCapacity(NumFloatTweens, NumVectorTweens, NumVector2DTweens, NumQuatTweens);
}

void UNsTweenFunctionLibrary::LogActiveTweens()
{
    NsTweenCore::LogActiveTweens();
}

void UNsTweenFunctionLibrary::DrawActiveTweens(UObject* WorldContextObject)
{
    if (!WorldContextObject)
    {
        return;
    }
    UWorld* World = GEngine ? GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::ReturnNull) : nullptr;
    NsTweenCore::DrawActiveTweens(World);
}
