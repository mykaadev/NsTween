// Copyright (C) 2025 nulled.softworks. All rights reserved.

#include "ValueStrategies/TweenValue_Vector.h"
#include "Math/UnrealMathUtility.h"

FTweenValue_Vector::FTweenValue_Vector(FVector* InTarget, const FVector& InStart, const FVector& InEnd)
    : Target(InTarget)
    , StartValue(InStart)
    , EndValue(InEnd)
{
}

void FTweenValue_Vector::Initialize()
{
    if (Target)
    {
        *Target = StartValue;
    }
}

void FTweenValue_Vector::Apply(float EasedAlpha)
{
    if (Target)
    {
        *Target = FMath::Lerp(StartValue, EndValue, EasedAlpha);
    }
}

void FTweenValue_Vector::ApplyFinal()
{
    if (Target)
    {
        *Target = EndValue;
    }
}

