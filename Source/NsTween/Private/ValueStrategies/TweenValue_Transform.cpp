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

    FVector Location = FMath::Lerp(StartValue.GetLocation(), EndValue.GetLocation(), EasedAlpha);
    FVector Scale = FMath::Lerp(StartValue.GetScale3D(), EndValue.GetScale3D(), EasedAlpha);
    FQuat Rotation = FQuat::Slerp(StartValue.GetRotation(), EndValue.GetRotation(), EasedAlpha).GetNormalized();

    Target->SetLocationAndRotation(Location, Rotation);
    Target->SetScale3D(Scale);
}

void FTweenValue_Transform::ApplyFinal()
{
    if (Target)
    {
        *Target = EndValue;
    }
}

