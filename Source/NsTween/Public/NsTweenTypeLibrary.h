// Copyright (C) 2025 nulled.softworks. All rights reserved.

#pragma once

#include "Delegates/DelegateCombinations.h"
#include "Interfaces/ITweenValue.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "NsTweenTypeLibrary.generated.h"

/** Delegate fired every tick of a tween with the normalized alpha. */
DECLARE_DELEGATE_OneParam(FNsTweenOnUpdate, float /*NormalizedAlpha*/);

/** Delegate fired when a tween completes. */
DECLARE_DELEGATE(FNsTweenOnComplete);

/** Delegate fired when a tween loops. */
DECLARE_DELEGATE(FNsTweenOnLoop);

/** Delegate fired when a tween ping-pongs. */
DECLARE_DELEGATE(FNsTweenOnPingPong);

/** Command types supported by the tween manager. */
enum class ENsTweenCommandType : uint8
{
    Spawn,
    Cancel,
    Pause,
    Resume
};

/** Direction in which the tween should initially play. */
UENUM(BlueprintType)
enum class ENsTweenDirection : uint8
{
    Forward,
    Backward
};

/** Wrap modes controlling tween repetition. */
UENUM(BlueprintType)
enum class ENsTweenWrapMode : uint8
{
    Once,
    Loop,
    PingPong
};

/** Easing presets that can be applied to tweens. */
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
    InQuart,
    OutQuart,
    InOutQuart,
    InQuint,
    OutQuint,
    InOutQuint,
    InExpo,
    OutExpo,
    InOutExpo,
    InCirc,
    OutCirc,
    InOutCirc,
    InElastic,
    OutElastic,
    InOutElastic,
    InBounce,
    OutBounce,
    InOutBounce,
    InBack,
    OutBack,
    InOutBack,
    CustomBezier,
    CurveAsset
};

/** Describes the configuration required to spawn a tween. */
USTRUCT(BlueprintType)
struct NSTWEEN_API FNsTweenSpec
{
    GENERATED_BODY()

public:
    /** Duration of the tween in seconds. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tween")
    float DurationSeconds = 0.5f;

    /** Initial delay applied before the tween starts. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tween")
    float DelaySeconds = 0.0f;

    /** Time scale applied while the tween plays. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tween")
    float TimeScale = 1.0f;

    /** Wrap mode defining how the tween repeats. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tween")
    ENsTweenWrapMode WrapMode = ENsTweenWrapMode::Once;

    /** Number of loops or ping-pong pairs to execute. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tween")
    int32 LoopCount = 0;

    /** Direction in which the tween initially plays. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tween")
    ENsTweenDirection Direction = ENsTweenDirection::Forward;

    /** Selected easing preset used to evaluate the tween. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tween")
    ENsTweenEase EasingPreset = ENsTweenEase::InOutSine;

    /** Control points for custom bezier easing. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tween")
    FVector4 BezierControlPoints = FVector4(0.25f, 0.1f, 0.25f, 1.0f);

    /** Curve asset used when the easing preset is curve-based. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tween")
    TWeakObjectPtr<UCurveFloat> CurveAsset = nullptr;

    /** Callback executed every tick of the tween. */
    FNsTweenOnUpdate OnUpdate;

    /** Callback executed when the tween completes. */
    FNsTweenOnComplete OnComplete;

    /** Callback executed when the tween loops. */
    FNsTweenOnLoop OnLoop;

    /** Callback executed when the tween ping-pongs. */
    FNsTweenOnPingPong OnPingPong;

    /** Pointer for user-defined context data. */
    void* UserData = nullptr;
};

/** A single entry within a tween sequence asset. */
USTRUCT(BlueprintType)
struct NSTWEEN_API FNsTweenSequenceItem
{
    GENERATED_BODY()

public:
    /** Specification describing the tween contained in this entry. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tween")
    FNsTweenSpec Spec;
};

/** UObject wrapper for configuring tween sequences. */
UCLASS(BlueprintType)
class NSTWEEN_API UNsTweenSequence : public UObject
{
    GENERATED_BODY()

public:
    /** Collection of sequence items executed sequentially. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tween")
    TArray<FNsTweenSequenceItem> Items;
};

/** Identifier assigned to spawned tweens. */
USTRUCT(BlueprintType)
struct NSTWEEN_API FNsTweenId
{
    GENERATED_BODY()

public:
    /** Raw integer value of the identifier. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tween")
    int32 Value = 0;

    /** Returns true when the identifier contains a non-zero value. */
    bool IsValid() const { return Value != 0; }

    /** Resets the identifier to an invalid state. */
    void Reset() { Value = 0; }

    /** Compares two identifiers for equality. */
    bool operator==(const FNsTweenId& Other) const { return Value == Other.Value; }

    /** Compares two identifiers for inequality. */
    bool operator!=(const FNsTweenId& Other) const { return !(*this == Other); }
};

/** Handle returned to callers when a tween is spawned. */
USTRUCT(BlueprintType)
struct NSTWEEN_API FNsTweenHandle
{
    GENERATED_BODY()

public:
    /** Identifier representing the spawned tween. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tween")
    FNsTweenId Id;

    /** Returns true when the handle references a valid tween. */
    bool IsValid() const { return Id.IsValid(); }

    /** Resets the handle to an invalid state. */
    void Reset() { Id.Reset(); }
};

/** Command data consumed by the tween manager. */
struct FNsTweenCommand
{
    /** Type of command being issued. */
    ENsTweenCommandType Type = ENsTweenCommandType::Spawn;

    /** Handle used to identify the target tween. */
    FNsTweenHandle Handle;

    /** Specification payload associated with the command. */
    FNsTweenSpec Spec;

    /** Strategy factory used when spawning tweens. */
    TSharedPtr<ITweenValue> Strategy;

    /** Tracks whether the final value should be applied on cancel. */
    bool bApplyFinalOnCancel = true;
};

/** Blueprint function library that exposes NsTween helpers. */
UCLASS()
class NSTWEEN_API UNsTweenTypeLibrary : public UBlueprintFunctionLibrary
{
    GENERATED_BODY()

public:
    /** Returns a hash for the provided tween identifier. */
    FORCEINLINE static uint32 GetTypeHash(const FNsTweenId& InId)
    {
        return ::GetTypeHash(InId.Value);
    }
};

