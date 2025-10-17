// Copyright (C) 2025 nulled.softworks. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "Templates/NsTweenInterpolator.h"
#include "Interfaces/ITweenValue.h"

/** Tween value strategy that invokes a callback for each interpolation update. */
template <typename TValue>
class FNsTweenCallbackValue : public ITweenValue
{
public:
    /** Type alias describing the function executed when the tween updates. */
    using FUpdateFunction = TFunction<void(const TValue&)>;

public:
    /** Constructs the strategy with the provided start, end, and update callback. */
    FNsTweenCallbackValue(const TValue& InStart, const TValue& InEnd, FUpdateFunction InUpdate)
        : StartValue(InStart)
        , EndValue(InEnd)
        , UpdateFunction(MoveTemp(InUpdate))
    {
    }

    /** Initializes the tween, applying the start value immediately. */
    virtual void Initialize() override
    {
        if (UpdateFunction)
        {
            UpdateFunction(StartValue);
        }
    }

    /** Applies an interpolated value for the supplied eased alpha. */
    virtual void Apply(float EasedAlpha) override
    {
        if (UpdateFunction)
        {
            const TValue Interpolated = FNsTweenInterpolator<TValue>::Lerp(StartValue, EndValue, EasedAlpha);
            UpdateFunction(Interpolated);
        }
    }

    /** Applies the final value when the tween completes. */
    virtual void ApplyFinal() override
    {
        if (UpdateFunction)
        {
            UpdateFunction(EndValue);
        }
    }

private:
    /** Starting value sampled at the beginning of the tween. */
    TValue StartValue;

    /** Target value sampled at the end of the tween. */
    TValue EndValue;

    /** Callback executed whenever the tween produces a new value. */
    FUpdateFunction UpdateFunction;
};

/** Creates a callback strategy instance for the provided tween value type. */
template <typename TValue>
TSharedPtr<ITweenValue> MakeNsTweenCallbackStrategy(const TValue& StartValue, const TValue& EndValue, TFunction<void(const TValue&)> Update)
{
    return MakeShared<FNsTweenCallbackValue<TValue>>(StartValue, EndValue, MoveTemp(Update));
}

