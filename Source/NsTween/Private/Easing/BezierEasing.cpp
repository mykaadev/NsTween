#include "Easing/BezierEasing.h"
#include "Math/UnrealMathUtility.h"

namespace
{
    constexpr float EPSILON = 1e-6f;
    constexpr int32 NEWTON_ITERATIONS = 8;
}

FBezierEasing::FBezierEasing(const FVector4& InControlPoints)
    : ControlPoints(InControlPoints)
{
}

float FBezierEasing::Evaluate(float T) const
{
    T = FMath::Clamp(T, 0.f, 1.f);
    const float Param = Solve(T);
    return SampleCurveY(Param);
}

float FBezierEasing::SampleCurveX(float T) const
{
    const float OneMinusT = 1.f - T;
    return 3.f * OneMinusT * OneMinusT * T * ControlPoints.X + 3.f * OneMinusT * T * T * ControlPoints.Z + T * T * T;
}

float FBezierEasing::SampleCurveY(float T) const
{
    const float OneMinusT = 1.f - T;
    return 3.f * OneMinusT * OneMinusT * T * ControlPoints.Y + 3.f * OneMinusT * T * T * ControlPoints.W + T * T * T;
}

float FBezierEasing::SampleDerivativeX(float T) const
{
    const float OneMinusT = 1.f - T;
    return 3.f * OneMinusT * OneMinusT * ControlPoints.X + 6.f * OneMinusT * T * (ControlPoints.Z - ControlPoints.X) + 3.f * T * T * (1.f - ControlPoints.Z);
}

float FBezierEasing::Solve(float X) const
{
    float T = X;
    for (int32 Iteration = 0; Iteration < NEWTON_ITERATIONS; ++Iteration)
    {
        const float CurrentX = SampleCurveX(T) - X;
        if (FMath::Abs(CurrentX) < EPSILON)
        {
            return T;
        }

        const float Derivative = SampleDerivativeX(T);
        if (FMath::Abs(Derivative) < EPSILON)
        {
            break;
        }

        T -= CurrentX / Derivative;
        T = FMath::Clamp(T, 0.f, 1.f);
    }

    float Lower = 0.f;
    float Upper = 1.f;
    T = X;

    while (Upper - Lower > EPSILON)
    {
        const float CurrentX = SampleCurveX(T);
        if (FMath::Abs(CurrentX - X) < EPSILON)
        {
            break;
        }

        if (CurrentX < X)
        {
            Lower = T;
        }
        else
        {
            Upper = T;
        }
        T = 0.5f * (Lower + Upper);
    }

    return T;
}

