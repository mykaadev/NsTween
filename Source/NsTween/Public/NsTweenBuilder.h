// Copyright (C) 2025 nulled.softworks. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "NsTweenTypeLibrary.h"

class ITweenValue;
class IEasingCurve;
class UCurveFloat;
class UNsTweenSubsystem;

struct NSTWEEN_API FNsTweenBuilder
{
public:
    /** Constructs an empty builder with no state. */
    FNsTweenBuilder();

    /** Copies another builder instance. */
    FNsTweenBuilder(const FNsTweenBuilder& Other) = default;

    /** Moves another builder instance. */
    FNsTweenBuilder(FNsTweenBuilder&& Other) = default;

    /** Assigns from another builder by copying. */
    FNsTweenBuilder& operator=(const FNsTweenBuilder& Other) = default;

    /** Assigns from another builder by moving. */
    FNsTweenBuilder& operator=(FNsTweenBuilder&& Other) = default;

    /** Destroys the builder, ensuring any state is released. */
    ~FNsTweenBuilder();

    /** Enables or disables ping-pong behaviour on the tween. */
    FNsTweenBuilder& SetPingPong(bool bEnable);

    /** Configures the number of loops the tween should perform. */
    FNsTweenBuilder& SetLoops(int32 LoopCount);

    /** Sets an initial delay before the tween begins playback. */
    FNsTweenBuilder& SetDelay(float DelaySeconds);

    /** Adjusts the time scale applied while the tween plays. */
    FNsTweenBuilder& SetTimeScale(float TimeScale);

    /** Specifies the curve asset used for easing evaluation. */
    FNsTweenBuilder& SetCurveAsset(UCurveFloat* Curve);

    /** Registers a callback executed when the tween completes. */
    FNsTweenBuilder& OnComplete(TFunction<void()> Callback);

    /** Registers a callback executed each time the tween loops. */
    FNsTweenBuilder& OnLoop(TFunction<void()> Callback);

    /** Registers a callback executed on every ping-pong direction swap. */
    FNsTweenBuilder& OnPingPong(TFunction<void()> Callback);

    /** Pauses the tween represented by this builder. */
    void Pause() const;

    /** Resumes the tween represented by this builder. */
    void Resume() const;

    /** Cancels the tween represented by this builder. */
    void Cancel(bool bApplyFinal = true) const;

    /** Returns true when the tween is currently active. */
    bool IsActive() const;

    /** Retrieves the handle associated with the tween. */
    FNsTweenHandle GetHandle() const;

    /** Returns true when the builder contains a valid tween. */
    bool IsValid() const;

    /** Implicit conversion to the underlying tween handle. */
    operator FNsTweenHandle() const
    {
        return GetHandle();
    }

private:
    /** Forward declaration for the shared state backing the builder. */
    struct FState;

    /** Constructs the builder from an explicit state pointer. */
    explicit FNsTweenBuilder(const TSharedPtr<FState>& InState);

    /** Returns true when the tween can still be configured. */
    bool CanConfigure() const;

    /** Activates the tween within the subsystem. */
    void Activate() const;

    /** Updates the wrap mode based on ping-pong and loop settings. */
    void UpdateWrapMode() const;

private:
    /** Shared state describing the tween being built. */
    TSharedPtr<FState> State;

    friend struct FNsTween;
};

struct FNsTweenBuilder::FState : public TSharedFromThis<FState>
{
    /** Specification describing the tween configuration. */
    FNsTweenSpec Spec;

    /** Factory that produces the tween value strategy. */
    TFunction<TSharedPtr<ITweenValue>()> StrategyFactory;

    /** Callback executed on tween completion. */
    TSharedPtr<TFunction<void()>> CompleteCallback;

    /** Callback executed when the tween loops. */
    TSharedPtr<TFunction<void()>> LoopCallback;

    /** Callback executed when the tween ping-pongs. */
    TSharedPtr<TFunction<void()>> PingPongCallback;

    /** Handle used to identify the spawned tween. */
    FNsTweenHandle Handle;

    /** Tracks whether looping is currently enabled. */
    bool bLooping = false;

    /** Tracks whether ping-pong behaviour is enabled. */
    bool bPingPong = false;

    /** Tracks whether the tween has already been activated. */
    bool bActivated = false;
};

