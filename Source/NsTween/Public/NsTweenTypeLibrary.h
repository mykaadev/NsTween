// Copyright (C) 2024 mykaa. All rights reserved.

#pragma once

#include "Delegates/DelegateCombinations.h"
#include "Interfaces/ITweenValue.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "NsTweenTypeLibrary.generated.h"

DECLARE_DELEGATE_OneParam(FNsTweenOnUpdate, float /*NormalizedAlpha*/);
DECLARE_DELEGATE(FNsTweenOnComplete);
DECLARE_DELEGATE(FNsTweenOnLoop);
DECLARE_DELEGATE(FNsTweenOnPingPong);

enum class ENsTweenCommandType : uint8
{
    Spawn,
    Cancel,
    Pause,
    Resume
};

UENUM(BlueprintType)
enum class ENsTweenDirection : uint8
{
    Forward,
    Backward
};

UENUM(BlueprintType)
enum class ENsTweenWrapMode : uint8
{
    Once,
    Loop,
    PingPong
};

UENUM(BlueprintType)
enum class ENsTweenEase : uint8
{
    Linear,
    InSine,
    OutSine,
    InOutSine,
    InQuad,
    OutQuad,
    InOutQuad,
    InCubic,
    OutCubic,
    InOutCubic,
    InExpo,
    OutExpo,
    InOutExpo,
    CustomBezier,
    CurveAsset
};


USTRUCT(BlueprintType)
struct NSTWEEN_API FNsTweenSpec
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
    ENsTweenWrapMode WrapMode = ENsTweenWrapMode::Once;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tween")
    int32 LoopCount = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tween")
    ENsTweenDirection Direction = ENsTweenDirection::Forward;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tween")
    ENsTweenEase EasingPreset = ENsTweenEase::InOutSine;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tween")
    FVector4 BezierControlPoints = FVector4(0.25f, 0.1f, 0.25f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tween")
    TWeakObjectPtr<UCurveFloat> CurveAsset = nullptr;

    FNsTweenOnUpdate OnUpdate;

    FNsTweenOnComplete OnComplete;

    FNsTweenOnLoop OnLoop;

    FNsTweenOnPingPong OnPingPong;

    void* UserData = nullptr;
};


USTRUCT(BlueprintType)
struct NSTWEEN_API FNsTweenSequenceItem
{
    GENERATED_BODY()

public:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tween")
    FNsTweenSpec Spec;
};

UCLASS(BlueprintType)
class NSTWEEN_API UNsTweenSequence : public UObject
{
    GENERATED_BODY()

public:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tween")
    TArray<FNsTweenSequenceItem> Items;
};

USTRUCT(BlueprintType)
struct NSTWEEN_API FNsTweenId
{
    GENERATED_BODY()

public:

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tween")
    int32 Value = 0;

    bool IsValid() const { return Value != 0; }
    void Reset() { Value = 0; }

    bool operator==(const FNsTweenId& Other) const { return Value == Other.Value; }
    bool operator!=(const FNsTweenId& Other) const { return !(*this == Other); }
};


USTRUCT(BlueprintType)
struct NSTWEEN_API FNsTweenHandle
{
    GENERATED_BODY()

public:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tween")
    FNsTweenId Id;

    bool IsValid() const { return Id.IsValid(); }
    void Reset() { Id.Reset(); }
};

struct FNsTweenCommand
{
    ENsTweenCommandType Type = ENsTweenCommandType::Spawn;
    FNsTweenHandle Handle;
    FNsTweenSpec Spec;
    TSharedPtr<ITweenValue> Strategy;
    bool bApplyFinalOnCancel = true;
};

UCLASS()
class NSTWEEN_API UNsTweenTypeLibrary : public UBlueprintFunctionLibrary
{
    GENERATED_BODY()

    FORCEINLINE static uint32 GetTypeHash(const FNsTweenId& InId)
    {
        return ::GetTypeHash(InId.Value);
    }
};



