// Copyright (C) 2025 nulled.softworks. All rights reserved.

#pragma once

#include "Interfaces/IEasingCurve.h"

struct FNsTweenBezierDefinition
{
    FVector2f P0 = FVector2f(0.f, 0.f);
    FVector2f P1 = FVector2f(0.25f, 0.1f);
    FVector2f P2 = FVector2f(0.25f, 1.0f);
    FVector2f P3 = FVector2f(1.f, 1.f);
};

class NSTWEEN_API FNsTweenBezierEasing : public IEasingCurve
{
public:
    explicit FNsTweenBezierEasing(const FVector4& InControlPoints);

    /** Returns the eased alpha for the supplied normalized time. */
    virtual float Evaluate(float T) const override;

private:
    float SampleCurveX(float T) const;
    float SampleCurveY(float T) const;
    float SampleDerivativeX(float T) const;
    float Solve(float X) const;

    FVector4 ControlPoints;
};

