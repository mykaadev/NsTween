#include "Easing/NsTweenCurveAssetEasingAdapter.h"
#include "Curves/CurveFloat.h"
#include "Math/UnrealMathUtility.h"

FNsTweenCurveAssetEasingAdapter::FNsTweenCurveAssetEasingAdapter(TWeakObjectPtr<UCurveFloat> InCurve)
    : Curve(InCurve)
{
}

float FNsTweenCurveAssetEasingAdapter::Evaluate(float T) const
{
    if (const UCurveFloat* CurveAsset = Curve.Get())
    {
        return CurveAsset->GetFloatValue(FMath::Clamp(T, 0.f, 1.f));
    }

    return FMath::Clamp(T, 0.f, 1.f);
}

