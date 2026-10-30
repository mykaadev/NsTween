# NsTween User Guide

## Purpose

NsTween moves a value from a start to an end over time, with an easing curve. Use it for UI motion and for small game motion, such as a floating pickup, a door, or a color pulse. You start a tween from C++ with `FNsTween::Play`, or from Blueprint with an async node.

NsTween is not a replacement for UMG animations or for Sequencer. It does not replicate, and it does not follow the game pause or the time dilation. Tweens run on real time.

## Setup

1. Copy the `NsTween` folder into the `Plugins` folder of your project.
2. Generate the project files.
3. Enable `NsTween` in **Edit > Plugins**.
4. For C++, add `"NsTween"` to the dependencies of your game module in its `Build.cs` file.

The plugin needs no project settings. The module builds only for Win64.

## Quick Start

### Blueprint

1. Open an Actor Blueprint.
2. From `Event BeginPlay`, add the `Tween Float` node.
3. Set **Start** to 0, **End** to 100, and **Duration Secs** to 2.
4. Set **Ease Type** to `InOutQuad`.
5. From **Apply Easing**, add `Set Actor Location`, and use **Value** as X.
6. Press Play. The actor moves 100 units on X in 2 seconds.

### C++

```cpp
#include "NsTween.h"

void AMyActor::BeginPlay()
{
    Super::BeginPlay();

    const FVector Start = GetActorLocation();
    FNsTween::Play(0.f, 100.f, 2.f, ENsTweenEase::InOutQuad, [this, Start](float Value)
    {
        SetActorLocation(Start + FVector(Value, 0.f, 0.f));
    })
    .SetOwner(this);
}
```

`Play` returns a builder. Chain the options in the same statement. The tween starts when the builder is destroyed, or when you call `Pause`, `Resume`, `Cancel`, `IsActive`, `GetHandle`, or `IsValid` on it. The start and the end must have the same type.

## Common Tasks

### Pick a node for a value type

| Type | C++ | Async node | Spec node |
|---|---|---|---|
| Float | `FNsTween::Play` | `Tween Float` | `Play Float Tween` |
| Vector | `FNsTween::Play` | `Tween Vector` | `Play Vector Tween` |
| Vector 2D | `FNsTween::Play` | `Tween Vector 2D` | None |
| Rotator | `FNsTween::Play` | `Tween Rotator` | `Play Rotator Tween` |
| Quat | `FNsTween::Play` | `Tween Quat`, `Tween Quat From Rotator` | None |
| Transform | `FNsTween::Play` | `Tween Transform` | `Play Transform Tween` |
| Linear Color | `FNsTween::Play` | `Tween Linear Color` | `Play Color Tween` |

Each async node also has a `Custom Curve` version, for example `Tween Float Custom Curve`.

### Loop a tween

In Blueprint:

1. Open the advanced pins of the async node.
2. Set **Loops** to the number of loops.

In C++:

1. Call `SetLoops(Count)` on the builder, with a count above 0.

`On Loop` (`OnLoop` in C++) fires at the end of each loop, the last loop included. A count of 0 or less turns looping off. To loop with no end, use ping-pong with no count, or an `FNsTweenSpec` with **Wrap Mode** `Loop` and **Loop Count** 0.

### Ping-pong a tween

In Blueprint:

1. Open the advanced pins of the async node.
2. Turn on **Ping Pong**.
3. Set **Loops** to the number of forward-and-back pairs, or leave it at 0 for no end.

In C++:

```cpp
const float StartZ = GetActorLocation().Z;
FNsTween::Play(StartZ, StartZ + 40.f, 1.f, ENsTweenEase::InOutSine, [this](float Z)
{
    FVector Location = GetActorLocation();
    Location.Z = Z;
    SetActorLocation(Location);
})
.SetPingPong(true)
.SetOwner(this);
```

`On Ping Pong` (`OnPingPong` in C++) fires at each change of direction.

### Spin a full circle

1. Tween a float from 0 to 360.
2. Use the float as the yaw of a new rotator in the update.

A Rotator tween takes the shortest path, so 0 to 360 degrees does not turn.

### Use a custom ease curve

In Blueprint:

