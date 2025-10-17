#include "NsTweenFunctionLibrary.h"
#include "NsTweenSubsystem.h"
#include "ValueStrategies/TweenValue_Float.h"
#include "ValueStrategies/TweenValue_Vector.h"
#include "ValueStrategies/TweenValue_Rotator.h"
#include "ValueStrategies/TweenValue_Transform.h"
#include "ValueStrategies/TweenValue_Color.h"
#include "Engine/World.h"
#include "Engine/Engine.h"


FNsTweenHandle UNsTweenBlueprintLibrary::PlayFloatTween(float StartValue, float EndValue, float& Target, const FNsTweenSpec& Spec)
{
    if (UNsTweenSubsystem* Manager = UNsTweenSubsystem::GetSubsystem())
    {
        TSharedPtr<ITweenValue> Strategy = MakeShared<FTweenValue_Float>(&Target, StartValue, EndValue);
        return Manager->EnqueueSpawn(Spec, Strategy);
    }

    return FNsTweenHandle();
}

FNsTweenHandle UNsTweenBlueprintLibrary::PlayVectorTween(const FVector& StartValue, const FVector& EndValue, FVector& Target, const FNsTweenSpec& Spec)
{
    if (UNsTweenSubsystem* Manager = UNsTweenSubsystem::GetSubsystem())
    {
        TSharedPtr<ITweenValue> Strategy = MakeShared<FTweenValue_Vector>(&Target, StartValue, EndValue);
        return Manager->EnqueueSpawn(Spec, Strategy);
    }

    return FNsTweenHandle();
}

FNsTweenHandle UNsTweenBlueprintLibrary::PlayRotatorTween(const FRotator& StartValue, const FRotator& EndValue, FRotator& Target, const FNsTweenSpec& Spec)
{
    if (UNsTweenSubsystem* Manager = UNsTweenSubsystem::GetSubsystem())
    {
        TSharedPtr<ITweenValue> Strategy = MakeShared<FTweenValue_Rotator>(&Target, StartValue, EndValue);
        return Manager->EnqueueSpawn(Spec, Strategy);
    }

    return FNsTweenHandle();
}

FNsTweenHandle UNsTweenBlueprintLibrary::PlayTransformTween(const FTransform& StartValue, const FTransform& EndValue, FTransform& Target, const FNsTweenSpec& Spec)
{
    if (UNsTweenSubsystem* Manager = UNsTweenSubsystem::GetSubsystem())
    {
        TSharedPtr<ITweenValue> Strategy = MakeShared<FTweenValue_Transform>(&Target, StartValue, EndValue);
        return Manager->EnqueueSpawn(Spec, Strategy);
    }

    return FNsTweenHandle();
}

FNsTweenHandle UNsTweenBlueprintLibrary::PlayColorTween(const FLinearColor& StartValue, const FLinearColor& EndValue, FLinearColor& Target, const FNsTweenSpec& Spec)
{
    if (UNsTweenSubsystem* Manager = UNsTweenSubsystem::GetSubsystem())
    {
        TSharedPtr<ITweenValue> Strategy = MakeShared<FTweenValue_Color>(&Target, StartValue, EndValue);
        return Manager->EnqueueSpawn(Spec, Strategy);
    }

    return FNsTweenHandle();
}

void UNsTweenBlueprintLibrary::PauseTween(FNsTweenHandle Handle)
{
    if (UNsTweenSubsystem* Manager = UNsTweenSubsystem::GetSubsystem())
    {
        Manager->EnqueuePause(Handle);
    }
}

void UNsTweenBlueprintLibrary::ResumeTween(FNsTweenHandle Handle)
{
    if (UNsTweenSubsystem* Manager = UNsTweenSubsystem::GetSubsystem())
    {
        Manager->EnqueueResume(Handle);
    }
}

void UNsTweenBlueprintLibrary::CancelTween(FNsTweenHandle Handle, bool bApplyFinal)
{
    if (UNsTweenSubsystem* Manager = UNsTweenSubsystem::GetSubsystem())
    {
        Manager->EnqueueCancel(Handle, bApplyFinal);
    }
}

bool UNsTweenBlueprintLibrary::IsTweenActive(FNsTweenHandle Handle)
{
    if (UNsTweenSubsystem* Manager = UNsTweenSubsystem::GetSubsystem())
    {
        return Manager->IsActive(Handle);
    }
    return false;
}

