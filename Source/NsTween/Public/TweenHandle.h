// Copyright (C) 2024 mykaa. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "TweenId.h"
#include "TweenHandle.generated.h"

USTRUCT(BlueprintType)
struct NSTWEEN_API FNovaTweenHandle
{
    GENERATED_BODY()

public:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tween")
    FNovaTweenId Id;

    bool IsValid() const { return Id.IsValid(); }
    void Reset() { Id.Reset(); }
};