1. Make a Float Curve asset with keys from time 0 to time 1.
2. Use a `Custom Curve` node, for example `Tween Float Custom Curve`.
3. Set **Curve** to your asset.

In C++:

1. Call `SetCurveAsset(Curve)` on the builder.

The tween reads the curve at a time from 0 to 1, and uses the curve value as the alpha.

### Use a Bezier ease

1. Make an `FNsTweenSpec`. In Blueprint, drag from a **Spec** pin and add a Make node.
2. Set **Easing Preset** to `CustomBezier`.
3. Set **Bezier Control Points**. X and Y are the first control point, Z and W are the second.
4. Pass the spec to a `Play ... Tween` node, or to `FNsTween::Play(Spec, Factory)` in C++.

### Tween a variable with a spec

1. Add a member variable to your Blueprint, for example a float.
2. Add `Play Float Tween`.
3. Connect the variable to **Target**.
4. Make a spec for **Spec**, and set **Owner** to Self.
5. Keep the returned handle in a variable.
6. Read the variable where you use it, for example in `Event Tick`.

The tween writes the value into **Target** on each tick. It sends no events.

### Pause, resume, or cancel a tween

In Blueprint, with a handle from a `Play ... Tween` node:

1. Call `Pause Tween`, `Resume Tween`, or `Cancel Tween` with the handle.

With an async node, call `Pause`, `Resume`, `Stop`, or `Restart` on the async action object that the node returns.

In C++:

```cpp
#include "NsTweenFunctionLibrary.h"

const FNsTweenHandle Handle = FNsTween::Play(0.f, 1.f, 0.3f, ENsTweenEase::OutCubic, [this](float Alpha) { SetAlpha(Alpha); });
UNsTweenBlueprintLibrary::CancelTween(Handle, true);
```

`Cancel Tween` applies the end value when **Apply Final** is true. A cancel also fires `On Complete`. These commands take effect at the next tick of the tween subsystem.

### Stop a tween with its object

1. In C++, call `SetOwner(this)` on the builder.
2. In a spec, set **Owner**.

The tween stops when the owner is destroyed. It does not fire `On Complete` then. The async nodes set no owner. Call `Stop` on the async action in `Event EndPlay` if the tween must stop with the actor.

### Run code at the end

1. In C++, call `OnComplete`, `OnLoop`, or `OnPingPong` on the builder with a function.
2. In Blueprint, connect **On Complete**, **On Loop**, or **On Ping Pong** on the async node.

## Settings

### Async nodes

| Setting | Default | Effect |
|---|---|---|
| **Start**, **End** | Float: 0 and 1. Vector, Vector 2D, Rotator: zero. Linear Color: black and white | The values of the tween. Quat and Transform have no default |
| **Duration Secs** | 1 | The time of one pass. A value of 0 or less gives an error |
| **Ease Type** | `InOutQuad` | The easing preset. The `Custom Curve` nodes have **Curve** here |
| **Curve** | None | The Float Curve of a `Custom Curve` node. None gives a linear ease |
| **Delay** | 0 | The wait before the first pass, in seconds |
| **Loops** | 0 | The number of loops, or of ping-pong pairs. 0 or less: no loop |
| **Ping Pong** | False | Play forward, then backward |
| **Loop Delay** | 0 | No effect. A value above 0 logs a warning |
| **Ping Pong Delay** | 0 | No effect. A value above 0 logs a warning |
| **Can Tick During Pause** | False | No effect. True logs a warning |
| **Use Global Time Dilation** | True | No effect. False logs a warning |

**Delay** and all pins after it are advanced pins.

### Tween spec (`FNsTweenSpec`)

| Setting | Default | Effect |
|---|---|---|
| **Duration Seconds** | 0.5 | The time of one pass |
| **Delay Seconds** | 0 | The wait before the first pass. A negative value counts as 0 |
| **Time Scale** | 1 | The speed multiplier. A negative value counts as 0, and 0 stops the tween from moving |
| **Wrap Mode** | `Once` | `Once`, `Loop`, or `PingPong`. A **Loop Count** above 0 turns `Once` into `Loop` |
| **Loop Count** | 0 | The number of loops, or of ping-pong pairs. 0 with `Loop` or `PingPong`: no end |
| **Direction** | `Forward` | `Backward` starts at the end value and plays to the start value |
| **Easing Preset** | `InOutSine` | The easing preset. `CustomBezier` uses **Bezier Control Points**. `CurveAsset` uses **Curve Asset** |
| **Bezier Control Points** | (0.25, 0.1, 0.25, 1.0) | The two inner points of the cubic Bezier: X, Y, then Z, W |
| **Curve Asset** | None | The Float Curve for `CurveAsset`. None gives a linear ease |
| **Owner** | None | The tween stops when this object is destroyed |

