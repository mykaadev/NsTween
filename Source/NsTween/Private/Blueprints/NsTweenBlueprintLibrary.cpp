#include "Blueprints/NsTweenBlueprintLibrary.h"
#include "Classes/NsTweenCore.h"

float UNsTweenBlueprintLibrary::Ease(float t, ENsTweenEase EaseType)

{
    return NsTweenEasing::Ease(t, EaseType);
}

float UNsTweenBlueprintLibrary::EaseWithParams(float t, ENsTweenEase EaseType, float Param1, float Param2)
{
    return NsTweenEasing::EaseWithParams(t, EaseType, Param1, Param2);
}

void UNsTweenBlueprintLibrary::EnsureTweenCapacity(int NumFloatTweens, int NumVectorTweens, int NumVector2DTweens, int NumQuatTweens)
{
    NsTweenCore::EnsureCapacity(NumFloatTweens, NumVectorTweens, NumVector2DTweens, NumQuatTweens);
}
