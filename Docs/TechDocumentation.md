# NsTween Technical Documentation

## Architecture

One runtime module, `NsTween`.

| Type | Role |
|---|---|
| `FNsTween` | One live tween: spec, value strategy, easing, delay, cycle time, loop count, and state. Also the static `Play` entry points. |
| `FNsTweenBuilder` | Move-only fluent handle. It edits the spec until activation, then forwards control calls. |
| `UNsTweenSubsystem` | Game instance subsystem. It owns the tween pool, the command queue, and the core ticker. |
| `FNsTweenSpec` | Duration, delay, time scale, wrap mode, loop count, direction, easing, Bezier points, curve asset, callbacks, and owner. |
| `FNsTweenHandle`, `FNsTweenId` | The id of a spawned tween. |
| `FNsTweenCommand`, `ENsTweenCommandType` | Queued `Spawn`, `Cancel`, `Pause`, `Resume`. |
| `ITweenValue` | Strategy contract. It applies the eased alpha to a target value. |
| `IEasingCurve` | Easing contract. |
| `FNsTweenPolynomialEasing`, `FNsTweenBezierEasing`, `FNsTweenCurveAssetEasingAdapter` | The easing implementations. |
| `FNsTweenInterpolator<T>`, callback strategy | Templates for typed lerp and lambda updates in the C++ path. |
| `TweenValue_*` | Private strategies for the Blueprint library: float, vector, rotator, transform, color. |
| `UNsTweenBlueprintLibrary` | Blueprint `Play*Tween`, `PauseTween`, `ResumeTween`, `CancelTween`, `IsTweenActive`. |
| `UNsTweenAsyncAction` and children | Async Blueprint nodes for float, vector, vector 2D, rotator, quaternion, transform, and linear color. |
| `UNsTweenSequence` | A list of `FNsTweenSequenceItem` specs. No player exists yet. |

### Spawn flow

1. `FNsTween::Play` makes a spec and a strategy factory, and returns a builder.
2. Builder calls edit the spec while it is not active.
3. `Activate` runs on the first control call or in the builder destructor. It calls the factory and `EnqueueSpawn`.
4. The subsystem processes the command queue at the start of the next tick and adds the tween to the pool.

### Tick flow

1. `ShouldTick` returns false during engine exit, or when no Game, PIE, or GamePreview world exists that is not tearing down.
2. Process all queued commands.
3. Under a read lock, collect the active tweens.
4. Outside the lock, tick each tween. Callbacks can run here and can enqueue new commands safely.
5. Compact the pool when a tween ended.

### Tween tick

A tween stops when its owner is gone (if `SetOwner` was used). It waits for the delay, then advances the cycle time with `TimeScale`, applies the eased value, and handles the boundary for `Once`, `Loop`, and `PingPong`.

## Lifetime

- The subsystem registers an `FTSTicker` callback in `Initialize`. The core ticker uses real time.
- `StopAllTweens` runs on world teardown, world cleanup, pre-exit, and PIE end. It clears the pool and drains the queue.
- `UNsTweenSubsystem::GetSubsystem` finds the subsystem of the first game or PIE world. With more than one PIE client, all tweens go to that one subsystem.

## Extension Points

| Point | Use |
|---|---|
| `ITweenValue` | A new value type or a custom apply step. Pass a factory to `FNsTween::Play(Spec, Factory)`. |
| `IEasingCurve` | A new easing family. Add it to `CreateEasing`. |
| `FNsTweenInterpolator<T>` | A specialization for a new type in the C++ `Play`. |
| `ENsTweenEase` | A new preset. Add it to the polynomial easing. |

## Dependencies

The module depends on engine modules only. No other studio plugin.

## Performance

- One core ticker callback for all tweens. Each tween costs one easing evaluation and one strategy apply each frame.
- The command queue is a multi-producer, single-consumer `TQueue`. The pool uses an `FRWLock`.
- Builder lambdas allocate. Avoid thousands of new tweens each frame.

## Networking

None. Tweens are local and cosmetic. Replicate the gameplay state, not the tween.

## Testing

Automation test `NsTween.Subsystem.CommandEnqueue` in `Private/Tests/NsTweenSubsystemCommandQueueTests.cpp`. It checks that spawn and cancel enqueue the right commands and handles.

Manual test path:

1. Play a ping-pong float tween with 2 loops. Make sure that `OnPingPong` fires and the tween stops.
2. Destroy the owner during a tween. Make sure that the update stops.
3. Pause the game. Make sure that tweens keep running. This is the current behavior.

## Change Rules

- Pause-aware and time-dilation-aware ticking is a candidate feature. The async node inputs exist already. A change here changes the behavior of current menus. Use Plugin Feature Fit first.
- A `UNsTweenSequence` player is a candidate feature.
- `UNsTweenTypeLibrary` only holds `GetTypeHash` for `FNsTweenId`.
- Keep `FNsTween::Play` stable. Game code in every project uses it.
