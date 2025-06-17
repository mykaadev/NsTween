// Copyright (C) 2025 mykaadev. All rights reserved.

#include "Classes/NsTweenCore.h"
#include "Classes/NsTweenManager.h"
#include "Classes/NsTweenSettings.h"

namespace
{
    template <typename ManagerType>
    FORCEINLINE void EnsureManagerCapacity(ManagerType* Manager, int Num, int& NumReserved)
    {
        Manager->EnsureCapacity(Num);
        NumReserved = Manager->GetCurrentCapacity();
    }

    template <typename ManagerType>
    FORCEINLINE void CheckManagerCapacity(const ManagerType* Manager, int NumReserved, const TCHAR* TypeName)
    {
        const int Current = Manager->GetCurrentCapacity();
        if (Current > NumReserved)
        {
            UE_LOG(LogNsTween,
                   Warning,
                   TEXT("Consider increasing initial capacity for %s tweens with NsTweenCore::EnsureCapacity(). %d were initially reserved, but now there are %d in memory."),
                   TypeName,
                   NumReserved,
                   Current);
        }
    }

    template <typename TweenType, typename ValueType>
    FORCEINLINE TweenType* PlayInternal(NsTweenManager<TweenType>* Manager,
                                        const ValueType& Start,
                                        const ValueType& End,
                                        float DurationSecs,
                                        ENsTweenEase EaseType,
                                        TFunction<void(ValueType)> OnUpdate)
    {
        TweenType* const NewTween = Manager->CreateTween();
        NewTween->Initialize(Start, End, MoveTemp(OnUpdate), DurationSecs, EaseType);
        return NewTween;
    }
}

DEFINE_LOG_CATEGORY(LogNsTween)

NsTweenManager<NsTweenInstanceFloat>* NsTweenCore::FloatTweenManager = nullptr;
NsTweenManager<NsTweenInstanceVector>* NsTweenCore::VectorTweenManager = nullptr;
NsTweenManager<NsTweenInstanceVector2D>* NsTweenCore::Vector2DTweenManager = nullptr;
NsTweenManager<NsTweenInstanceQuat>* NsTweenCore::QuatTweenManager = nullptr;

int NsTweenCore::NumReservedFloat = 0;
int NsTweenCore::NumReservedVector = 0;
int NsTweenCore::NumReservedVector2D = 0;
int NsTweenCore::NumReservedQuat = 0;

namespace
{
    template <typename Func>
    FORCEINLINE void ForEachManager(Func&& Fn)
    {
        Fn(NsTweenCore::FloatTweenManager);
        Fn(NsTweenCore::VectorTweenManager);
        Fn(NsTweenCore::Vector2DTweenManager);
        Fn(NsTweenCore::QuatTweenManager);
    }
}

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
    EnsureManagerCapacity(FloatTweenManager, NumFloatTweens, NumReservedFloat);
    EnsureManagerCapacity(VectorTweenManager, NumVectorTweens, NumReservedVector);
    EnsureManagerCapacity(Vector2DTweenManager, NumVector2DTweens, NumReservedVector2D);
    EnsureManagerCapacity(QuatTweenManager, NumQuatTweens, NumReservedQuat);
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
    ForEachManager([](auto* Manager) { Manager->ClearActiveTweens(); });
}

int NsTweenCore::CheckTweenCapacity()
{
    CheckManagerCapacity(FloatTweenManager, NumReservedFloat, TEXT("Float"));
    CheckManagerCapacity(VectorTweenManager, NumReservedVector, TEXT("Vector (3d vector)"));
    CheckManagerCapacity(Vector2DTweenManager, NumReservedVector2D, TEXT("Vector2D"));
    CheckManagerCapacity(QuatTweenManager, NumReservedQuat, TEXT("Quaternion"));

    return FloatTweenManager->GetCurrentCapacity() + VectorTweenManager->GetCurrentCapacity() + Vector2DTweenManager->GetCurrentCapacity() + QuatTweenManager->GetCurrentCapacity();
}

float NsTweenCore::Ease(const float T, const ENsTweenEase EaseType)
{
    return NsTweenEasing::Ease(T, EaseType);
}

NsTweenInstanceFloat* NsTweenCore::Play(const float Start, const float End, const float DurationSecs, const ENsTweenEase EaseType, TFunction<void(float)> OnUpdate)
{
    return PlayInternal(FloatTweenManager, Start, End, DurationSecs, EaseType, MoveTemp(OnUpdate));
}

NsTweenInstanceVector* NsTweenCore::Play(const FVector& Start, const FVector& End, const float DurationSecs, const ENsTweenEase EaseType, TFunction<void(FVector)> OnUpdate)
{
    return PlayInternal(VectorTweenManager, Start, End, DurationSecs, EaseType, MoveTemp(OnUpdate));
}

NsTweenInstanceVector2D* NsTweenCore::Play(const FVector2D Start, const FVector2D End, const float DurationSecs, const ENsTweenEase EaseType, TFunction<void(FVector2D)> OnUpdate)
{
    return PlayInternal(Vector2DTweenManager, Start, End, DurationSecs, EaseType, MoveTemp(OnUpdate));
}

NsTweenInstanceQuat* NsTweenCore::Play(const FQuat& Start, const FQuat& End, const float DurationSecs, const ENsTweenEase EaseType, TFunction<void(FQuat)> OnUpdate)
{
    return PlayInternal(QuatTweenManager, Start, End, DurationSecs, EaseType, MoveTemp(OnUpdate));
}
