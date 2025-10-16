// Copyright (C) 2024 mykaa. All rights reserved.

#pragma once

#include "Kismet/BlueprintFunctionLibrary.h"
#include "TweenHandle.h"
#include "TweenSpec.h"
#include "TweenBlueprintLibrary.generated.h"

class UNsTweenManagerSubsystem;

UCLASS()
class NSTWEEN_API UNsTweenBlueprintLibrary : public UBlueprintFunctionLibrary
{
    GENERATED_BODY()

public:
    UFUNCTION(BlueprintCallable, Category = "NsTween", meta = (WorldContext = "WorldContextObject"))
    static FNovaTweenHandle PlayFloatTween(UObject* WorldContextObject, float StartValue, float EndValue, UPARAM(ref) float& Target, const FNovaTweenSpec& Spec);

    UFUNCTION(BlueprintCallable, Category = "NsTween", meta = (WorldContext = "WorldContextObject"))
    static FNovaTweenHandle PlayVectorTween(UObject* WorldContextObject, const FVector& StartValue, const FVector& EndValue, UPARAM(ref) FVector& Target, const FNovaTweenSpec& Spec);

    UFUNCTION(BlueprintCallable, Category = "NsTween", meta = (WorldContext = "WorldContextObject"))
    static FNovaTweenHandle PlayRotatorTween(UObject* WorldContextObject, const FRotator& StartValue, const FRotator& EndValue, UPARAM(ref) FRotator& Target, const FNovaTweenSpec& Spec);

    UFUNCTION(BlueprintCallable, Category = "NsTween", meta = (WorldContext = "WorldContextObject"))
    static FNovaTweenHandle PlayTransformTween(UObject* WorldContextObject, const FTransform& StartValue, const FTransform& EndValue, UPARAM(ref) FTransform& Target, const FNovaTweenSpec& Spec);

    UFUNCTION(BlueprintCallable, Category = "NsTween", meta = (WorldContext = "WorldContextObject"))
    static FNovaTweenHandle PlayColorTween(UObject* WorldContextObject, const FLinearColor& StartValue, const FLinearColor& EndValue, UPARAM(ref) FLinearColor& Target, const FNovaTweenSpec& Spec);

    UFUNCTION(BlueprintCallable, Category = "NsTween", meta = (WorldContext = "WorldContextObject"))
    static void PauseTween(UObject* WorldContextObject, FNovaTweenHandle Handle);

    UFUNCTION(BlueprintCallable, Category = "NsTween", meta = (WorldContext = "WorldContextObject"))
    static void ResumeTween(UObject* WorldContextObject, FNovaTweenHandle Handle);

    UFUNCTION(BlueprintCallable, Category = "NsTween", meta = (WorldContext = "WorldContextObject"))
    static void CancelTween(UObject* WorldContextObject, FNovaTweenHandle Handle, bool bApplyFinal = true);

    UFUNCTION(BlueprintCallable, Category = "NsTween", meta = (WorldContext = "WorldContextObject"))
    static bool IsTweenActive(UObject* WorldContextObject, FNovaTweenHandle Handle);

private:
    static UNsTweenManagerSubsystem* GetManager(UObject* WorldContextObject);
};

