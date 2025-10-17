// Copyright (C) 2024 mykaa. All rights reserved.

#pragma once

#include "Interfaces/IEasingCurve.h"
#include "NsTweenTypeLibrary.h"

class NSTWEEN_API FNsTweenPolynomialEasing : public IEasingCurve
{
public:
    explicit FNsTweenPolynomialEasing(ENsTweenEase InPreset);

    virtual float Evaluate(float T) const override;

private:
    ENsTweenEase Preset;
};

