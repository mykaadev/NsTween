// Copyright (C) 2025 mykaadev. All rights reserved.

#include "Classes/NsTweenCore.h"
#include "Classes/NsTweenManager.h"
#include "Classes/NsTweenSettings.h"

DEFINE_LOG_CATEGORY(LogNsTween)

NsTweenManager<NsTweenInstanceFloat>* NsTweenCore::FloatTweenManager = nullptr;
NsTweenManager<NsTweenInstanceVector>* NsTweenCore::VectorTweenManager = nullptr;
NsTweenManager<NsTweenInstanceVector2D>* NsTweenCore::Vector2DTweenManager = nullptr;
NsTweenManager<NsTweenInstanceQuat>* NsTweenCore::QuatTweenManager = nullptr;

void NsTweenCore::Initialize()
{
    if (const UNsTweenSettings* const TweenSettings = UNsTweenSettings::GetSettings())
    {
        FloatTweenManager = new NsTweenManager<NsTweenInstanceFloat>(TweenSettings->FloatTweenCapacity);
        VectorTweenManager = new NsTweenManager<NsTweenInstanceVector>(TweenSettings->VectorTweenCapacity);
        Vector2DTweenManager = new NsTweenManager<NsTweenInstanceVector2D>(TweenSettings->Vector2DTweenCapacity);
        QuatTweenManager = new NsTweenManager<NsTweenInstanceQuat>(TweenSettings->QuatTweenCapacity);
    }
}

void NsTweenCore::Deinitialize()
{
    delete FloatTweenManager;
    delete VectorTweenManager;
    delete Vector2DTweenManager;
    delete QuatTweenManager;

    FloatTweenManager = nullptr;
    VectorTweenManager = nullptr;
    Vector2DTweenManager = nullptr;
    QuatTweenManager = nullptr;
}

void NsTweenCore::EnsureCapacity(const int32 NumFloatTweens, const int32 NumVectorTweens, const int32 NumVector2DTweens, const int32 NumQuatTweens)
{
    EnsureManagerCapacity(FloatTweenManager, NumFloatTweens);
    EnsureManagerCapacity(VectorTweenManager, NumVectorTweens);
    EnsureManagerCapacity(Vector2DTweenManager, NumVector2DTweens);
    EnsureManagerCapacity(QuatTweenManager, NumQuatTweens);
}

void NsTweenCore::EnsureCapacity(const int NumTweens)
{
    EnsureCapacity(NumTweens, NumTweens, NumTweens, NumTweens);
}

void NsTweenCore::Update(const float UnscaledDeltaSeconds, const float DilatedDeltaSeconds, const bool bIsGamePaused)
{
    ForEachManager([&](auto* Manager)
    {
        Manager->Update(UnscaledDeltaSeconds, DilatedDeltaSeconds, bIsGamePaused);
    });
}

void NsTweenCore::ClearActiveTweens()
{
    ForEachManager([](auto* Manager)
    {
        Manager->ClearActiveTweens();
    });
}

int NsTweenCore::CheckTweenCapacity()
{
    if (const UNsTweenSettings* const TweenSettings = UNsTweenSettings::GetSettings())
    {
        CheckManagerCapacity(FloatTweenManager, TweenSettings->FloatTweenCapacity, TEXT("Float"));
        CheckManagerCapacity(VectorTweenManager, TweenSettings->VectorTweenCapacity, TEXT("Vector (3D vector)"));
        CheckManagerCapacity(Vector2DTweenManager, TweenSettings->Vector2DTweenCapacity, TEXT("Vector2D"));
        CheckManagerCapacity(QuatTweenManager, TweenSettings->QuatTweenCapacity, TEXT("Quaternion"));
    }

    return FloatTweenManager->GetCurrentCapacity() + VectorTweenManager->GetCurrentCapacity() + Vector2DTweenManager->GetCurrentCapacity() + QuatTweenManager->GetCurrentCapacity();
}

float NsTweenCore::Ease(const float T, const ENsTweenEase EaseType)
{
    return NsTweenEasing::Ease(T, EaseType);
}

NsTweenInstanceFloat& NsTweenCore::Play(const float Start, const float End, const float DurationSecs, const ENsTweenEase EaseType, TFunction<void(float)> OnUpdate)
{
    return *PlayInternal(FloatTweenManager, Start, End, DurationSecs, EaseType, MoveTemp(OnUpdate));
}

NsTweenInstanceVector& NsTweenCore::Play(const FVector& Start, const FVector& End, const float DurationSecs, const ENsTweenEase EaseType, TFunction<void(FVector)> OnUpdate)
{
    return *PlayInternal(VectorTweenManager, Start, End, DurationSecs, EaseType, MoveTemp(OnUpdate));
}

NsTweenInstanceVector2D& NsTweenCore::Play(const FVector2D Start, const FVector2D End, const float DurationSecs, const ENsTweenEase EaseType, TFunction<void(FVector2D)> OnUpdate)
{
    return *PlayInternal(Vector2DTweenManager, Start, End, DurationSecs, EaseType, MoveTemp(OnUpdate));
}

NsTweenInstanceQuat& NsTweenCore::Play(const FQuat& Start, const FQuat& End, const float DurationSecs, const ENsTweenEase EaseType, TFunction<void(FQuat)> OnUpdate)
{
    return *PlayInternal(QuatTweenManager, Start, End, DurationSecs, EaseType, MoveTemp(OnUpdate));
}
