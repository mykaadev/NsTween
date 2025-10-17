// Copyright (C) 2025 nulled.softworks. All rights reserved.

#pragma once

class NSTWEEN_API IEasingCurve
{
public:
    virtual ~IEasingCurve() = default;

    virtual float Evaluate(float T) const = 0;
};

