// Copyright (C) 2025 nulled.softworks. All rights reserved.

#pragma once

#include "Modules/ModuleManager.h"

class NSTWEEN_API FNsTweenModule : public IModuleInterface
{
public:
    virtual void StartupModule() override;
    virtual void ShutdownModule() override;
};

