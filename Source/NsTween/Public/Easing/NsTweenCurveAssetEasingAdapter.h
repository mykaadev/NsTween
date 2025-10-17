// Copyright (C) 2024 mykaa. All rights reserved.

#pragma once

#include "Interfaces/IEasingCurve.h"
#include "UObject/WeakObjectPtr.h"

class UCurveFloat;

class NSTWEEN_API FNsTweenCurveAssetEasingAdapter : public IEasingCurve
{
public:
    explicit FNsTweenCurveAssetEasingAdapter(TWeakObjectPtr<UCurveFloat> InCurve);

    virtual float Evaluate(float T) const override;

private:
    TWeakObjectPtr<UCurveFloat> Curve;
};

