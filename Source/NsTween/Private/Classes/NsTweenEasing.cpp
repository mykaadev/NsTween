// Copyright (C) 2024 mykaa. All rights reserved.

#include "Classes/NsTweenEasing.h"

const float BACK_INOUT_OVERSHOOT_MODIFIER = 1.525f;
const float BOUNCE_R = 1.0f / 2.75f;       // reciprocal
const float BOUNCE_K1 = BOUNCE_R;          // 36.36%
const float BOUNCE_K2 = 2 * BOUNCE_R;      // 72.72%
const float BOUNCE_K3 = 1.5f * BOUNCE_R;   // 54.54%
const float BOUNCE_K4 = 2.5f * BOUNCE_R;   // 90.90%
const float BOUNCE_K5 = 2.25f * BOUNCE_R;  // 81.81%
const float BOUNCE_K6 = 2.625f * BOUNCE_R; // 95.45%
const float BOUNCE_K0 = 7.5625f;

float NsTweenEasing::Ease(const float InT, const ENsTweenEase EaseType)
{
    switch (EaseType)
    {
        default:
        case ENsTweenEase::Linear:
            return EaseLinear(InT);
        case ENsTweenEase::Smoothstep:
            return EaseSmoothStep(InT);
        case ENsTweenEase::Stepped:
            return EaseStepped(InT);
        case ENsTweenEase::InSine:
            return EaseInSine(InT);
        case ENsTweenEase::OutSine:
            return EaseOutSine(InT);
        case ENsTweenEase::InOutSine:
            return EaseInOutSine(InT);
        case ENsTweenEase::InQuad:
            return EaseInQuad(InT);
        case ENsTweenEase::OutQuad:
            return EaseOutQuad(InT);
        case ENsTweenEase::InOutQuad:
            return EaseInOutQuad(InT);
        case ENsTweenEase::InCubic:
            return EaseInCubic(InT);
        case ENsTweenEase::OutCubic:
            return EaseOutCubic(InT);
        case ENsTweenEase::InOutCubic:
            return EaseInOutCubic(InT);
        case ENsTweenEase::InQuart:
            return EaseInQuart(InT);
        case ENsTweenEase::OutQuart:
            return EaseOutQuart(InT);
        case ENsTweenEase::InOutQuart:
            return EaseInOutQuart(InT);
        case ENsTweenEase::InQuint:
            return EaseInQuint(InT);
        case ENsTweenEase::OutQuint:
            return EaseOutQuint(InT);
        case ENsTweenEase::InOutQuint:
            return EaseInOutQuint(InT);
        case ENsTweenEase::InExpo:
            return EaseInExpo(InT);
        case ENsTweenEase::OutExpo:
            return EaseOutExpo(InT);
        case ENsTweenEase::InOutExpo:
            return EaseInOutExpo(InT);
        case ENsTweenEase::InCirc:
            return EaseInCirc(InT);
        case ENsTweenEase::OutCirc:
            return EaseOutCirc(InT);
        case ENsTweenEase::InOutCirc:
            return EaseInOutCirc(InT);
        case ENsTweenEase::InElastic:
            return EaseInElastic(InT);
        case ENsTweenEase::OutElastic:
            return EaseOutElastic(InT);
        case ENsTweenEase::InOutElastic:
            return EaseInOutElastic(InT);
        case ENsTweenEase::InBounce:
            return EaseInBounce(InT);
        case ENsTweenEase::OutBounce:
            return EaseOutBounce(InT);
        case ENsTweenEase::InOutBounce:
            return EaseInOutBounce(InT);
        case ENsTweenEase::InBack:
            return EaseInBack(InT);
        case ENsTweenEase::OutBack:
            return EaseOutBack(InT);
        case ENsTweenEase::InOutBack:
            return EaseInOutBack(InT);
    }
}

