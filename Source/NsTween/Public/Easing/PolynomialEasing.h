// Copyright (C) 2024 mykaa. All rights reserved.

#pragma once

#include "Interfaces/IEasingCurve.h"
#include "TweenEnums.h"

class NSTWEEN_API FPolynomialEasing : public IEasingCurve
{
public:
    explicit FPolynomialEasing(ENovaEasingPreset InPreset);

    virtual float Evaluate(float T) const override;

private:
    ENovaEasingPreset Preset;
};

