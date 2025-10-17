// Copyright (C) 2025 nulled.softworks. All rights reserved.

#pragma once

/** Defines the contract for objects that apply tweened values. */
class NSTWEEN_API ITweenValue
{
public:
    /** Virtual destructor to ensure proper cleanup of derived strategies. */
    virtual ~ITweenValue() = default;

    /** Prepares the strategy before the first update is applied. */
    virtual void Initialize() = 0;

    /** Applies an eased alpha to update the target value. */
    virtual void Apply(float EasedAlpha) = 0;

    /** Applies the final value when the tween completes. */
    virtual void ApplyFinal() { Apply(1.0f); }
};

