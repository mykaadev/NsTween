// Copyright (C) 2024 mykaa. All rights reserved.

#pragma once

#include "Interfaces/IEasingCurve.h"
#include "UObject/WeakObjectPtr.h"

class UCurveFloat;

class NSTWEEN_API FCurveAssetEasingAdapter : public IEasingCurve
{
public:
    explicit FCurveAssetEasingAdapter(TWeakObjectPtr<UCurveFloat> InCurve);

    virtual float Evaluate(float T) const override;

private:
    TWeakObjectPtr<UCurveFloat> Curve;
};

