// Copyright (C) 2024 mykaa. All rights reserved.

#pragma once

/**
 * Enum used to represent the Ease type
 */
UENUM(BlueprintType)
enum class ENsTweenEase : uint8
{
    Linear,
    Smoothstep,
    Stepped,
    InSine,
    OutSine,
    InOutSine,
    InQuad,
    OutQuad,
    InOutQuad,
    InCubic,
    OutCubic,
    InOutCubic,
    InQuart,
    OutQuart,
    InOutQuart,
    InQuint,
    OutQuint,
    InOutQuint,
    // Like Quintic but even sharper
    InExpo,
    OutExpo,
    InOutExpo,
    InCirc,
    OutCirc,
    InOutCirc,
    InElastic,
    OutElastic,
    InOutElastic,
    InBounce,
    OutBounce,
    InOutBounce,
    // Anticipation; pull back a little before going forward
    InBack,
    OutBack,
    InOutBack,
};

/**
 * Night Shift - Tween Easing
 */
class NSTWEEN_API NsTweenEasing
{
// Functions
public:

    /** Ease function */
    static float Ease(const float InT, ENsTweenEase EaseType);
    /**
     * Ease with overriding parameters
     * @param InT Progress
     * @param InEaseType Ease type
     * @param InParamOne Elastic: Amplitude (1.0) / Back: Overshoot (1.70158) / Stepped: Steps (10) / Smoothstep: x0 (0)
     * @param InParamTwo Elastic: Period (0.2) / SmoothStep: x1 (1)
     */
    static float EaseWithParams(const float InT, const ENsTweenEase InEaseType, const float InParamOne = 0, const float InParamTwo = 0);
    static float EaseLinear(const float InT);
    static float EaseSmoothStep(const float InT, const float InXZero = 0, const float InXOne = 1);
    static float EaseStepped(const float InT, const int32 InSteps = 10);
    static float EaseInSine(const float InT);
    static float EaseOutSine(const float InT);
    static float EaseInOutSine(const float InT);
    static float EaseInQuad(const float InT);
    static float EaseOutQuad(const float InT);
    static float EaseInOutQuad(const float InT);
    static float EaseInCubic(const float InT);
    static float EaseOutCubic(const float InT);
    static float EaseInOutCubic(const float InT);
    static float EaseInQuart(const float InT);
    static float EaseOutQuart(const float InT);
    static float EaseInOutQuart(const float InT);
    static float EaseInQuint(const float InT);
    static float EaseOutQuint(const float InT);
    static float EaseInOutQuint(const float InT);
    static float EaseInExpo(const float InT);
    static float EaseOutExpo(const float InT);
    static float EaseInOutExpo(const float InT);
    static float EaseInCirc(const float InT);
    static float EaseOutCirc(const float InT);
    static float EaseInOutCirc(const float InT);
    static float EaseInElastic(const float InT, const float InAmplitude = 1.0f, const float InPeriod = .2f);
    static float EaseOutElastic(const float InT, const float InAmplitude = 1.0f, const float InPeriod = .2f);
    static float EaseInOutElastic(const float InT, const float InAmplitude = 1.0f, const float InPeriod = .2f);
    static float EaseInBounce(const float InT);
    static float EaseOutBounce(const float InT);
    static float EaseInOutBounce(const float InT);
    static float EaseInBack(const float InT, const float InOvershoot = 1.70158f);
    static float EaseOutBack(const float InT, const float Overshoot = 1.70158f);
    static float EaseInOutBack(const float InT, const float Overshoot = 1.70158f);
};
