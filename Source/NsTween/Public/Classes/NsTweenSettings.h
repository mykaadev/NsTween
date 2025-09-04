// Copyright (C) 2025 nulled.softworks. All rights reserved.

#pragma once

#include "Engine/DeveloperSettings.h"
#include "UObject/UObjectIterator.h"
#include "NsTweenSettings.generated.h"

/**
 * Global settings for NsTween
 */
UCLASS(Config=Game, DefaultConfig, meta=(DisplayName="NsTween"))
class NSTWEEN_API UNsTweenSettings : public UDeveloperSettings
{
    GENERATED_BODY()

// Functions
public:

    /** Constructor */
    UNsTweenSettings();

    /** Get Settings */
    UFUNCTION(BlueprintCallable, Category = "Tween Settings")
    static UNsTweenSettings* GetSettings()
    {
        static UNsTweenSettings* Instance;
        if(Instance != nullptr)
        {
            return Instance;
        }

        for (const TObjectIterator<UNsTweenSettings> SettingsIt(RF_NoFlags); SettingsIt;)
        {
            Instance = *SettingsIt;
            break;
        }
        return Instance;
    }

// Variables
public:

    /** Initial number of float tweens reserved */
    UPROPERTY(EditAnywhere, Config, Category="Tween Capacity", meta=(ClampMin="0"))
    int32 FloatTweenCapacity;

    /** Initial number of vector tweens reserved */
    UPROPERTY(EditAnywhere, Config, Category="Tween Capacity", meta=(ClampMin="0"))
    int32 VectorTweenCapacity;

    /** Initial number of vector2D tweens reserved */
    UPROPERTY(EditAnywhere, Config, Category="Tween Capacity", meta=(ClampMin="0"))
    int32 Vector2DTweenCapacity;

    /** Initial number of quaternion tweens reserved */
    UPROPERTY(EditAnywhere, Config, Category="Tween Capacity", meta=(ClampMin="0"))
    int32 QuatTweenCapacity;

};


