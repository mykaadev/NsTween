#include "Easing/CurveAssetEasingAdapter.h"
#include "Curves/CurveFloat.h"
#include "Math/UnrealMathUtility.h"

FCurveAssetEasingAdapter::FCurveAssetEasingAdapter(TWeakObjectPtr<UCurveFloat> InCurve)
    : Curve(InCurve)
{
}

float FCurveAssetEasingAdapter::Evaluate(float T) const
{
    if (const UCurveFloat* CurveAsset = Curve.Get())
    {
        return CurveAsset->GetFloatValue(FMath::Clamp(T, 0.f, 1.f));
    }

    return FMath::Clamp(T, 0.f, 1.f);
}

