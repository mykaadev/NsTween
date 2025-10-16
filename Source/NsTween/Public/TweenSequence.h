// Copyright (C) 2024 mykaa. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "TweenHandle.h"
#include "TweenSpec.h"
#include "TweenSequence.generated.h"

USTRUCT(BlueprintType)
struct NSTWEEN_API FNovaTweenSequenceItem
{
    GENERATED_BODY()

public:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tween")
    FNovaTweenSpec Spec;
};

UCLASS(BlueprintType)
class NSTWEEN_API UNovaTweenSequence : public UObject
{
    GENERATED_BODY()

public:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tween")
    TArray<FNovaTweenSequenceItem> Items;
};