float NsTweenEasing::EaseWithParams(const float InT, const ENsTweenEase InEaseType, const float InParamOne, const float InParamTwo)
{
    if (InParamOne == 0 && InParamTwo == 0)
    {
        return Ease(InT, InEaseType);
    }

    switch (InEaseType)
    {
        default:
        case ENsTweenEase::Linear:
            return EaseLinear(InT);
        case ENsTweenEase::Smoothstep:
            return EaseSmoothStep(InT, InParamOne, InParamTwo);
        case ENsTweenEase::Stepped:
            return EaseStepped(InT, InParamOne);
        case ENsTweenEase::InSine:
            return EaseInSine(InT);
        case ENsTweenEase::OutSine:
            return EaseOutSine(InT);
        case ENsTweenEase::InOutSine:
            return EaseInOutSine(InT);
        case ENsTweenEase::InQuad:
            return EaseInQuad(InT);
        case ENsTweenEase::OutQuad:
            return EaseOutQuad(InT);
        case ENsTweenEase::InOutQuad:
            return EaseInOutQuad(InT);
        case ENsTweenEase::InCubic:
            return EaseInCubic(InT);
        case ENsTweenEase::OutCubic:
            return EaseOutCubic(InT);
        case ENsTweenEase::InOutCubic:
            return EaseInOutCubic(InT);
        case ENsTweenEase::InQuart:
            return EaseInQuart(InT);
        case ENsTweenEase::OutQuart:
            return EaseOutQuart(InT);
        case ENsTweenEase::InOutQuart:
            return EaseInOutQuart(InT);
        case ENsTweenEase::InQuint:
            return EaseInQuint(InT);
        case ENsTweenEase::OutQuint:
            return EaseOutQuint(InT);
        case ENsTweenEase::InOutQuint:
            return EaseInOutQuint(InT);
        case ENsTweenEase::InExpo:
            return EaseInExpo(InT);
        case ENsTweenEase::OutExpo:
            return EaseOutExpo(InT);
        case ENsTweenEase::InOutExpo:
            return EaseInOutExpo(InT);
        case ENsTweenEase::InCirc:
            return EaseInCirc(InT);
        case ENsTweenEase::OutCirc:
            return EaseOutCirc(InT);
        case ENsTweenEase::InOutCirc:
            return EaseInOutCirc(InT);
        case ENsTweenEase::InElastic:
            return EaseInElastic(InT, InParamOne, InParamTwo);
        case ENsTweenEase::OutElastic:
            return EaseOutElastic(InT, InParamOne, InParamTwo);
        case ENsTweenEase::InOutElastic:
            return EaseInOutElastic(InT, InParamOne, InParamTwo);
        case ENsTweenEase::InBounce:
            return EaseInBounce(InT);
        case ENsTweenEase::OutBounce:
            return EaseOutBounce(InT);
        case ENsTweenEase::InOutBounce:
            return EaseInOutBounce(InT);
        case ENsTweenEase::InBack:
            return EaseInBack(InT, InParamOne);
        case ENsTweenEase::OutBack:
            return EaseOutBack(InT, InParamOne);
        case ENsTweenEase::InOutBack:
            return EaseInOutBack(InT, InParamOne);
    }
}

float NsTweenEasing::EaseLinear(const float InT)
{
    return InT;
}

float NsTweenEasing::EaseSmoothStep(const float InT, const float InXZero, const float InXOne)
{
    const float X = FMath::Clamp<float>((InT - InXZero) / (InXOne - InXZero), 0.0f, 1.0f);
    return X * X * (3.0f - 2.0f * X);
}

float NsTweenEasing::EaseStepped(const float InT, const int32 InSteps)
{
    if (InT <= 0)
    {
        return 0;
    }
    else if (InT >= 1)
    {
        return 1;
    }
    else
    {
        return FMath::FloorToFloat(InSteps * InT) / InSteps;
    }
}

float NsTweenEasing::EaseInSine(const float InT)
{
    return 1 - FMath::Cos(InT * PI * .5f);
}

float NsTweenEasing::EaseOutSine(const float InT)
{
    return FMath::Sin(InT * PI * .5f);
}

float NsTweenEasing::EaseInOutSine(const float InT)
{
    return 0.5f * (1 - FMath::Cos(InT * PI));
}

float NsTweenEasing::EaseInQuad(const float InT)
{
    return InT * InT;
}

float NsTweenEasing::EaseOutQuad(const float InT)
{
    return InT * (2 - InT);
}

