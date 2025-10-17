// Copyright (C) 2025 nulled.softworks. All rights reserved.

#pragma once

#include "Interfaces/IEasingCurve.h"
#include "NsTweenTypeLibrary.h"

/** Easing curve implementation that evaluates engine-friendly presets. */
class NSTWEEN_API FNsTweenPolynomialEasing : public IEasingCurve
{
public:
    /** Builds an easing evaluator bound to the provided preset. */
    explicit FNsTweenPolynomialEasing(ENsTweenEase InPreset);

    /** Returns the eased alpha for the supplied normalized time. */
    virtual float Evaluate(float T) const override;

private:
    /** Preset determining the formula used for evaluation. */
    ENsTweenEase Preset;
};

