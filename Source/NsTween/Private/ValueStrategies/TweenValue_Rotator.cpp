#include "ValueStrategies/TweenValue_Rotator.h"
#include "Math/Quat.h"
#include "Math/UnrealMathUtility.h"

FTweenValue_Rotator::FTweenValue_Rotator(FRotator* InTarget, const FRotator& InStart, const FRotator& InEnd)
    : Target(InTarget)
    , StartQuat(InStart.Quaternion())
    , EndQuat(InEnd.Quaternion())
{
}

void FTweenValue_Rotator::Initialize()
{
    if (Target)
    {
        *Target = StartQuat.Rotator();
    }
}

void FTweenValue_Rotator::Apply(float EasedAlpha)
{
    if (Target)
    {
        FQuat Result = FQuat::Slerp(StartQuat, EndQuat, EasedAlpha).GetNormalized();
        *Target = Result.Rotator();
    }
}

void FTweenValue_Rotator::ApplyFinal()
{
    if (Target)
    {
        *Target = EndQuat.Rotator();
    }
}

