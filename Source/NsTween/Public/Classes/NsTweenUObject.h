// Copyright (C) 2025 mykaadev. All rights reserved.

#pragma once

#include "NsTweenUObject.generated.h"

/**
 * Simple UObject wrapper around a NsTweenInstance.
 * Ensures the tween is cleaned up when this UObject is destroyed.
 */
UCLASS()
class UNsTweenUObject : public UObject
{
    GENERATED_BODY()

// Functions
public:

    /** Constructor */
    UNsTweenUObject();

    //~ Begin UObject Interface
    virtual void BeginDestroy() override;
    //~ End UObject Interface

    /** Assign the tween instance to manage. */
    void SetTweenInstance(class NsTweenInstance* const InTween);

    /** Stop the tween immediately and mark this UObject for destruction */
    void Destroy();

// Variables
public:

    /** Managed tween instance */
    class NsTweenInstance* Tween;
};
