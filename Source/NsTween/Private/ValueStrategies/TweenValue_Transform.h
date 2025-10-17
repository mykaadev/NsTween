#pragma once

#include "Interfaces/ITweenValue.h"
#include "CoreMinimal.h"

class FTweenValue_Transform : public ITweenValue
{
public:
    FTweenValue_Transform(FTransform* InTarget, const FTransform& InStart, const FTransform& InEnd);

    virtual void Initialize() override;
    virtual void Apply(float EasedAlpha) override;
    virtual void ApplyFinal() override;

private:
    FTransform* Target = nullptr;
    FTransform StartValue;
    FTransform EndValue;
};

