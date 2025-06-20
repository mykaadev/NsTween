<!-- GH_ONLY_START -->
<h1 align="center">
  <br>
  <a href="https://github.com/mykaadev/NsTween">
    <img src="https://github.com/mykaadev/NsTween/blob/cc9e0b441a9305994196757e401b1465734f7954/Resources/Banner.png" alt="NsTween" width="400">
  </a>
</h1>

<h4 align="center">A lightweight tweening library for Unreal Engine.</h4>

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
  <a href="#-api">🔧 API</a> •
  <a href="#-credits">❤️ Credits</a> •
  <a href="#-support">📞 Support</a> •
  <a href="#-license">📃 License</a>
</p>
<a href="https://buymeacoffee.com/mykaadev"><img src="https://www.svgrepo.com/show/476855/coffee-to-go.svg" alt="Coffee" width="50px"></a>
<p><b>Buy me a coffee!</b></p>
</div>
<!-- GH_ONLY_END -->

## 👀 Summary
NsTwee is a small yet powerful tweening framework for Unreal Engine. It allows smooth interpolation of floats, vectors and quaternions using a rich set of easing functions. Tweens can be controlled entirely through C++ or Blueprint nodes.

## 📦 Features
- **Multiple data types**: Tween `float`, `vector`, `vector2D`, `rotator` and `quaternion` values.
- **Custom easing**: Choose from many easing curves or drive interpolation via a user supplied curve.
- **Looping and yo-yo**: Built-in looping, delays and yo-yo behaviour with callbacks.
- **Blueprint actions**: Async Blueprint nodes for quick setup without code.
- **Subsystem based**: A game instance subsystem updates active tweens automatically.
- **Capacity control**: Reserve tween instances up front to avoid runtime allocations.

## ⚙️ Requirements
Unreal Engine 5.2+

## 🛠️ Installation
1. **Clone** or download this repository.
2. Copy the `NsTween` folder into your project's `Plugins` directory.
3. Generate project files and enable the plugin when prompted.

## 🚀 Getting Started
Below is a minimal C++ example showing how to move an actor along the X axis using `NsTweenCore`:

```cpp
#include "NsTweenCore.h"

void AMyActor::BeginPlay()
{
    Super::BeginPlay();

    NsTweenCore::Play(
        /**Start*/   0.f,
        /**End*/     100.f,
        /**Time*/    2.f,
        /**Ease*/    ENsTweenEase::InOutQuad,
        /**Update*/  [this](float Value)
        {
            SetActorLocation(FVector(Value, 0.f, 0.f));
        });
}
```

The library also exposes Blueprint nodes for the same functionality if you prefer a visual approach.

Below is a slightly more advanced snippet showing how to make an item float up and down while rotating. When the rotation tween finishes we call a custom `Pop()` function:

```cpp
#include "NsTweenCore.h"

void AFloatingItem::BeginPlay()
{
    Super::BeginPlay();

        // Float continuously
    NsTweenCore::Play(
         /**Start*/   GetActorLocation().Z,
         /**End*/     GetActorLocation().Z + 40.f,
         /**Time*/    1.f,
         /**Ease*/    ENsTweenEase::InOutSine,
         /**Update*/  [this](float Z)
        {
            FVector CurrentLocation = GetActorLocation();
            CurrentLocation.Z = Z;
            SetActorLocation(CurrentLocation);
        })
        .SetPingPong(true)
        .SetLoops(-1); // infinite loops


    // Rotate and print 10 times the Loop
    NsTweenCore::Play(
        /** Start  */  0.f,
        /** End    */  360.f,
        /** Time   */  2.f,
        /** Ease   */  ENsTweenEase::Linear,
        /** Update */  [this](float Yaw)
        {
            SetActorRotation(FRotator(0.f, Yaw, 0.f));
        })
        .SetLoops(10) // 10 full spins
        .OnLoop([this]()
        {
            UE_LOG(LogTemp, Warning, TEXT("Spin finished"));
        });
}
```

## 🔧 API
### Key Classes
- `NsTweenCore` – static helpers to play tweens in C++.
- `UNsTweenSubsystem` – automatically ticks and manages active tweens.
- `UNsTweenAsyncAction` – base class for asynchronous Blueprint tween nodes.
- `UNsTweenBlueprintLibrary` – utility functions including `Ease` helpers.

### Main Functions
- `NsTweenCore::Play` – start a tween in C++ for various types.
- `UNsTweenAsyncActionFloat::TweenFloat` – Blueprint node to tween a float value.
- `UNsTweenAsyncActionVector::TweenVector` – Blueprint node to tween vectors.
- `NsTweenInstance` – represents a single tween instance with control methods.

<!-- GH_ONLY_START -->
## ❤️ Credits
<a href="https://github.com/mykaadev/NsTween/graphs/contributors"><img src="https://contrib.rocks/image?repo=mykaadev/NsTween"/></a>

## 📞 Support
Reach out via the **[profile page](https://github.com/mykaadev)**.

## 📃 License
[![License](https://img.shields.io/badge/license-MIT-green)](https://www.tldrlegal.com/license/mit-license)
<!-- GH_ONLY_END -->
