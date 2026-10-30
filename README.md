<!-- GH_ONLY_START -->
<h1 align="center">
  <br>
  <a href="https://github.com/mykaadev/NsTween"><img src="https://raw.githubusercontent.com/mykaadev/NsTween/refs/heads/main/Resources/Banner.png" alt="NsTween" width="400"></a>
</h1>

<h4 align="center">A small tween library for Unreal Engine, for C++ and Blueprint.</h4>

<div align="center">
    <a href="https://github.com/mykaadev/NsTween/commits/main"><img src="https://img.shields.io/github/last-commit/mykaadev/NsTween?style=plastic&logo=github&logoColor=white" alt="GitHub Last Commit"></a>
    <a href="https://github.com/mykaadev/NsTween/issues"><img src="https://img.shields.io/github/issues-raw/mykaadev/NsTween?style=plastic&logo=github&logoColor=white" alt="GitHub Issues"></a>
    <a href="https://github.com/mykaadev/NsTween/pulls"><img src="https://img.shields.io/github/issues-pr-raw/mykaadev/NsTween?style=plastic&logo=github&logoColor=white" alt="GitHub Pull Requests"></a>
    <a href="https://github.com/mykaadev/NsTween"><img src="https://img.shields.io/github/stars/mykaadev/NsTween?style=plastic&logo=github" alt="GitHub Stars"></a>
    <a href="https://twitter.com/mykaadev/"><img src="https://img.shields.io/twitter/follow/mykaadev?style=plastic&logo=x" alt="Twitter Follow"></a>

<p style="display:none;">
  <a href="#-summary">👀 Summary</a> •
  <a href="#-features">📦 Features</a> •
  <a href="#-requirements">⚙️ Requirements</a> •
  <a href="#-installation">🛠️ Installation</a> •
  <a href="#-getting-started">🚀 Getting Started</a> •
  <a href="#-blueprint-nodes">🧩 Blueprint Nodes</a> •
  <a href="#-api">🔧 API</a> •
  <a href="#-limits">🚧 Limits</a> •
  <a href="#-acknowledgement">🧾 Acknowledgement</a> •
  <a href="#-documentation">📚 Documentation</a> •
  <a href="#-credits">❤️ Credits</a> •
  <a href="#-support">📞 Support</a> •
  <a href="#-license">📃 License</a>
</p>
<a href="https://buymeacoffee.com/mykaadev"><img src="https://www.svgrepo.com/show/476855/coffee-to-go.svg" alt="Coffee" width="50px"></a>
<p><b>Buy me a coffee!</b></p>
</div>
<!-- GH_ONLY_END -->

## 👀 Summary
NsTween moves a value from a start to an end over time, with an easing curve. Use it from C++ with `FNsTween::Play`, or from Blueprint with async nodes. It suits UI motion and small game motion, such as a floating pickup, a door, or a color pulse.

NsTween does not replace UMG animations or Sequencer, and it does not replicate. Tweens run on real time, so a game pause or a time dilation does not slow them.

<div align="center">
 <img src="https://github.com/mykaadev/NsTween/blob/main/Resources/SupportedCurvesRow.png" alt="NsTween easing presets" width="750">
</div>

## 📦 Features
- **Value types**: Float, Vector, Vector 2D, Rotator, Quat, Transform, and Linear Color.
- **Easing**: Linear, 30 In, Out, and InOut presets from Sine to Back, a cubic Bezier ease, and a Float Curve asset.
- **Repeats**: Loops, ping-pong, a start delay, and a time scale.
- **Callbacks**: `OnComplete`, `OnLoop`, and `OnPingPong` in C++, and the same events on the async nodes.
- **Control**: Pause, resume, cancel, and test a tween by its handle.
- **Owner**: A tween with an owner stops when the owner is destroyed.
- **Blueprint nodes**: Async tween nodes for each value type, and `Play ... Tween` nodes that take a full spec.
- **One subsystem**: A game instance subsystem ticks all tweens from one core ticker.

