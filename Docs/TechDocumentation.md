# NsTween Technical Documentation

## Architecture

### Modules

| Module | Type | Loading Phase | Platforms | Content |
|---|---|---|---|---|
| `NsTween` | Runtime | Default | Win64 (`PlatformAllowList`) | The tween runtime, the subsystem, the Blueprint library, and the async nodes |

`CanContainContent` is true, but the plugin has no `Content` folder. `Config/DefaultNsTween.ini` holds one `[CoreRedirects]` entry (see Change Rules).

### Folders

- `Source/NsTween/Public`: `NsTween.h`, `NsTweenBuilder.h`, `NsTweenSubsystem.h`, `NsTweenTypeLibrary.h`, `NsTweenFunctionLibrary.h`, `NsTweenAsyncAction.h`, `NsTweenModule.h`.
- `Source/NsTween/Public/Easing`: the three easing classes.
- `Source/NsTween/Public/Interfaces`: `ITweenValue.h`, `IEasingCurve.h`.
- `Source/NsTween/Public/Templates`: `NsTweenInterpolator.h`, `NsTweenCallbackStrategy.h`.
- `Source/NsTween/Private`: the `.cpp` files of the public headers.
- `Source/NsTween/Private/Easing`: the easing `.cpp` files.
- `Source/NsTween/Private/ValueStrategies`: `TweenValue_Float`, `TweenValue_Vector`, `TweenValue_Rotator`, `TweenValue_Transform`, `TweenValue_Color`.
- `Source/NsTween/Private/Utils`: `NsTweenLogging` (`LogNsTween`) and `NsTweenProfiling.h` (`NSTWEEN_SCOPE_CYCLE_COUNTER`).
- `Source/NsTween/Private/Tests`: `NsTweenSubsystemCommandQueueTests.cpp`.

### Main Classes

| Type | Responsibility |
|---|---|
| `FNsTween` | One live tween: the spec, the value strategy, the easing, the delay, the cycle time, the loop counters, and the state. Also the static `Play` entry points |
| `FNsTweenBuilder` | Move-only handle that `Play` returns. It edits the spec until activation, then forwards the control calls to the subsystem |
| `UNsTweenSubsystem` | `UGameInstanceSubsystem`. It owns the tween pool, the command queue, and the core ticker. The class has no `NSTWEEN_API`, so code outside the module cannot call its functions |
| `FNsTweenSpec` | `USTRUCT(BlueprintType)` with the options of one tween, the four C++ delegates (`OnUpdate`, `OnComplete`, `OnLoop`, `OnPingPong`), and the owner |
| `FNsTweenHandle`, `FNsTweenId` | The id of a tween. An id of 0 is not valid |
| `FNsTweenCommand`, `ENsTweenCommandType` | A plain C++ command: `Spawn`, `Cancel`, `Pause`, or `Resume` |
| `ITweenValue` | Strategy that applies the eased alpha to a value |
| `IEasingCurve` | Maps a linear time from 0 to 1 to an eased alpha |
| `FNsTweenPolynomialEasing` | The presets of `ENsTweenEase`, from `Linear` to `InOutBack` |
| `FNsTweenBezierEasing` | A cubic Bezier with fixed end points (0, 0) and (1, 1). Eight Newton steps, then a bisection |
| `FNsTweenCurveAssetEasingAdapter` | Reads a `UCurveFloat` at the clamped time. With no curve, it returns the linear time |
| `FNsTweenInterpolator<T>` | The lerp of the C++ path. It has specializations for `FVector2D`, `FRotator`, `FTransform`, `FLinearColor`, and `FQuat` |
| `FNsTweenCallbackValue<T>` | The strategy of the C++ path. It calls the update function with the start value, each lerped value, and the end value |
| `TweenValue_*` | The private strategies of `UNsTweenBlueprintLibrary`. They write through a raw pointer to the `Target` parameter |
| `UNsTweenBlueprintLibrary` | `Play*Tween`, `PauseTween`, `ResumeTween`, `CancelTween`, `IsTweenActive` |
| `UNsTweenAsyncAction` and its seven children | The async Blueprint nodes. Each child holds its start and end values and its `ApplyEasing` delegate |
| `UNsTweenSequence` | `UCLASS(BlueprintType)` with a list of `FNsTweenSequenceItem`. No code reads it |
| `UNsTweenTypeLibrary` | Holds one static `GetTypeHash(const FNsTweenId&)`. It has no Blueprint functions |

