// Copyright (C) 2025 nulled.softworks. All rights reserved.

#pragma once

#include "Kismet/BlueprintFunctionLibrary.h"
#include "NsTween.h"
#include "NsTweenBuilder.h"
#include "NsTweenSubsystem.h"
#include "NsTweenTypeLibrary.h"
#include "NsTweenFunctionLibrary.generated.h"

class UNsTweenSubsystem;

/**
 * Helper that routes every Blueprint spawn call through the same builder logic.
 * This guarantees parity with the C++ helpers and keeps the redundant boilerplate in one place.
 */
template <typename TValue, typename TStrategy>
FNsTweenHandle PlayTypedTween(TValue& Target, const TValue& StartValue, const TValue& EndValue, const FNsTweenSpec& Spec)
{
    const FNsTweenBuilder Builder = FNsTween::Play(Spec, [&Target, StartValue, EndValue]() -> TSharedPtr<ITweenValue>
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

/** Blueprint convenience entry points for spawning and controlling tweens. */
UCLASS()
class NSTWEEN_API UNsTweenBlueprintLibrary : public UBlueprintFunctionLibrary
{
    GENERATED_BODY()

public:
    /** Plays a float tween that writes the interpolated value back to the provided reference. */
    UFUNCTION(BlueprintCallable, Category = "NsTween", meta = (WorldContext = "WorldContextObject"))
    static FNsTweenHandle PlayFloatTween(float StartValue, float EndValue, UPARAM(ref) float& Target, const FNsTweenSpec& Spec);

    /** Plays a vector tween that writes the interpolated value back to the provided reference. */
    UFUNCTION(BlueprintCallable, Category = "NsTween", meta = (WorldContext = "WorldContextObject"))
    static FNsTweenHandle PlayVectorTween(const FVector& StartValue, const FVector& EndValue, UPARAM(ref) FVector& Target, const FNsTweenSpec& Spec);

    /** Plays a rotator tween that writes the interpolated value back to the provided reference. */
    UFUNCTION(BlueprintCallable, Category = "NsTween", meta = (WorldContext = "WorldContextObject"))
    static FNsTweenHandle PlayRotatorTween(const FRotator& StartValue, const FRotator& EndValue, UPARAM(ref) FRotator& Target, const FNsTweenSpec& Spec);

    /** Plays a transform tween that writes the interpolated value back to the provided reference. */
    UFUNCTION(BlueprintCallable, Category = "NsTween", meta = (WorldContext = "WorldContextObject"))
    static FNsTweenHandle PlayTransformTween(const FTransform& StartValue, const FTransform& EndValue, UPARAM(ref) FTransform& Target, const FNsTweenSpec& Spec);

    /** Plays a color tween that writes the interpolated value back to the provided reference. */
    UFUNCTION(BlueprintCallable, Category = "NsTween", meta = (WorldContext = "WorldContextObject"))
    static FNsTweenHandle PlayColorTween(const FLinearColor& StartValue, const FLinearColor& EndValue, UPARAM(ref) FLinearColor& Target, const FNsTweenSpec& Spec);

    /** Pauses the tween represented by the supplied handle. */
    UFUNCTION(BlueprintCallable, Category = "NsTween", meta = (WorldContext = "WorldContextObject"))
    static void PauseTween(FNsTweenHandle Handle);

    /** Resumes the tween represented by the supplied handle. */
    UFUNCTION(BlueprintCallable, Category = "NsTween", meta = (WorldContext = "WorldContextObject"))
    static void ResumeTween(FNsTweenHandle Handle);

    /** Cancels the tween represented by the supplied handle. */
    UFUNCTION(BlueprintCallable, Category = "NsTween", meta = (WorldContext = "WorldContextObject"))
    static void CancelTween(FNsTweenHandle Handle, bool bApplyFinal = true);

    /** Returns true when the tween represented by the supplied handle is currently active. */
    UFUNCTION(BlueprintCallable, Category = "NsTween", meta = (WorldContext = "WorldContextObject"))
    static bool IsTweenActive(FNsTweenHandle Handle);
};

