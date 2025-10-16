// Copyright (C) 2024 mykaa. All rights reserved.

#pragma once

#include "Interfaces/IEasingCurve.h"

struct FNovaBezierDefinition
{
    FVector2f P0 = FVector2f(0.f, 0.f);
    FVector2f P1 = FVector2f(0.25f, 0.1f);
    FVector2f P2 = FVector2f(0.25f, 1.0f);
    FVector2f P3 = FVector2f(1.f, 1.f);
};

class NSTWEEN_API FBezierEasing : public IEasingCurve
{
public:
    explicit FBezierEasing(const FVector4& InControlPoints);

    virtual float Evaluate(float T) const override;

private:
    float SampleCurveX(float T) const;
    float SampleCurveY(float T) const;
    float SampleDerivativeX(float T) const;
    float Solve(float X) const;

    FVector4 ControlPoints;
};