### Data Flow

Spawn:

1. `FNsTween::Play(Start, End, Duration, Ease, Update)` makes a spec and a factory that returns an `FNsTweenCallbackValue<T>`. It returns a builder.
2. Builder calls edit the spec while `bActivated` is false.
3. `Activate` runs in the builder destructor, or on the first call of `Pause`, `Resume`, `Cancel`, `IsActive`, `GetHandle`, `IsValid`, or the conversion to `FNsTweenHandle`. It runs the factory and calls `EnqueueSpawn` on `UNsTweenSubsystem::GetSubsystem()`.
4. `EnqueueSpawn` gives the next id from an `FThreadSafeCounter` and puts a `Spawn` command in the queue.

Tick of the subsystem:

1. `ShouldTick` returns false during engine exit, or when no `PIE`, `Game`, or `GamePreview` world exists that is not tearing down.
2. `ProcessCommands` runs all queued commands in order. `SpawnTween` calls `CreateEasing` and adds an `FNsTween` to the pool.
3. If `DeltaTime` is below `KINDA_SMALL_NUMBER`, the tick stops here.
4. Under a read lock, the subsystem collects the active tweens.
5. Outside the lock, it calls `FNsTween::Tick` on each one. Callbacks run here, and they can queue new commands.
6. Under a write lock, it removes the tweens that are not active.

Tick of one tween:

1. A tween that is not active or is paused returns at once.
2. A tween with `bEnforceOwnerLifetime` and a dead owner stops. No callback runs.
3. On the first tick, the strategy runs `Initialize`. A `Backward` tween then runs `ApplyFinal`.
4. The delta is multiplied by `TimeScale`. The delay uses the scaled delta first.
5. The cycle time moves toward the boundary of the direction. `Apply` evaluates the easing and calls the strategy and `OnUpdate`.
6. At a boundary, `HandleBoundary` handles `Once`, `Loop`, and `PingPong`. It fires `OnLoop` or `OnPingPong`, and at the end it runs `ApplyFinal` and `OnComplete`.

Async nodes:

1. The static factory makes the node with `CreateAsyncNode`, stores the inputs, and calls `RegisterWithGameInstance`.
2. `Activate` checks the duration, then calls `LaunchTween`.
3. `StartTypedTween` calls `FNsTween::Play` with a weak pointer to the node, applies the options and the three callbacks, and activates the builder.
4. The complete callback broadcasts `OnComplete` and calls `SetReadyToDestroy`.

All tween work runs on the game thread, from the core ticker.

## Lifetime

1. `Initialize` adds a core ticker (`FTSTicker`) with no delay. It binds `FWorldDelegates::OnWorldBeginTearDown`, `FWorldDelegates::OnWorldCleanup`, `FCoreDelegates::OnPreExit`, and in the editor `FEditorDelegates::PrePIEEnded`.
2. Each of these four events calls `StopAllTweens(false)` and empties the queue. `StopAllTweens` calls `Cancel(false)` on each tween, and `Cancel` runs `OnComplete`. The handlers do not filter by world.
3. `Deinitialize` removes the ticker and the delegates, then stops all tweens and empties the queue.
4. The value strategy of a tween runs `Initialize` on its first tick, not at spawn.
5. `UNsTweenAsyncAction::BeginDestroy` cancels its tween with `Cancel(false)` if the builder is still valid.

`GetSubsystem` returns the subsystem of the first world context that is a game world or a PIE world. With more than one PIE client, every tween goes to that one subsystem.

## Extension Points

