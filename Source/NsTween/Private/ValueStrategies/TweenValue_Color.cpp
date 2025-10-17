// Copyright (C) 2025 nulled.softworks. All rights reserved.

#include "ValueStrategies/TweenValue_Color.h"
#include "Math/UnrealMathUtility.h"

FTweenValue_Color::FTweenValue_Color(FLinearColor* InTarget, const FLinearColor& InStart, const FLinearColor& InEnd)
    : Target(InTarget)
    , StartValue(InStart)
    , EndValue(InEnd)
{
}

void FTweenValue_Color::Initialize()
{
    if (Target)
    {
        *Target = StartValue;
    }
}

void FTweenValue_Color::Apply(float EasedAlpha)
{
    if (Target)
    {
        *Target = FMath::Lerp(StartValue, EndValue, EasedAlpha);
    }
}

void FTweenValue_Color::ApplyFinal()
{
    if (Target)
    {
        *Target = EndValue;
    }
}