### Builder calls (C++)

| Setting | Default | Effect |
|---|---|---|
| `SetLoops(int32)` | 0 | The number of loops, or of ping-pong pairs. 0 or less turns looping off |
| `SetPingPong(bool)` | False | Play forward, then backward |
| `SetDelay(float)` | 0 | The wait before the first pass, in seconds |
| `SetTimeScale(float)` | 1 | The speed multiplier. The minimum is 0.0001 |
| `SetCurveAsset(UCurveFloat*)` | None | A Float Curve as the ease |
| `SetOwner(UObject*)` | None | The tween stops when the owner is destroyed |

The builder calls work only before the tween starts. After that, they do nothing.

### Easing presets (`ENsTweenEase`)

`Linear`, and the `In`, `Out`, and `InOut` versions of `Sine`, `Quad`, `Cubic`, `Quart`, `Quint`, `Expo`, `Circ`, `Elastic`, `Bounce`, and `Back`. Also `CustomBezier` and `CurveAsset`.

![Easing presets](https://github.com/mykaadev/NsTween/blob/main/Resources/SupportedCurves.png)

## Debug

- The log category is `LogNsTween`. It shows the warnings of the async nodes, and "Cannot spawn tween without strategy."
- A Blueprint error "Tween duration must be greater than zero." comes from an async node with **Duration Secs** of 0 or less.
- `Is Tween Active` returns true while a tween runs or is paused.
- The tween functions have CPU trace scopes, such as `NsTween::Tick` and `NsTweenSubsystem::Tick`. See them in Unreal Insights with the CPU channel.

## Troubleshooting

| Problem | Cause | Fix |
|---|---|---|
| A tween does nothing in the editor | Tweens tick only while a game world exists | Test in Play In Editor |
| `Is Tween Active` is false right after the start | The tween joins the subsystem at its next tick | Test it one frame later |
| `SetLoops(-1)` plays one time | A negative count turns looping off | Use a count above 0, or ping-pong with no count |
| A builder call has no effect | The tween started already | Chain all options before any control call |
| A Rotator tween from 0 to 360 does not turn | A Rotator tween takes the shortest path | Tween a float yaw |
| `On Complete` fires after `Stop` or `Cancel Tween` | A cancel also fires `On Complete` | Check a flag of your own in the event |
| `Set Time Multiplier` has no effect | It changes the speed only at the next `Restart` | Call `Restart` after it |
| A tween keeps running after its actor is destroyed | The tween has no owner | Call `SetOwner`, or set **Owner** in the spec |
| A warning about Loop Delay, Ping Pong Delay, pause, or time dilation | These async inputs have no effect | Reset them to their defaults |
| A `Custom Curve` node plays a linear ease | **Curve** is empty | Set **Curve** |

## Limits

- Tweens use real time. A game pause and a time dilation do not change them.
- When any world starts teardown or cleanup, all tweens stop. They do not apply the end value, but `On Complete` fires.
- All tweens go to the subsystem of the first game world. With more than one Play In Editor client, all clients share one subsystem.
- A tween that ends applies the end value. A `Backward` tween and a ping-pong tween with a count therefore finish on the end value.
- A **Delay** applies one time, before the first pass. It does not apply between loops.
- The `Play ... Tween` nodes keep the address of **Target**. Use a member variable, and set **Owner** to the object that holds it.
- The C++ `Play` and the async nodes blend a Linear Color in HSV. `Play Color Tween` blends it in RGB.
- After `Restart`, the async node fires `On Complete` for the old tween and then ends. The new tween runs, but the node sends no more events after it is garbage collected.
- `UNsTweenSequence` holds a list of specs, but no code plays it.
- The module builds only for Win64.
