// Copyright (C) 2025 nulled.softworks. All rights reserved.

#pragma once

#include "Interfaces/IEasingCurve.h"
#include "UObject/WeakObjectPtr.h"

class UCurveFloat;

class NSTWEEN_API FNsTweenCurveAssetEasingAdapter : public IEasingCurve
{
public:
    explicit FNsTweenCurveAssetEasingAdapter(TWeakObjectPtr<UCurveFloat> InCurve);

    /** Returns the eased alpha for the supplied normalized time. */
    virtual float Evaluate(float T) const override;

private:
    TWeakObjectPtr<UCurveFloat> Curve;
};

