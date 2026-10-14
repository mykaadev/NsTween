# NsTween User Guide

## Purpose

NsTween is a small tween library. It moves a value from a start to an end over time with an easing curve. Use it for UI motion and for simple game animation, such as a floating pickup, a door, or a color pulse.

NsTween does not replace UMG animations for authored UI sequences, or Sequencer for cutscenes.

## Setup

1. Add `NsTween` to the `Plugins` list of the `.uproject` file.
2. For C++, add `"NsTween"` to the module dependencies in `Build.cs`.

## Quick Start

### C++

```cpp
#include "NsTween.h"

FNsTween::Play(0.f, 100.f, 2.f, ENsTweenEase::InOutQuad, [this](const float Value)
{
    SetActorLocation(FVector(Value, 0.f, 0.f));
})
.SetOwner(this);
```

`Play` returns a builder. The tween starts when the builder is destroyed, or when you call a control function on it. Chain the settings in the same statement.

### Blueprint

Use an async node, for example `Tween Float`. Connect `Apply Easing` to the code that uses the value. `On Complete` fires at the end.

## Common Tasks

### Loop or ping-pong

```cpp
FNsTween::Play(StartZ, StartZ + 40.f, 1.f, ENsTweenEase::InOutSine, [this](const float Z) { SetHoverZ(Z); })
    .SetPingPong(true)
    .SetLoops(-1)
    .SetOwner(this);
```

`SetLoops(-1)` loops forever.

### Control a tween

Keep the handle, then pause, resume, or cancel it.

```cpp
FNsTweenHandle Handle = FNsTween::Play(0.f, 1.f, 0.3f, ENsTweenEase::OutCubic, [this](const float Alpha) { SetAlpha(Alpha); });
UNsTweenBlueprintLibrary::CancelTween(Handle, true);
```

In Blueprint, use `Pause Tween`, `Resume Tween`, `Cancel Tween`, and `Is Tween Active`.

### Use a custom curve

- Call `SetCurveAsset(Curve)` on the builder, or use a "Custom Curve" async node.
- For a Bezier curve, set `EasingPreset` to `CustomBezier` and set `BezierControlPoints` on a `FNsTweenSpec`.

### Run callbacks

Use `OnComplete`, `OnLoop`, and `OnPingPong` on the builder.

## Builder Calls

| Call | Effect |
|---|---|
| `SetPingPong(bool)` | Plays forward, then backward. |
| `SetLoops(int32)` | Number of loops. -1 means forever. |
| `SetDelay(float)` | Delay before the start, in seconds. |
| `SetTimeScale(float)` | Speed multiplier. |
| `SetCurveAsset(UCurveFloat*)` | Easing from a curve asset. |
| `SetOwner(UObject*)` | Stops the tween when the owner is destroyed. |
| `OnComplete`, `OnLoop`, `OnPingPong` | Callbacks. |
| `Pause`, `Resume`, `Cancel`, `IsActive`, `GetHandle` | Control. |

## Value Types

| Type | C++ `Play` | Blueprint |
|---|---|---|
| `float` | Yes | `Tween Float`, `Play Float Tween` |
| `FVector` | Yes | `Tween Vector`, `Play Vector Tween` |
| `FVector2D` | Yes | `Tween Vector 2D` |
| `FRotator` | Yes | `Tween Rotator`, `Play Rotator Tween` |
| `FQuat` | Yes | `Tween Quat` |
| `FTransform` | Yes | `Tween Transform`, `Play Transform Tween` |
| `FLinearColor` | Yes | `Tween Linear Color`, `Play Color Tween` |

## Easing Presets

`ENsTweenEase`: `Linear`, then `In`, `Out`, and `InOut` versions of `Sine`, `Quad`, `Cubic`, `Quart`, `Quint`, `Expo`, `Circ`, `Elastic`, `Bounce`, and `Back`. Also `CustomBezier` and `CurveAsset`.

## Rules

- Call `SetOwner` for every tween that touches an object. Without an owner, the tween keeps running after the object is gone.
- Do not keep a builder in a variable. Keep the `FNsTweenHandle` instead.
- To spin a full circle, tween a float yaw. A rotator tween takes the shortest path, so 0 to 360 degrees does not move.

## Settings

NsTween has no project settings.

## Debug

- Log category: `LogNsTween`.
- Stat counters wrap the main functions. Use Unreal Insights to see the tween cost.

## Limits

- Tweens run on the engine core ticker with real time. They keep running while the game is paused, and they ignore time dilation and the world time.
- The async nodes have `bCanTickDuringPause`, `bUseGlobalTimeDilation`, `LoopDelay`, and `PingPongDelay` inputs. NsTween ignores them and logs a warning.
- Tweens do not tick outside a game world, for example in the editor without PIE.
- Color tweens in C++ blend in HSV.
- `UNsTweenSequence` holds a list of specs, but no code plays a sequence today.
