// Copyright (C) 2025 mykaadev. All rights reserved.

#include "Classes/NsTweenCore.h"
#include "Classes/NsTweenManager.h"
#include "Classes/NsTweenSettings.h"

DEFINE_LOG_CATEGORY(LogNsTween)

NsTweenManager<NsTweenInstanceFloat>* NsTweenCore::FloatTweenManager = nullptr;
NsTweenManager<NsTweenInstanceVector>* NsTweenCore::VectorTweenManager = nullptr;
NsTweenManager<NsTweenInstanceVector2D>* NsTweenCore::Vector2DTweenManager = nullptr;
NsTweenManager<NsTweenInstanceQuat>* NsTweenCore::QuatTweenManager = nullptr;

int NsTweenCore::NumReservedFloat = 0;
int NsTweenCore::NumReservedVector = 0;
int NsTweenCore::NumReservedVector2D = 0;
int NsTweenCore::NumReservedQuat = 0;

void NsTweenCore::Initialize()
{
    const UNsTweenSettings* Settings = UNsTweenSettings::GetSettings();
    const int FloatCapacity = Settings ? Settings->FloatTweenCapacity : 0;
    const int VectorCapacity = Settings ? Settings->VectorTweenCapacity : 0;
    const int Vector2DCapacity = Settings ? Settings->Vector2DTweenCapacity : 0;
    const int QuatCapacity = Settings ? Settings->QuatTweenCapacity : 0;

    FloatTweenManager = new NsTweenManager<NsTweenInstanceFloat>(FloatCapacity);
    VectorTweenManager = new NsTweenManager<NsTweenInstanceVector>(VectorCapacity);
    Vector2DTweenManager = new NsTweenManager<NsTweenInstanceVector2D>(Vector2DCapacity);
    QuatTweenManager = new NsTweenManager<NsTweenInstanceQuat>(QuatCapacity);

    NumReservedFloat = FloatCapacity;
    NumReservedVector = VectorCapacity;
    NumReservedVector2D = Vector2DCapacity;
    NumReservedQuat = QuatCapacity;
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

    NumReservedFloat = 0;
    NumReservedVector = 0;
    NumReservedVector2D = 0;
    NumReservedQuat = 0;
}

void NsTweenCore::EnsureCapacity(const int NumFloatTweens, const int NumVectorTweens, const int NumVector2DTweens, const int NumQuatTweens)
{
    FloatTweenManager->EnsureCapacity(NumFloatTweens);
    VectorTweenManager->EnsureCapacity(NumVectorTweens);
    Vector2DTweenManager->EnsureCapacity(NumVector2DTweens);
    QuatTweenManager->EnsureCapacity(NumQuatTweens);

    NumReservedFloat = FloatTweenManager->GetCurrentCapacity();
    NumReservedVector = VectorTweenManager->GetCurrentCapacity();
    NumReservedVector2D = Vector2DTweenManager->GetCurrentCapacity();
    NumReservedQuat = QuatTweenManager->GetCurrentCapacity();
}

void NsTweenCore::EnsureCapacity(const int NumTweens)
{
    EnsureCapacity(NumTweens, NumTweens, NumTweens, NumTweens);
}

void NsTweenCore::Update(const float UnscaledDeltaSeconds, const float DilatedDeltaSeconds, const bool bIsGamePaused)
{
    FloatTweenManager->Update(UnscaledDeltaSeconds, DilatedDeltaSeconds, bIsGamePaused);
    VectorTweenManager->Update(UnscaledDeltaSeconds, DilatedDeltaSeconds, bIsGamePaused);
    Vector2DTweenManager->Update(UnscaledDeltaSeconds, DilatedDeltaSeconds, bIsGamePaused);
    QuatTweenManager->Update(UnscaledDeltaSeconds, DilatedDeltaSeconds, bIsGamePaused);
}

void NsTweenCore::ClearActiveTweens()
{
    FloatTweenManager->ClearActiveTweens();
    VectorTweenManager->ClearActiveTweens();
    Vector2DTweenManager->ClearActiveTweens();
    QuatTweenManager->ClearActiveTweens();
}

