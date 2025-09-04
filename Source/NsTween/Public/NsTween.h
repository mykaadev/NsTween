// Copyright (C) 2025 nulled.softworks. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"

/**
 * Nulled Softworks Tween Runtime Module
 */
class FNsTweenModule : public IModuleInterface
{

// Functions
public:

    //~ Begin IModuleInterface Interface
    virtual void StartupModule() override;
    virtual void ShutdownModule() override;
    //~ End IModuleInterface Interface
};
