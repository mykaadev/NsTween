// Copyright (C) 2024 mykaa. All rights reserved.

#include "Classes/NsTweenCore.h"
#include "Classes/NsTweenManager.h"

DEFINE_LOG_CATEGORY(LogNsTween)

constexpr int DEFAULT_FLOAT_TWEEN_CAPACITY = 50;
constexpr int DEFAULT_VECTOR_TWEEN_CAPACITY = 50;
constexpr int DEFAULT_VECTOR2D_TWEEN_CAPACITY = 50;
constexpr int DEFAULT_QUAT_TWEEN_CAPACITY = 10;

NsTweenManager<NsTweenInstanceFloat>* NsTweenCore::FloatTweenManager = nullptr;
NsTweenManager<NsTweenInstanceVector>* NsTweenCore::VectorTweenManager = nullptr;
NsTweenManager<NsTweenInstanceVector2D>* NsTweenCore::Vector2DTweenManager = nullptr;
NsTweenManager<NsTweenInstanceQuat>* NsTweenCore::QuatTweenManager = nullptr;

int NsTweenCore::NumReservedFloat = DEFAULT_FLOAT_TWEEN_CAPACITY;
int NsTweenCore::NumReservedVector = DEFAULT_VECTOR_TWEEN_CAPACITY;
int NsTweenCore::NumReservedVector2D = DEFAULT_VECTOR2D_TWEEN_CAPACITY;
int NsTweenCore::NumReservedQuat = DEFAULT_QUAT_TWEEN_CAPACITY;

void NsTweenCore::Initialize()
{
    FloatTweenManager = new NsTweenManager<NsTweenInstanceFloat>(DEFAULT_FLOAT_TWEEN_CAPACITY);
    VectorTweenManager = new NsTweenManager<NsTweenInstanceVector>(DEFAULT_VECTOR_TWEEN_CAPACITY);
    Vector2DTweenManager = new NsTweenManager<NsTweenInstanceVector2D>(DEFAULT_VECTOR2D_TWEEN_CAPACITY);
    QuatTweenManager = new NsTweenManager<NsTweenInstanceQuat>(DEFAULT_QUAT_TWEEN_CAPACITY);

    NumReservedFloat = DEFAULT_FLOAT_TWEEN_CAPACITY;
    NumReservedVector = DEFAULT_VECTOR_TWEEN_CAPACITY;
    NumReservedVector2D = DEFAULT_VECTOR2D_TWEEN_CAPACITY;
    NumReservedQuat = DEFAULT_QUAT_TWEEN_CAPACITY;
}

void NsTweenCore::Deinitialize()
{
    delete FloatTweenManager;
    delete VectorTweenManager;
    delete Vector2DTweenManager;
    delete QuatTweenManager;
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