float NsTweenEasing::EaseInOutQuad(const float InT)
{
    const float T2 = InT * 2;
    if (T2 < 1)
    {
        return InT * T2;
    }
    else
    {
        const float M = InT - 1;
        return 1 - M * M * 2;
    }
}

float NsTweenEasing::EaseInCubic(const float InT)
{
    return InT * InT * InT;
}

float NsTweenEasing::EaseOutCubic(const float InT)
{
    const float M = InT - 1;
    return 1 + M * M * M;
}

float NsTweenEasing::EaseInOutCubic(const float InT)
{
    const float T2 = InT * 2;
    if (T2 < 1)
    {
        return InT * T2 * T2;
    }
    else
    {
        const float M = InT - 1;
        return 1 + M * M * M * 4;
    }
}

float NsTweenEasing::EaseInQuart(const float InT)
{
    return InT * InT * InT * InT;
}

float NsTweenEasing::EaseOutQuart(const float InT)
{
    const float M = InT - 1;
    return 1 - M * M * M * M;
}

float NsTweenEasing::EaseInOutQuart(const float InT)
{
    const float T2 = InT * 2;
    if (T2 < 1)
    {
        return InT * T2 * T2 * T2;
    }
    else
    {
        const float M = InT - 1;
        return 1 - M * M * M * M * 8;
    }
}

float NsTweenEasing::EaseInQuint(const float InT)
{
    return InT * InT * InT * InT * InT;
}

float NsTweenEasing::EaseOutQuint(const float InT)
{
    const float M = InT - 1;
    return 1 + M * M * M * M * M;
}

float NsTweenEasing::EaseInOutQuint(const float InT)
{
    const float T2 = InT * 2;
    if (T2 < 1)
    {
        return InT * T2 * T2 * T2 * T2;
    }
    else
    {
        const float M = InT - 1;
        return 1 + M * M * M * M * M * 16;
    }
}

float NsTweenEasing::EaseInExpo(const float InT)
{
    if (InT <= 0)
    {
        return 0;
    }
    if (InT >= 1)
    {
        return 1;
    }
    return FMath::Pow(2, 10 * (InT - 1));
}

float NsTweenEasing::EaseOutExpo(const float InT)
{
    if (InT <= 0)
    {
        return 0;
    }
    if (InT >= 1)
    {
        return 1;
    }
    return 1 - FMath::Pow(2, -10 * InT);
}

float NsTweenEasing::EaseInOutExpo(const float InT)
{
    if (InT <= 0)
    {
        return 0;
    }
    if (InT >= 1)
    {
        return 1;
    }
    if (InT < 0.5f)
    {
        return FMath::Pow(2, 10 * (2 * InT - 1) - 1);
    }
    else
    {
        return 1 - FMath::Pow(2, -10 * (2 * InT - 1) - 1);
    }
}

float NsTweenEasing::EaseInCirc(const float InT)
{
    return 1 - FMath::Sqrt(1 - InT * InT);
}

float NsTweenEasing::EaseOutCirc(const float InT)
{
    const float M = InT - 1;
    return FMath::Sqrt(1 - M * M);
}

float NsTweenEasing::EaseInOutCirc(const float InT)
{
    const float T2 = InT * 2;
    if (T2 < 1)
    {
        return (1 - FMath::Sqrt(1 - T2 * T2)) * .5f;
    }
    else
    {
        const float M = InT - 1;
        return (FMath::Sqrt(1 - 4 * M * M) + 1) * .5f;
    }
}

float NsTweenEasing::EaseInElastic(const float InT, const float InAmplitude, const float InPeriod)
{
    if (InT == 0)
    {
        return 0;
    }
    else if (InT == 1)
    {
        return 1;
    }
    else
    {
        const float M = InT - 1;
        float S = InPeriod / 4.0f;
        if (InAmplitude > 1)
        {
            S = InPeriod * FMath::Asin(1.0f / InAmplitude) / (2.0f * PI);
        }

        return -(InAmplitude * FMath::Pow(2, 10 * M) * FMath::Sin((M - S) * (2.0f * PI) / InPeriod));
    }
}

// baked-in-parameters version
// float NsTweenCore::EaseInElastic(float t)
// {
// 	float m = t - 1;
// 	return -FMath::Pow(2, 10 * m) * FMath::Sin((m * 40 - 3) * PI / 6);
// }

