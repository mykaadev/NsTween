#include "ValueStrategies/TweenValue_Float.h"
#include "Math/UnrealMathUtility.h"

FTweenValue_Float::FTweenValue_Float(float* InTarget, float InStart, float InEnd)
    : Target(InTarget)
    , StartValue(InStart)
    , EndValue(InEnd)
{
}

void FTweenValue_Float::Initialize()
{
    if (Target)
    {
        *Target = StartValue;
    }
}

void FTweenValue_Float::Apply(float EasedAlpha)
{
    if (Target)
    {
        *Target = FMath::Lerp(StartValue, EndValue, EasedAlpha);
    }
}

void FTweenValue_Float::ApplyFinal()
{
    if (Target)
    {
        *Target = EndValue;
    }
}

