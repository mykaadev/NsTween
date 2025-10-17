// Copyright (C) 2025 nulled.softworks. All rights reserved.

#include "NsTweenFunctionLibrary.h"

#include "NsTween.h"
#include "Interfaces/ITweenValue.h"
#include "NsTweenSubsystem.h"
#include "ValueStrategies/TweenValue_Color.h"
#include "ValueStrategies/TweenValue_Float.h"
#include "ValueStrategies/TweenValue_Rotator.h"
#include "ValueStrategies/TweenValue_Transform.h"
#include "ValueStrategies/TweenValue_Vector.h"

namespace
{
/**
 * Helper that routes every Blueprint spawn call through the same builder logic.
 * This guarantees parity with the C++ helpers and keeps the redundant boilerplate in one place.
 */
template <typename TValue, typename TStrategy>
FNsTweenHandle PlayTypedTween(TValue& Target, const TValue& StartValue, const TValue& EndValue, const FNsTweenSpec& Spec)
{
    FNsTweenBuilder Builder = FNsTween::Play(
        Spec,
        [&Target, StartValue, EndValue]() -> TSharedPtr<ITweenValue>
        {
            return MakeShared<TStrategy>(&Target, StartValue, EndValue);
        });

    // Activating via GetHandle ensures the tween is registered with the subsystem before returning.
    return Builder.GetHandle();
}

/** Invokes a subsystem command if the singleton is currently available. */
template <typename TCallback>
void DispatchToSubsystem(FNsTweenHandle Handle, TCallback&& Callback)
{
    if (UNsTweenSubsystem* Manager = UNsTweenSubsystem::GetSubsystem())
    {
        Callback(*Manager, Handle);
    }
}
} // namespace

FNsTweenHandle UNsTweenBlueprintLibrary::PlayFloatTween(float StartValue, float EndValue, float& Target, const FNsTweenSpec& Spec)
{
    return PlayTypedTween<float, FTweenValue_Float>(Target, StartValue, EndValue, Spec);
}

FNsTweenHandle UNsTweenBlueprintLibrary::PlayVectorTween(const FVector& StartValue, const FVector& EndValue, FVector& Target, const FNsTweenSpec& Spec)
{
    return PlayTypedTween<FVector, FTweenValue_Vector>(Target, StartValue, EndValue, Spec);
}

FNsTweenHandle UNsTweenBlueprintLibrary::PlayRotatorTween(const FRotator& StartValue, const FRotator& EndValue, FRotator& Target, const FNsTweenSpec& Spec)
{
    return PlayTypedTween<FRotator, FTweenValue_Rotator>(Target, StartValue, EndValue, Spec);
}

FNsTweenHandle UNsTweenBlueprintLibrary::PlayTransformTween(const FTransform& StartValue, const FTransform& EndValue, FTransform& Target, const FNsTweenSpec& Spec)
{
    return PlayTypedTween<FTransform, FTweenValue_Transform>(Target, StartValue, EndValue, Spec);
}

FNsTweenHandle UNsTweenBlueprintLibrary::PlayColorTween(const FLinearColor& StartValue, const FLinearColor& EndValue, FLinearColor& Target, const FNsTweenSpec& Spec)
{
    return PlayTypedTween<FLinearColor, FTweenValue_Color>(Target, StartValue, EndValue, Spec);
}

void UNsTweenBlueprintLibrary::PauseTween(FNsTweenHandle Handle)
{
    DispatchToSubsystem(Handle, [](UNsTweenSubsystem& Manager, FNsTweenHandle InHandle)
    {
        Manager.EnqueuePause(InHandle);
    });
}

void UNsTweenBlueprintLibrary::ResumeTween(FNsTweenHandle Handle)
{
    DispatchToSubsystem(Handle, [](UNsTweenSubsystem& Manager, FNsTweenHandle InHandle)
    {
        Manager.EnqueueResume(InHandle);
    });
}

void UNsTweenBlueprintLibrary::CancelTween(FNsTweenHandle Handle, bool bApplyFinal)
{
    DispatchToSubsystem(Handle, [bApplyFinal](UNsTweenSubsystem& Manager, FNsTweenHandle InHandle)
    {
        Manager.EnqueueCancel(InHandle, bApplyFinal);
    });
}

bool UNsTweenBlueprintLibrary::IsTweenActive(FNsTweenHandle Handle)
{
    if (UNsTweenSubsystem* Manager = UNsTweenSubsystem::GetSubsystem())
    {
        return Manager->IsActive(Handle);
    }

    return false;
}
