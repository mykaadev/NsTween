// Copyright (C) 2025 nulled.softworks. All rights reserved.

#include "ValueStrategies/TweenValue_Transform.h"
#include "Math/Quat.h"
#include "Math/UnrealMathUtility.h"

FTweenValue_Transform::FTweenValue_Transform(FTransform* InTarget, const FTransform& InStart, const FTransform& InEnd)
    : Target(InTarget)
    , StartValue(InStart)
    , EndValue(InEnd)
{
}

void FTweenValue_Transform::Initialize()
{
    if (Target)
    {
        *Target = StartValue;
    }
}

void FTweenValue_Transform::Apply(float EasedAlpha)
{
    if (!Target)
    {
        return;
    }

    const FVector Location = FMath::Lerp(StartValue.GetLocation(), EndValue.GetLocation(), EasedAlpha);
    const FVector Scale = FMath::Lerp(StartValue.GetScale3D(), EndValue.GetScale3D(), EasedAlpha);
    const FQuat Rotation = FQuat::Slerp(StartValue.GetRotation(), EndValue.GetRotation(), EasedAlpha).GetNormalized();

    Target->SetLocation(Location);
    Target->SetRotation(Rotation);
    Target->SetScale3D(Scale);
}

void FTweenValue_Transform::ApplyFinal()
{
    if (Target)
    {
        *Target = EndValue;
    }
}

