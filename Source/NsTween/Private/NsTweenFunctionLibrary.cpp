// Copyright (C) 2025 nulled.softworks. All rights reserved.

#include "NsTweenFunctionLibrary.h"
#include "NsTweenSubsystem.h"
#include "Utils/NsTweenProfiling.h"
#include "ValueStrategies/TweenValue_Color.h"
#include "ValueStrategies/TweenValue_Float.h"
#include "ValueStrategies/TweenValue_Rotator.h"
#include "ValueStrategies/TweenValue_Transform.h"
#include "ValueStrategies/TweenValue_Vector.h"

FNsTweenHandle UNsTweenBlueprintLibrary::PlayFloatTween(float StartValue, float EndValue, float& Target, const FNsTweenSpec& Spec)
{
    NSTWEEN_SCOPE_CYCLE_COUNTER("NsTweenBlueprintLibrary::PlayFloatTween");
    return PlayTypedTween<float, FTweenValue_Float>(Target, StartValue, EndValue, Spec);
}

FNsTweenHandle UNsTweenBlueprintLibrary::PlayVectorTween(const FVector& StartValue, const FVector& EndValue, FVector& Target, const FNsTweenSpec& Spec)
{
    NSTWEEN_SCOPE_CYCLE_COUNTER("NsTweenBlueprintLibrary::PlayVectorTween");
    return PlayTypedTween<FVector, FTweenValue_Vector>(Target, StartValue, EndValue, Spec);
}

FNsTweenHandle UNsTweenBlueprintLibrary::PlayRotatorTween(const FRotator& StartValue, const FRotator& EndValue, FRotator& Target, const FNsTweenSpec& Spec)
{
    NSTWEEN_SCOPE_CYCLE_COUNTER("NsTweenBlueprintLibrary::PlayRotatorTween");
    return PlayTypedTween<FRotator, FTweenValue_Rotator>(Target, StartValue, EndValue, Spec);
}

FNsTweenHandle UNsTweenBlueprintLibrary::PlayTransformTween(const FTransform& StartValue, const FTransform& EndValue, FTransform& Target, const FNsTweenSpec& Spec)
{
    NSTWEEN_SCOPE_CYCLE_COUNTER("NsTweenBlueprintLibrary::PlayTransformTween");
    return PlayTypedTween<FTransform, FTweenValue_Transform>(Target, StartValue, EndValue, Spec);
}

FNsTweenHandle UNsTweenBlueprintLibrary::PlayColorTween(const FLinearColor& StartValue, const FLinearColor& EndValue, FLinearColor& Target, const FNsTweenSpec& Spec)
{
    NSTWEEN_SCOPE_CYCLE_COUNTER("NsTweenBlueprintLibrary::PlayColorTween");
    return PlayTypedTween<FLinearColor, FTweenValue_Color>(Target, StartValue, EndValue, Spec);
}

void UNsTweenBlueprintLibrary::PauseTween(FNsTweenHandle Handle)
{
    NSTWEEN_SCOPE_CYCLE_COUNTER("NsTweenBlueprintLibrary::PauseTween");
    DispatchToSubsystem(Handle, [](UNsTweenSubsystem& Manager, FNsTweenHandle InHandle)
    {
        Manager.EnqueuePause(InHandle);
    });
}

void UNsTweenBlueprintLibrary::ResumeTween(FNsTweenHandle Handle)
{
    NSTWEEN_SCOPE_CYCLE_COUNTER("NsTweenBlueprintLibrary::ResumeTween");
    DispatchToSubsystem(Handle, [](UNsTweenSubsystem& Manager, FNsTweenHandle InHandle)
    {
        Manager.EnqueueResume(InHandle);
    });
}

void UNsTweenBlueprintLibrary::CancelTween(FNsTweenHandle Handle, bool bApplyFinal)
{
    NSTWEEN_SCOPE_CYCLE_COUNTER("NsTweenBlueprintLibrary::CancelTween");
    DispatchToSubsystem(Handle, [bApplyFinal](UNsTweenSubsystem& Manager, FNsTweenHandle InHandle)
    {
        Manager.EnqueueCancel(InHandle, bApplyFinal);
    });
}

bool UNsTweenBlueprintLibrary::IsTweenActive(FNsTweenHandle Handle)
{
    NSTWEEN_SCOPE_CYCLE_COUNTER("NsTweenBlueprintLibrary::IsTweenActive");
    if (UNsTweenSubsystem* Manager = UNsTweenSubsystem::GetSubsystem())
    {
        return Manager->IsActive(Handle);
    }

    return false;
}
