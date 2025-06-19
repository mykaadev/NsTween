#pragma once

#include "NsTweenCore.h"
#include "Curves/CurveFloat.h"

namespace NsTweenAsyncActionHelpers
{
    template<typename T>
    FORCEINLINE T Interpolate(const T& Start, const T& End, float Alpha)
    {
        return FMath::Lerp(Start, End, Alpha);
    }

    template<>
    FORCEINLINE FQuat Interpolate<FQuat>(const FQuat& Start, const FQuat& End, float Alpha)
    {
        return FQuat::Slerp(Start, End, Alpha);
    }

    template<typename T, typename Callback>
    FORCEINLINE FNsTweenInstance* CreateTween(const T& Start, const T& End, float DurationSecs, ENsTweenEase EaseType, Callback&& OnUpdate)
    {
        return &NsTweenCore::Play(Start, End, DurationSecs, EaseType, Forward<Callback>(OnUpdate));
    }

    template<typename T, typename Callback>
    FORCEINLINE FNsTweenInstance* CreateTweenCustomCurve(const T& Start, const T& End, float DurationSecs, UCurveFloat* Curve, ENsTweenEase EaseType, Callback&& OnUpdate)
    {
        return &NsTweenCore::Play(0.f, 1.f, DurationSecs, EaseType, [=](float TVal)
        {
            const float Alpha = Curve->GetFloatValue(TVal);
            OnUpdate(Interpolate(Start, End, Alpha));
        });
    }
}
