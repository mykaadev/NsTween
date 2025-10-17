#pragma once

#include "Interfaces/ITweenValue.h"
#include "CoreMinimal.h"

class FTweenValue_Vector : public ITweenValue
{
public:
    FTweenValue_Vector(FVector* InTarget, const FVector& InStart, const FVector& InEnd);

    virtual void Initialize() override;
    virtual void Apply(float EasedAlpha) override;
    virtual void ApplyFinal() override;

private:
    FVector* Target = nullptr;
    FVector StartValue = FVector::ZeroVector;
    FVector EndValue = FVector::ZeroVector;
};

