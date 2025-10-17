// Copyright (C) 2024 mykaa. All rights reserved.

#pragma once

#include "Kismet/BlueprintFunctionLibrary.h"
#include "NsTweenTypeLibrary.h"
#include "NsTweenFunctionLibrary.generated.h"

class UNsTweenSubsystem;

UCLASS()
class NSTWEEN_API UNsTweenBlueprintLibrary : public UBlueprintFunctionLibrary
{
    GENERATED_BODY()

public:

    UFUNCTION(BlueprintCallable, Category = "NsTween", meta = (WorldContext = "WorldContextObject"))
    static FNsTweenHandle PlayFloatTween(float StartValue, float EndValue, UPARAM(ref) float& Target, const FNsTweenSpec& Spec);

    UFUNCTION(BlueprintCallable, Category = "NsTween", meta = (WorldContext = "WorldContextObject"))
    static FNsTweenHandle PlayVectorTween(const FVector& StartValue, const FVector& EndValue, UPARAM(ref) FVector& Target, const FNsTweenSpec& Spec);

    UFUNCTION(BlueprintCallable, Category = "NsTween", meta = (WorldContext = "WorldContextObject"))
    static FNsTweenHandle PlayRotatorTween(const FRotator& StartValue, const FRotator& EndValue, UPARAM(ref) FRotator& Target, const FNsTweenSpec& Spec);

    UFUNCTION(BlueprintCallable, Category = "NsTween", meta = (WorldContext = "WorldContextObject"))
    static FNsTweenHandle PlayTransformTween(const FTransform& StartValue, const FTransform& EndValue, UPARAM(ref) FTransform& Target, const FNsTweenSpec& Spec);

    UFUNCTION(BlueprintCallable, Category = "NsTween", meta = (WorldContext = "WorldContextObject"))
    static FNsTweenHandle PlayColorTween(const FLinearColor& StartValue, const FLinearColor& EndValue, UPARAM(ref) FLinearColor& Target, const FNsTweenSpec& Spec);

    UFUNCTION(BlueprintCallable, Category = "NsTween", meta = (WorldContext = "WorldContextObject"))
    static void PauseTween(FNsTweenHandle Handle);

    UFUNCTION(BlueprintCallable, Category = "NsTween", meta = (WorldContext = "WorldContextObject"))
    static void ResumeTween(FNsTweenHandle Handle);

    UFUNCTION(BlueprintCallable, Category = "NsTween", meta = (WorldContext = "WorldContextObject"))
    static void CancelTween(FNsTweenHandle Handle, bool bApplyFinal = true);

    UFUNCTION(BlueprintCallable, Category = "NsTween", meta = (WorldContext = "WorldContextObject"))
    static bool IsTweenActive(FNsTweenHandle Handle);
};

