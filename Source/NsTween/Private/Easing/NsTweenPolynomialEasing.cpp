#include "Easing/NsTweenPolynomialEasing.h"
#include "Math/UnrealMathUtility.h"

FNsTweenPolynomialEasing::FNsTweenPolynomialEasing(ENsTweenEase InPreset)
    : Preset(InPreset)
{
}

float FNsTweenPolynomialEasing::Evaluate(float T) const
{
    T = FMath::Clamp(T, 0.f, 1.f);

    switch (Preset)
    {
    case ENsTweenEase::InSine:
        return 1.f - FMath::Cos((T * PI) * 0.5f);
    case ENsTweenEase::OutSine:
        return FMath::Sin((T * PI) * 0.5f);
    case ENsTweenEase::InOutSine:
        return -0.5f * (FMath::Cos(PI * T) - 1.f);
    case ENsTweenEase::InQuad:
        return T * T;
    case ENsTweenEase::OutQuad:
        return 1.f - (1.f - T) * (1.f - T);
    case ENsTweenEase::InOutQuad:
        return T < 0.5f ? 2.f * T * T : 1.f - FMath::Pow(-2.f * T + 2.f, 2.f) * 0.5f;
    case ENsTweenEase::InCubic:
        return T * T * T;
    case ENsTweenEase::OutCubic:
        return 1.f - FMath::Pow(1.f - T, 3.f);
    case ENsTweenEase::InOutCubic:
        return T < 0.5f ? 4.f * T * T * T : 1.f - FMath::Pow(-2.f * T + 2.f, 3.f) * 0.5f;
    case ENsTweenEase::InExpo:
        return FMath::IsNearlyZero(T) ? 0.f : FMath::Pow(2.f, 10.f * T - 10.f);
    case ENsTweenEase::OutExpo:
        return FMath::IsNearlyEqual(T, 1.f) ? 1.f : 1.f - FMath::Pow(2.f, -10.f * T);
    case ENsTweenEase::InOutExpo:
        if (FMath::IsNearlyZero(T))
        {
            return 0.f;
        }
        if (FMath::IsNearlyEqual(T, 1.f))
        {
            return 1.f;
        }
        if (T < 0.5f)
        {
            return FMath::Pow(2.f, 20.f * T - 10.f) * 0.5f;
        }
        return (2.f - FMath::Pow(2.f, -20.f * T + 10.f)) * 0.5f;
    case ENsTweenEase::Linear:
    default:
        return T;
    }
}

