// Copyright (C) 2024 mykaa. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintAsyncActionBase.h"
#include "TweenHandle.h"
#include "TweenSpec.h"
#include "NsTweenAsyncAction.generated.h"

class UNsTweenManagerSubsystem;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FNovaTweenAsyncUpdate, float, NormalizedAlpha);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FNovaTweenAsyncEvent);

UCLASS()
class NSTWEEN_API UNsTweenAsyncAction : public UBlueprintAsyncActionBase
{
    GENERATED_BODY()

public:
    UPROPERTY(BlueprintAssignable)
    FNovaTweenAsyncUpdate OnUpdate;

    UPROPERTY(BlueprintAssignable)
    FNovaTweenAsyncEvent OnComplete;

    UPROPERTY(BlueprintAssignable)
    FNovaTweenAsyncEvent OnLoop;

    UPROPERTY(BlueprintAssignable)
    FNovaTweenAsyncEvent OnPingPong;

    UFUNCTION(BlueprintCallable, meta = (BlueprintInternalUseOnly = "true", WorldContext = "WorldContextObject"))
    static UNsTweenAsyncAction* PlayFloatTween(UObject* WorldContextObject, float StartValue, float EndValue, UPARAM(ref) float& Target, const FNovaTweenSpec& Spec);

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

    FNovaTweenSpec Spec;
    FNovaTweenHandle Handle;
    float StartValue = 0.f;
    float EndValue = 1.f;
    float* TargetPtr = nullptr;
    bool bActivated = false;
};

