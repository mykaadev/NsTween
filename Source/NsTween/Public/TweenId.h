// Copyright (C) 2024 mykaa. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "TweenId.generated.h"

USTRUCT(BlueprintType)
struct NSTWEEN_API FNovaTweenId
{
    GENERATED_BODY()

public:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tween")
    uint32 Value = 0;

    bool IsValid() const { return Value != 0; }
    void Reset() { Value = 0; }

    bool operator==(const FNovaTweenId& Other) const { return Value == Other.Value; }
    bool operator!=(const FNovaTweenId& Other) const { return !(*this == Other); }
};

FORCEINLINE uint32 GetTypeHash(const FNovaTweenId& InId)
{
    return ::GetTypeHash(InId.Value);
}

