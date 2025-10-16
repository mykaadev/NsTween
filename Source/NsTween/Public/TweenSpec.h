// Copyright (C) 2024 mykaa. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "TweenDelegates.h"
#include "TweenEnums.h"
#include "TweenSpec.generated.h"

class UCurveFloat;

USTRUCT(BlueprintType)
struct NSTWEEN_API FNovaTweenSpec
{
    GENERATED_BODY()

public:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tween")
    float DurationSeconds = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tween")
    float DelaySeconds = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tween")
    float TimeScale = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tween")
    ENovaTweenWrapMode WrapMode = ENovaTweenWrapMode::Once;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tween")
    int32 LoopCount = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tween")
    ENovaTweenDirection Direction = ENovaTweenDirection::Forward;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tween")
    ENovaEasingPreset EasingPreset = ENovaEasingPreset::EaseInOutSine;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tween")
    FVector4 BezierControlPoints = FVector4(0.25f, 0.1f, 0.25f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tween")
    TWeakObjectPtr<UCurveFloat> CurveAsset = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tween")
    FNovaTweenUpdate OnUpdate;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tween")
    FNovaTweenComplete OnComplete;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tween")
    FNovaTweenLoop OnLoop;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tween")
    FNovaTweenPingPong OnPingPong;

    void* UserData = nullptr;
};