| Point | Signature | Use |
|---|---|---|
| `ITweenValue` | `virtual void Initialize() = 0`, `virtual void Apply(float EasedAlpha) = 0`, `virtual void ApplyFinal()` | A new value type or a new apply step. The default `ApplyFinal` calls `Apply(1.0f)` |
| `FNsTween::Play` | `static FNsTweenBuilder Play(FNsTweenSpec Spec, TFunction<TSharedPtr<ITweenValue>()> StrategyFactory)` | Spawn a tween with a full spec and your own strategy. This is the only C++ path to `OnUpdate`, `Direction`, and the Bezier points |
| `FNsTweenInterpolator<T>` | `static T Lerp(const T& A, const T& B, float Alpha)` | Specialize it to change the lerp of a type in the C++ `Play` |
| `IEasingCurve` | `virtual float Evaluate(float T) const = 0` | A new easing family. Add a case to the private `UNsTweenSubsystem::CreateEasing` |
| `ENsTweenEase` | `UENUM(BlueprintType)` | A new preset. Add its formula to `FNsTweenPolynomialEasing::Evaluate` |
| `UNsTweenAsyncAction` | `virtual void LaunchTween()` | A new async node. Use `CreateAsyncNode<T>` and `StartTypedTween` |
| `FNsTweenSpec::OnUpdate` | `DECLARE_DELEGATE_OneParam(FNsTweenOnUpdate, float)` | Gets the eased alpha after each apply |

For the default values of the spec, the builder, and the async nodes, see the [User Guide](UserGuide.md).

## Dependencies

| Dependency | Kind | Why |
|---|---|---|
| `Core`, `CoreUObject`, `Engine` | Public modules | Containers, `FTSTicker`, `UGameInstanceSubsystem`, `UCurveFloat`, `UBlueprintAsyncActionBase` |
| `Slate`, `SlateCore` | Private modules | Listed in `NsTween.Build.cs`. No source file includes them |
| `UnrealEd` | Private module, editor builds only | `Editor.h` for `FEditorDelegates::PrePIEEnded` |

The `.uplugin` lists no plugin dependencies.

## Performance

- One core ticker callback serves all tweens of a subsystem. It runs each engine frame.
- Each active tween costs one easing evaluation and one strategy apply per frame, plus one more at each boundary.
- The command queue is a `TQueue` in `EQueueMode::Mpsc`. The pool is a `TArray<TUniquePtr<FNsTween>>` with an `FRWLock`.
- `IsActive`, `Pause`, `Resume`, and `Cancel` search the pool in a linear loop.
- Each `Play` makes a `TFunction`, a shared strategy, and a shared easing object. Many new tweens per frame cost heap allocations.
- Every main function has a `TRACE_CPUPROFILER_EVENT_SCOPE_TEXT` scope, through `NSTWEEN_SCOPE_CYCLE_COUNTER`.

## Networking

The plugin does not replicate.

## Testing

| Test | Path | Checks |
|---|---|---|
| `FNsTweenSubsystemCommandQueueTest` | `NsTween.Subsystem.CommandEnqueue` | `EnqueueSpawn` returns a valid handle. Spawn, cancel, pause, and resume each put the right command in the queue. Cancel keeps `bApplyFinalOnCancel` |

The test makes a subsystem with `NewObject` and does not initialize it. It reads the queue through `FNsTweenSubsystemTestAccessor`. It runs in the editor context (`EditorContext | EngineFilter`), under `WITH_AUTOMATION_TESTS`.

To run it:

1. Open **Tools > Session Frontend > Automation**.
2. Search for `NsTween`.
3. Run `NsTween.Subsystem.CommandEnqueue`.

From the command line, use `-ExecCmds="Automation RunTests NsTween"`.

## Change Rules

- Do not rename the `UPROPERTY` fields of `FNsTweenSpec`, or the values of `ENsTweenEase`, `ENsTweenWrapMode`, and `ENsTweenDirection`. Blueprints save them by name.
- Do not rename the parameters of the async factories or of `UNsTweenBlueprintLibrary`. Blueprint pins link by parameter name.
- `Config/DefaultNsTween.ini` redirects the class `/Script/NsTween.NsTweenBlueprintLibrary` to `/Script/NsTween.NsTweenFunctionLibrary`. No class has the name `NsTweenFunctionLibrary`: the class in `NsTweenFunctionLibrary.h` is `UNsTweenBlueprintLibrary`. Make sure that the redirect and the class name agree before you rename either one.
- `FNsTweenBezierDefinition` in `NsTweenBezierEasing.h` is not used. `FNsTweenBezierEasing` reads an `FVector4`.
