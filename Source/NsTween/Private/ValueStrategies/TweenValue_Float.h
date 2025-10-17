#pragma once

#include "Interfaces/ITweenValue.h"
#include "CoreMinimal.h"

class FTweenValue_Float : public ITweenValue
{
public:
    FTweenValue_Float(float* InTarget, float InStart, float InEnd);

    virtual void Initialize() override;
    virtual void Apply(float EasedAlpha) override;
    virtual void ApplyFinal() override;

private:
    float* Target = nullptr;
    float StartValue = 0.f;
    float EndValue = 0.f;
};

