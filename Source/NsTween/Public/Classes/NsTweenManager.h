// Copyright (C) 2025 mykaadev. All rights reserved.

#pragma once

/**
 * Tween Manager
 */
template <class T>
class NSTWEEN_API NsTweenManager
{
public:
    NsTweenManager(int Capacity = 0)
    {
        ActiveTweens.Reserve(Capacity);
        RecycledTweens.Reserve(Capacity);
        TweensToActivate.Reserve(Capacity);
        for (int i = 0; i < Capacity; ++i)
        {
            RecycledTweens.Add(new T());
        }
    }

    ~NsTweenManager()
    {
        for (T* Tween : ActiveTweens)
        {
            delete Tween;
        }
        for (T* Tween : RecycledTweens)
        {
            delete Tween;
        }
        for (T* Tween : TweensToActivate)
        {
            delete Tween;
        }
    }

    void EnsureCapacity(int Num)
    {
        const int NumExisting = ActiveTweens.Num() + TweensToActivate.Num() + RecycledTweens.Num();
        for (int i = NumExisting; i < Num; ++i)
        {
            RecycledTweens.Add(new T());
        }
    }

    int GetCurrentCapacity() const
    {
        return ActiveTweens.Num() + TweensToActivate.Num() + RecycledTweens.Num();
    }

    void Update(float UnscaledDeltaSeconds, float DilatedDeltaSeconds, bool bIsGamePaused)
    {
        for (T* Tween : TweensToActivate)
        {
            Tween->Start();
            ActiveTweens.Add(Tween);
        }
        TweensToActivate.Reset();

        for (int32 i = ActiveTweens.Num() - 1; i >= 0; --i)
        {
            NsTweenInstance* CurTween = static_cast<NsTweenInstance*>(ActiveTweens[i]);
            CurTween->Update(UnscaledDeltaSeconds, DilatedDeltaSeconds, bIsGamePaused);
            if (!CurTween->bIsActive)
            {
                RecycleTween(ActiveTweens[i]);
                ActiveTweens.RemoveAt(i);
            }
        }
    }

    void ClearActiveTweens()
    {
        for (T* Tween : TweensToActivate)
        {
            Tween->Destroy();
            RecycledTweens.Add(Tween);
        }
        TweensToActivate.Reset();

        for (T* Tween : ActiveTweens)
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
            return RecycledTweens.Pop(false);
        }
        return new T();
    }

    void RecycleTween(T* ToRecycle)
    {
        RecycledTweens.Add(ToRecycle);
    }

private:
    TArray<T*> ActiveTweens;
    TArray<T*> RecycledTweens;
    TArray<T*> TweensToActivate;
};
