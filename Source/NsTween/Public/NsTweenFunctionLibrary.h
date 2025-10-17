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
    static FNsTweenHandle PlayFloatTween(UObject* WorldContextObject, float StartValue, float EndValue, UPARAM(ref) float& Target, const FNsTweenSpec& Spec);

    UFUNCTION(BlueprintCallable, Category = "NsTween", meta = (WorldContext = "WorldContextObject"))
    static FNsTweenHandle PlayVectorTween(UObject* WorldContextObject, const FVector& StartValue, const FVector& EndValue, UPARAM(ref) FVector& Target, const FNsTweenSpec& Spec);

    UFUNCTION(BlueprintCallable, Category = "NsTween", meta = (WorldContext = "WorldContextObject"))
    static FNsTweenHandle PlayRotatorTween(UObject* WorldContextObject, const FRotator& StartValue, const FRotator& EndValue, UPARAM(ref) FRotator& Target, const FNsTweenSpec& Spec);

    UFUNCTION(BlueprintCallable, Category = "NsTween", meta = (WorldContext = "WorldContextObject"))
    static FNsTweenHandle PlayTransformTween(UObject* WorldContextObject, const FTransform& StartValue, const FTransform& EndValue, UPARAM(ref) FTransform& Target, const FNsTweenSpec& Spec);

    UFUNCTION(BlueprintCallable, Category = "NsTween", meta = (WorldContext = "WorldContextObject"))
    static FNsTweenHandle PlayColorTween(UObject* WorldContextObject, const FLinearColor& StartValue, const FLinearColor& EndValue, UPARAM(ref) FLinearColor& Target, const FNsTweenSpec& Spec);

    UFUNCTION(BlueprintCallable, Category = "NsTween", meta = (WorldContext = "WorldContextObject"))
    static void PauseTween(UObject* WorldContextObject, FNsTweenHandle Handle);

    UFUNCTION(BlueprintCallable, Category = "NsTween", meta = (WorldContext = "WorldContextObject"))
    static void ResumeTween(UObject* WorldContextObject, FNsTweenHandle Handle);

    UFUNCTION(BlueprintCallable, Category = "NsTween", meta = (WorldContext = "WorldContextObject"))
    static void CancelTween(UObject* WorldContextObject, FNsTweenHandle Handle, bool bApplyFinal = true);

    UFUNCTION(BlueprintCallable, Category = "NsTween", meta = (WorldContext = "WorldContextObject"))
    static bool IsTweenActive(UObject* WorldContextObject, FNsTweenHandle Handle);

private:
    static UNsTweenSubsystem* GetManager(UObject* WorldContextObject);
};

