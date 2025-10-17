// Copyright (C) 2024 mykaa. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintAsyncActionBase.h"
#include "NsTweenTypeLibrary.h"
#include "NsTweenAsyncAction.generated.h"

class UNsTweenSubsystem;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FNsTweenAsyncUpdate, float, NormalizedAlpha);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FNsTweenAsyncEvent);

UCLASS()
class NSTWEEN_API UNsTweenAsyncAction : public UBlueprintAsyncActionBase
{
    GENERATED_BODY()

public:
    UPROPERTY(BlueprintAssignable)
    FNsTweenAsyncUpdate OnUpdate;

    UPROPERTY(BlueprintAssignable)
    FNsTweenAsyncEvent OnComplete;

    UPROPERTY(BlueprintAssignable)
    FNsTweenAsyncEvent OnLoop;

    UPROPERTY(BlueprintAssignable)
    FNsTweenAsyncEvent OnPingPong;

    UFUNCTION(BlueprintCallable, meta = (BlueprintInternalUseOnly = "true", WorldContext = "WorldContextObject"))
    static UNsTweenAsyncAction* PlayFloatTween(UObject* WorldContextObject, float StartValue, float EndValue, UPARAM(ref) float& Target, const FNsTweenSpec& Spec);

    void Activate() override;
    virtual void SetReadyToDestroy() override;

private:
    void BindDelegates();

    UFUNCTION()
    void HandleUpdate(float NormalizedAlpha);

    UFUNCTION()
    void HandleComplete();

    UFUNCTION()
    void HandleLoop();

    UFUNCTION()
    void HandlePingPong();

private:
    UPROPERTY()
    UObject* WorldContext = nullptr;

    FNsTweenSpec Spec;
    FNsTweenHandle Handle;
    float StartValue = 0.f;
    float EndValue = 1.f;
    float* TargetPtr = nullptr;
    bool bActivated = false;
};