int NsTweenCore::CheckTweenCapacity()
{
    if (FloatTweenManager->GetCurrentCapacity() > NumReservedFloat)
    {
        UE_LOG
        (LogNsTween,
         Warning,
         TEXT("Consider increasing initial capacity for Float tweens with NsTweenCore::EnsureCapacity(). %d were initially reserved, but now there are %d in memory."),
         NumReservedFloat,
         FloatTweenManager->GetCurrentCapacity()
        );
    }
    if (VectorTweenManager->GetCurrentCapacity() > NumReservedVector)
    {
        UE_LOG
        (LogNsTween,
         Warning,
         TEXT("Consider increasing initial capacity for Vector (3d vector) tweens with NsTweenCore::EnsureCapacity(). %d were initially reserved, but now there are %d in memory."),
         NumReservedVector,
         VectorTweenManager->GetCurrentCapacity()
        );
    }
    if (Vector2DTweenManager->GetCurrentCapacity() > NumReservedVector2D)
    {
        UE_LOG
        (LogNsTween,
         Warning,
         TEXT("Consider increasing initial capacity for Vector2D tweens with NsTweenCore::EnsureCapacity(). %d were initially reserved, but now there are %d in memory."),
         NumReservedVector2D,
         Vector2DTweenManager->GetCurrentCapacity()
        );
    }
    if (QuatTweenManager->GetCurrentCapacity() > NumReservedQuat)
    {
        UE_LOG
        (LogNsTween,
         Warning,
         TEXT("Consider increasing initial capacity for Quaternion tweens with NsTweenCore::EnsureCapacity(). %d were initially reserved, but now there are %d in memory."),
         NumReservedQuat,
         QuatTweenManager->GetCurrentCapacity()
        );
    }

    return FloatTweenManager->GetCurrentCapacity() + VectorTweenManager->GetCurrentCapacity() + Vector2DTweenManager->GetCurrentCapacity() + QuatTweenManager->GetCurrentCapacity();
}

float NsTweenCore::Ease(const float T, const ENsTweenEase EaseType)
{
    return NsTweenEasing::Ease(T, EaseType);
}

NsTweenInstanceFloat* NsTweenCore::Play(const float Start, const float End, const float DurationSecs, const ENsTweenEase EaseType, TFunction<void(float)> OnUpdate)
{
    NsTweenInstanceFloat* const NewTween = FloatTweenManager->CreateTween();
    NewTween->Initialize(Start, End, MoveTemp(OnUpdate), DurationSecs, EaseType);
    return NewTween;
}

NsTweenInstanceVector* NsTweenCore::Play(const FVector& Start, const FVector& End, const float DurationSecs, const ENsTweenEase EaseType, TFunction<void(FVector)> OnUpdate)
{
    NsTweenInstanceVector* const NewTween = VectorTweenManager->CreateTween();
    NewTween->Initialize(Start, End, MoveTemp(OnUpdate), DurationSecs, EaseType);
    return NewTween;
}

NsTweenInstanceVector2D* NsTweenCore::Play(const FVector2D Start, const FVector2D End, const float DurationSecs, const ENsTweenEase EaseType, TFunction<void(FVector2D)> OnUpdate)
{
    NsTweenInstanceVector2D* const NewTween = Vector2DTweenManager->CreateTween();
    NewTween->Initialize(Start, End, MoveTemp(OnUpdate), DurationSecs, EaseType);
    return NewTween;
}

NsTweenInstanceQuat* NsTweenCore::Play(const FQuat& Start, const FQuat& End, const float DurationSecs, const ENsTweenEase EaseType, TFunction<void(FQuat)> OnUpdate)
{
    NsTweenInstanceQuat* const NewTween = QuatTweenManager->CreateTween();
    NewTween->Initialize(Start, End, MoveTemp(OnUpdate), DurationSecs, EaseType);
    return NewTween;
}
