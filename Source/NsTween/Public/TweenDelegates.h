// Copyright (C) 2024 mykaa. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "Delegates/DelegateCombinations.h"

DECLARE_DYNAMIC_DELEGATE_OneParam(FNovaTweenUpdate, float, NormalizedAlpha);
DECLARE_DYNAMIC_DELEGATE(FNovaTweenComplete);
DECLARE_DYNAMIC_DELEGATE(FNovaTweenLoop);
DECLARE_DYNAMIC_DELEGATE(FNovaTweenPingPong);

