// Copyright (C) 2025 nulled.softworks. All rights reserved.

#pragma once

#include "Launch/Resources/Version.h"

/**
 * Tween Manager
 */
template <class T>
class NSTWEEN_API NsTweenManager
{
public:
    NsTweenManager(int32 Capacity = 0)
    {
        ActiveTweens.Reserve(Capacity);
        RecycledTweens.Reserve(Capacity);
        TweensToActivate.Reserve(Capacity);
        for (int32 i = 0; i < Capacity; ++i)
        {
            RecycledTweens.Add(new T());
        }
    }

    ~NsTweenManager()
    {
        for (const T* const Tween : ActiveTweens)
        {
            delete Tween;
        }
        for (const T* const Tween : RecycledTweens)
        {
            delete Tween;
        }
        for (const T* const Tween : TweensToActivate)
        {
            delete Tween;
        }
    }

    void EnsureCapacity(const int32 Num)
    {
        const int32 NumExisting = ActiveTweens.Num() + TweensToActivate.Num() + RecycledTweens.Num();
        for (int32 i = NumExisting; i < Num; ++i)
        {
            RecycledTweens.Add(new T());
        }
    }

    int32 GetCurrentCapacity() const
    {
        return ActiveTweens.Num() + TweensToActivate.Num() + RecycledTweens.Num();
    }

    void Update(const float UnscaledDeltaSeconds, const float DilatedDeltaSeconds, const bool bIsGamePaused)
    {
        for (T* const Tween : TweensToActivate)
        {
            Tween->Start();
            ActiveTweens.Add(Tween);
        }
        TweensToActivate.Reset();

        for (int32 i = ActiveTweens.Num() - 1; i >= 0; --i)
        {
            if (FNsTweenInstance* CurTween = static_cast<FNsTweenInstance*>(ActiveTweens[i]))
            {
                CurTween->Update(UnscaledDeltaSeconds, DilatedDeltaSeconds, bIsGamePaused);
                if (!CurTween->IsActive())
                {
                    RecycleTween(ActiveTweens[i]);
                    ActiveTweens.RemoveAt(i);
                }
            }
        }
    }

    void ClearActiveTweens()
    {
        for (T* const Tween : TweensToActivate)
        {
            Tween->Destroy();
            RecycledTweens.Add(Tween);
        }
        TweensToActivate.Reset();

        for (T* const Tween : ActiveTweens)
        {
            Tween->Destroy();
            RecycledTweens.Add(Tween);
        }
        ActiveTweens.Reset();
    }

    T* CreateTween()
    {
        T* NewTween = GetNewTween();
        TweensToActivate.Add(NewTween);
        return NewTween;
    }

private:
    T* GetNewTween()
    {
        if (RecycledTweens.Num() > 0)
        {
#if ENGINE_MAJOR_VERSION >= 5 && ENGINE_MINOR_VERSION >= 4
            return RecycledTweens.Pop(EAllowShrinking::No);
#else
            return RecycledTweens.Pop(false);
#endif
        }
        return new T();
    }

    void RecycleTween(T* ToRecycle)
    {
        RecycledTweens.Add(ToRecycle);
    }

private:

    /** Active tweens */
    TArray<T*> ActiveTweens;

    /** Recycled tweens */
    TArray<T*> RecycledTweens;

    /** Tweens pending activation */
    TArray<T*> TweensToActivate;
};
