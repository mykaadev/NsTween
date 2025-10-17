// Copyright (C) 2025 nulled.softworks. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "Interfaces/ITweenValue.h"

class FTweenValue_Color : public ITweenValue
{
public:
    FTweenValue_Color(FLinearColor* InTarget, const FLinearColor& InStart, const FLinearColor& InEnd);

    virtual void Initialize() override;
    virtual void Apply(float EasedAlpha) override;
    virtual void ApplyFinal() override;

private:
    FLinearColor* Target = nullptr;
    FLinearColor StartValue = FLinearColor::Black;
    FLinearColor EndValue = FLinearColor::White;
};

