// Copyright (C) 2025 nulled.softworks. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "NsTweenTypeLibrary.h"
#include "Templates/Function.h"
#include "Templates/SharedPointer.h"

class ITweenValue;
class IEasingCurve;
class UCurveFloat;
class UNsTweenSubsystem;

struct NSTWEEN_API FNsTweenBuilder
{
public:
    /** Constructs an empty builder with no state. */
    FNsTweenBuilder();

    /** Builders are move-only to keep ownership tight and predictable. */
    FNsTweenBuilder(const FNsTweenBuilder& Other) = delete;

    /** Moves another builder instance. */
    FNsTweenBuilder(FNsTweenBuilder&& Other) noexcept;

    /** Assigns from another builder by copying. */
    FNsTweenBuilder& operator=(const FNsTweenBuilder& Other) = delete;

    /** Assigns from another builder by moving. */
    FNsTweenBuilder& operator=(FNsTweenBuilder&& Other) noexcept;

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

    /** Associates the tween with an owning UObject to gate its lifetime. */
    FNsTweenBuilder& SetOwner(UObject* InOwner);

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
    /** Constructs the builder from explicit specification data. */
    FNsTweenBuilder(FNsTweenSpec&& InSpec, TFunction<TSharedPtr<ITweenValue>()>&& InStrategyFactory);

    /** Activates the tween within the subsystem. */
    void Activate() const;

    /** Updates the wrap mode based on ping-pong and loop settings. */
    void UpdateWrapMode() const;

    /** Returns true when configuration can still be modified. */
    bool CanConfigure() const;

    /** Configures the completion callback on the specification. */
    void ConfigureComplete(TFunction<void()>&& Callback) const;

    /** Configures the loop callback on the specification. */
    void ConfigureLoop(TFunction<void()>&& Callback) const;

    /** Configures the ping-pong callback on the specification. */
    void ConfigurePingPong(TFunction<void()>&& Callback) const;

private:
    /** Specification used when spawning the tween. */
    mutable FNsTweenSpec Spec;

    /** Deferred factory used to create the tween strategy. */
    mutable TFunction<TSharedPtr<ITweenValue>()> StrategyFactory;

    /** Callback storage to keep bound lambdas alive. */
    mutable TSharedPtr<TFunction<void()>> CompleteCallback;

    /** Callback storage to keep bound lambdas alive. */
    mutable TSharedPtr<TFunction<void()>> LoopCallback;

    /** Callback storage to keep bound lambdas alive. */
    mutable TSharedPtr<TFunction<void()>> PingPongCallback;

    /** Handle returned by the subsystem after activation. */
    mutable FNsTweenHandle Handle;

    /** True when the builder has been initialized with a specification. */
    bool bHasSpec = false;

    /** True when the tween loops continuously. */
    mutable bool bLooping = false;

    /** True when the tween ping-pongs instead of looping. */
    mutable bool bPingPong = false;

    /** True once the tween has been activated. */
    mutable bool bActivated = false;

    friend struct FNsTween;
};