float NsTweenEasing::EaseOutElastic(const float InT, const float InAmplitude, const float InPeriod)
{
    if (InT == 0)
    {
        return 0;
    }
    else if (InT == 1)
    {
        return 1;
    }
    else
    {
        float s = InPeriod / 4.0f;
        if (InAmplitude > 1)
        {
            s = InPeriod * FMath::Asin(1.0f / InAmplitude) / (2.0f * PI);
        }
        return 1.0f + InAmplitude * FMath::Pow(2, -10 * InT) * FMath::Sin((InT - s) * (2.0f * PI) / InPeriod);
    }
}

// baked-in-parameters version
// float NsTweenCore::EaseOutElastic(float InT)
// {
// 	return 1 + FMath::Pow(2, 10 * (-InT)) * FMath::Sin((-InT * 40 - 3) * PI / 6);
// }
float NsTweenEasing::EaseInOutElastic(const float InT, const float InAmplitude, const float InPeriod)
{
    if (InT == 0)
    {
        return 0;
    }
    else if (InT == 1)
    {
        return 1;
    }
    else
    {
        const float M = 2.0f * InT - 1;
        float S = InPeriod / 4.0f;
        if (InAmplitude > 1)
        {
            S = InPeriod * FMath::Asin(1.0f / InAmplitude) / (2.0f * PI);
        }

        if (M < 0)
        {
            return .5f * -(InAmplitude * FMath::Pow(2, 10 * M) * FMath::Sin((M - S) * (2.0f * PI) / InPeriod));
        }
        else
        {
            return 1.0f + .5f * (InAmplitude * FMath::Pow(2, -10 * InT) * FMath::Sin((InT - S) * (2.0f * PI) / InPeriod));
        }
    }
}

// baked-in-parameters version
// float NsTweenCore::EaseInOutElastic(float t)
// {
// 	float s = 2 * t - 1;
// 	float k = (80 * s - 9) * PI / 18;
// 	if (s < 0)
// 	{
// 		return -.5f * FMath::Pow(2, 10 * s) * FMath::Sin(k);
// 	}
// 	else
// 	{
// 		return 1 + .5f * FMath::Pow(2, -10 * s) * FMath::Sin(k);
// 	}
// }

float NsTweenEasing::EaseInBounce(const float InT)
{
    return 1 - EaseOutBounce(1 - InT);
}

float NsTweenEasing::EaseOutBounce(const float InT)
{
    float t2;

    if (InT < BOUNCE_K1)
    {
        return BOUNCE_K0 * InT * InT;
    }
    else if (InT < BOUNCE_K2)
    {
        t2 = InT - BOUNCE_K3;
        return BOUNCE_K0 * t2 * t2 + 0.75f;
    }
    else if (InT < BOUNCE_K4)
    {
        t2 = InT - BOUNCE_K5;
        return BOUNCE_K0 * t2 * t2 + 0.9375f;
    }
    else
    {
        t2 = InT - BOUNCE_K6;
        return BOUNCE_K0 * t2 * t2 + 0.984375f;
    }
}

float NsTweenEasing::EaseInOutBounce(const float InT)
{
    const float T2 = InT * 2;
    if (T2 < 1)
    {
        return .5f - .5f * EaseOutBounce(1 - T2);
    }
    else
    {
        return .5f + .5f * EaseOutBounce(T2 - 1);
    }
}

float NsTweenEasing::EaseInBack(const float InT, const float InOvershoot)
{
    return InT * InT * ((InOvershoot + 1) * InT - InOvershoot);
}

float NsTweenEasing::EaseOutBack(const float InT, const float Overshoot)
{
    const float M = InT - 1;
    return 1 + M * M * (M * (Overshoot + 1) + Overshoot);
}

float NsTweenEasing::EaseInOutBack(const float InT, const float Overshoot)
{
    const float T2 = InT * 2;
    const float S = Overshoot * BACK_INOUT_OVERSHOOT_MODIFIER;
    if (InT < .5f)
    {
        return InT * T2 * (T2 * (S + 1) - S);
    }
    else
    {
        const float M = InT - 1;
        return 1 + 2 * M * M * (2 * M * (S + 1) + S);
    }
}
