// Copyright (C) 2025 nulled.softworks. All rights reserved.

#include "Easing/NsTweenPolynomialEasing.h"
#include "Math/UnrealMathUtility.h"

namespace
{
/** Returns the bounce-out eased value used by several presets. */
float EaseOutBounce(float T)
{
    const float N1 = 7.5625f;
    const float D1 = 2.75f;

    if (T < 1.f / D1)
    {
        return N1 * T * T;
    }
    if (T < 2.f / D1)
    {
        T -= 1.5f / D1;
        return N1 * T * T + 0.75f;
    }
    if (T < 2.5f / D1)
    {
        T -= 2.25f / D1;
        return N1 * T * T + 0.9375f;
    }

    T -= 2.625f / D1;
    return N1 * T * T + 0.984375f;
}
} // namespace

FNsTweenPolynomialEasing::FNsTweenPolynomialEasing(ENsTweenEase InPreset)
    : Preset(InPreset)
{
}

float FNsTweenPolynomialEasing::Evaluate(float T) const
{
    const float X = FMath::Clamp(T, 0.f, 1.f);

    switch (Preset)
    {
    case ENsTweenEase::InSine:
        return 1.f - FMath::Cos((X * PI) * 0.5f);
    case ENsTweenEase::OutSine:
        return FMath::Sin((X * PI) * 0.5f);
    case ENsTweenEase::InOutSine:
        return -0.5f * (FMath::Cos(PI * X) - 1.f);
    case ENsTweenEase::InQuad:
        return X * X;
    case ENsTweenEase::OutQuad:
        return 1.f - (1.f - X) * (1.f - X);
    case ENsTweenEase::InOutQuad:
        return X < 0.5f ? 2.f * X * X : 1.f - FMath::Pow(-2.f * X + 2.f, 2.f) * 0.5f;
    case ENsTweenEase::InCubic:
        return X * X * X;
    case ENsTweenEase::OutCubic:
        return 1.f - FMath::Pow(1.f - X, 3.f);
    case ENsTweenEase::InOutCubic:
        return X < 0.5f ? 4.f * X * X * X : 1.f - FMath::Pow(-2.f * X + 2.f, 3.f) * 0.5f;
    case ENsTweenEase::InQuart:
        return X * X * X * X;
    case ENsTweenEase::OutQuart:
        return 1.f - FMath::Pow(1.f - X, 4.f);
    case ENsTweenEase::InOutQuart:
        return X < 0.5f ? 8.f * FMath::Pow(X, 4.f) : 1.f - FMath::Pow(-2.f * X + 2.f, 4.f) * 0.5f;
    case ENsTweenEase::InQuint:
        return FMath::Pow(X, 5.f);
    case ENsTweenEase::OutQuint:
        return 1.f - FMath::Pow(1.f - X, 5.f);
    case ENsTweenEase::InOutQuint:
        return X < 0.5f ? 16.f * FMath::Pow(X, 5.f) : 1.f - FMath::Pow(-2.f * X + 2.f, 5.f) * 0.5f;
    case ENsTweenEase::InExpo:
        return FMath::IsNearlyZero(X) ? 0.f : FMath::Pow(2.f, 10.f * X - 10.f);
    case ENsTweenEase::OutExpo:
        return FMath::IsNearlyEqual(X, 1.f) ? 1.f : 1.f - FMath::Pow(2.f, -10.f * X);
    case ENsTweenEase::InOutExpo:
        if (FMath::IsNearlyZero(X))
        {
            return 0.f;
        }
        if (FMath::IsNearlyEqual(X, 1.f))
        {
            return 1.f;
        }
        if (X < 0.5f)
        {
            return FMath::Pow(2.f, 20.f * X - 10.f) * 0.5f;
        }
        return (2.f - FMath::Pow(2.f, -20.f * X + 10.f)) * 0.5f;
    case ENsTweenEase::InCirc:
        return 1.f - FMath::Sqrt(1.f - X * X);
    case ENsTweenEase::OutCirc:
        return FMath::Sqrt(1.f - FMath::Pow(X - 1.f, 2.f));
    case ENsTweenEase::InOutCirc:
        return X < 0.5f
            ? (1.f - FMath::Sqrt(1.f - FMath::Pow(2.f * X, 2.f))) * 0.5f
            : (FMath::Sqrt(1.f - FMath::Pow(-2.f * X + 2.f, 2.f)) + 1.f) * 0.5f;
    case ENsTweenEase::InElastic:
    {
        if (FMath::IsNearlyZero(X))
        {
            return 0.f;
        }
        if (FMath::IsNearlyEqual(X, 1.f))
        {
            return 1.f;
        }

        const float C = (2.f * PI) / 3.f;
        return -FMath::Pow(2.f, 10.f * X - 10.f) * FMath::Sin((X * 10.f - 10.75f) * C);
    }
    case ENsTweenEase::OutElastic:
    {
        if (FMath::IsNearlyZero(X))
        {
            return 0.f;
        }
        if (FMath::IsNearlyEqual(X, 1.f))
        {
            return 1.f;
        }

        const float C = (2.f * PI) / 3.f;
        return FMath::Pow(2.f, -10.f * X) * FMath::Sin((X * 10.f - 0.75f) * C) + 1.f;
    }
    case ENsTweenEase::InOutElastic:
    {
        if (FMath::IsNearlyZero(X))
        {
            return 0.f;
        }
        if (FMath::IsNearlyEqual(X, 1.f))
        {
            return 1.f;
        }

        const float C = (2.f * PI) / 4.5f;
        if (X < 0.5f)
        {
            return -0.5f * FMath::Pow(2.f, 20.f * X - 10.f) * FMath::Sin((20.f * X - 11.125f) * C);
        }
        return FMath::Pow(2.f, -20.f * X + 10.f) * FMath::Sin((20.f * X - 11.125f) * C) * 0.5f + 1.f;
    }
    case ENsTweenEase::InBounce:
        return 1.f - EaseOutBounce(1.f - X);
    case ENsTweenEase::OutBounce:
        return EaseOutBounce(X);
    case ENsTweenEase::InOutBounce:
        return X < 0.5f
            ? (1.f - EaseOutBounce(1.f - 2.f * X)) * 0.5f
            : (1.f + EaseOutBounce(2.f * X - 1.f)) * 0.5f;
    case ENsTweenEase::InBack:
    {
        const float C1 = 1.70158f;
        const float C3 = C1 + 1.f;
        return C3 * X * X * X - C1 * X * X;
    }
    case ENsTweenEase::OutBack:
    {
        const float C1 = 1.70158f;
        const float C3 = C1 + 1.f;
        return 1.f + C3 * FMath::Pow(X - 1.f, 3.f) + C1 * FMath::Pow(X - 1.f, 2.f);
    }
    case ENsTweenEase::InOutBack:
    {
        const float C1 = 1.70158f;
        const float C2 = C1 * 1.525f;
        if (X < 0.5f)
        {
            return FMath::Pow(2.f * X, 2.f) * ((C2 + 1.f) * 2.f * X - C2) * 0.5f;
        }
        return (FMath::Pow(2.f * X - 2.f, 2.f) * ((C2 + 1.f) * (X * 2.f - 2.f) + C2) + 2.f) * 0.5f;
    }
    case ENsTweenEase::Linear:
    default:
        return X;
    }
}

