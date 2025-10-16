// Copyright (C) 2024 mykaa. All rights reserved.

#pragma once

class NSTWEEN_API ITweenValue
{
public:
    virtual ~ITweenValue() = default;

    virtual void Initialize() = 0;
    virtual void Apply(float EasedAlpha) = 0;
    virtual void ApplyFinal() { Apply(1.0f); }
};