## ⚙️ Requirements
- Unreal Engine 5.7+
- Windows (Win64). The module builds only for Win64.

## 🛠️ Installation
1. **Clone** or download this repository.
2. Copy the `NsTween` folder into the `Plugins` folder of your project.
3. For C++, add `"NsTween"` to the dependencies of your game module.
4. Generate the project files and enable the plugin.

## 🚀 Getting Started
In C++, `FNsTween::Play` returns a builder. Chain the options in the same statement. The tween starts when the builder goes out of scope.

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

In Blueprint:

1. Add the `Tween Float` node.
2. Set **Start**, **End**, **Duration Secs**, and **Ease Type**.
3. Connect **Apply Easing** to the code that uses **Value**.
4. Connect **On Complete** to the code that runs at the end.

## 🧩 Blueprint Nodes
| Node | Output |
|---|---|
| `Tween Float`, `Tween Vector`, `Tween Vector 2D` | The value on **Apply Easing** |
| `Tween Rotator`, `Tween Quat`, `Tween Quat From Rotator` | A Rotator or a Quat on **Apply Easing** |
| `Tween Transform`, `Tween Linear Color` | The value on **Apply Easing** |
| `... Custom Curve` versions | The same, with a Float Curve as the ease |
| `Play Float Tween`, `Play Vector Tween`, `Play Rotator Tween`, `Play Transform Tween`, `Play Color Tween` | A handle. The tween writes to the **Target** variable |
| `Pause Tween`, `Resume Tween`, `Cancel Tween`, `Is Tween Active` | Control by handle |

## 🔧 API
- `FNsTween::Play` – Make a tween from a start, an end, a duration, an ease, and an update function. A second overload takes an `FNsTweenSpec` and a value strategy factory.
- `FNsTweenBuilder` – Set the options: `SetLoops`, `SetPingPong`, `SetDelay`, `SetTimeScale`, `SetCurveAsset`, `SetOwner`, `OnComplete`, `OnLoop`, `OnPingPong`.
- `FNsTweenBuilder` control – `Pause`, `Resume`, `Cancel`, `IsActive`, `GetHandle`.
- `FNsTweenSpec` – The Blueprint struct with all the options of one tween.
- `FNsTweenHandle` – The id of one tween.
- `ENsTweenEase` – The easing presets, plus `CustomBezier` and `CurveAsset`.
- `UNsTweenBlueprintLibrary` – The `Play ... Tween` and control nodes.
- `UNsTweenAsyncAction` – The base class of the async tween nodes.

## 🚧 Limits
- Tweens use real time. The async inputs **Can Tick During Pause**, **Use Global Time Dilation**, **Loop Delay**, and **Ping Pong Delay** have no effect.
- Tweens tick only while a game world exists (Play In Editor, a game, or a game preview).
- A Rotator tween takes the shortest path, so a tween from 0 to 360 degrees does not turn.
- `UNsTweenSequence` holds a list of specs, but no code plays it.

## 🧾 Acknowledgement
NsTween takes its inspiration from **Fresh Cooked Tweens** by **Jared Cook**.

## 📚 Documentation
- [User Guide](https://mykaadev.github.io/plugins/nstween/user-guide/) – Setup, common tasks, settings, and debug tools.
- [Technical Documentation](https://mykaadev.github.io/plugins/nstween/tech-documentation/) – Architecture, lifetime, and extension points.

<!-- GH_ONLY_START -->
## ❤️ Credits
<a href="https://github.com/mykaadev/NsTween/graphs/contributors"><img src="https://contrib.rocks/image?repo=mykaadev/NsTween"/></a>

## 📞 Support
Reach out via the **[profile page](https://github.com/mykaadev)**.

## 📃 License
[![License](https://img.shields.io/badge/license-MIT-green)](https://www.tldrlegal.com/license/mit-license)
<!-- GH_ONLY_END -->
