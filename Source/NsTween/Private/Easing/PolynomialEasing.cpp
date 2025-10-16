#include "Easing/PolynomialEasing.h"
#include "Math/UnrealMathUtility.h"

FPolynomialEasing::FPolynomialEasing(ENovaEasingPreset InPreset)
    : Preset(InPreset)
{
}

float FPolynomialEasing::Evaluate(float T) const
{
    T = FMath::Clamp(T, 0.f, 1.f);

    switch (Preset)
    {
    case ENovaEasingPreset::EaseInSine:
        return 1.f - FMath::Cos((T * PI) * 0.5f);
    case ENovaEasingPreset::EaseOutSine:
        return FMath::Sin((T * PI) * 0.5f);
    case ENovaEasingPreset::EaseInOutSine:
        return -0.5f * (FMath::Cos(PI * T) - 1.f);
    case ENovaEasingPreset::EaseInQuad:
        return T * T;
    case ENovaEasingPreset::EaseOutQuad:
        return 1.f - (1.f - T) * (1.f - T);
    case ENovaEasingPreset::EaseInOutQuad:
        return T < 0.5f ? 2.f * T * T : 1.f - FMath::Pow(-2.f * T + 2.f, 2.f) * 0.5f;
    case ENovaEasingPreset::EaseInCubic:
        return T * T * T;
    case ENovaEasingPreset::EaseOutCubic:
        return 1.f - FMath::Pow(1.f - T, 3.f);
    case ENovaEasingPreset::EaseInOutCubic:
        return T < 0.5f ? 4.f * T * T * T : 1.f - FMath::Pow(-2.f * T + 2.f, 3.f) * 0.5f;
    case ENovaEasingPreset::EaseInExpo:
        return FMath::IsNearlyZero(T) ? 0.f : FMath::Pow(2.f, 10.f * T - 10.f);
    case ENovaEasingPreset::EaseOutExpo:
        return FMath::IsNearlyEqual(T, 1.f) ? 1.f : 1.f - FMath::Pow(2.f, -10.f * T);
    case ENovaEasingPreset::EaseInOutExpo:
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
    case ENovaEasingPreset::Linear:
    default:
        return T;
    }
}

