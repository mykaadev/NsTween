// Copyright (C) 2024 mykaa. All rights reserved.

#pragma once

#include "NsTweenUObject.generated.h"

/**
 * Use this to wrap an NsTweenInstance inside a UObject, so that it's destroyed when its outer object is destroyed
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

    /** Stop the tween immediately and mark this object for destruction */
    void SetTweenInstance(class NsTweenInstance* const InTween);

    /** Stop the tween immediately and mark this object for destruction */
    void Destroy();

// Variables
public:

    /** Instance */
    class NsTweenInstance* Tween;
};
