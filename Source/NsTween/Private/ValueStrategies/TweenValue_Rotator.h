#pragma once

#include "Interfaces/ITweenValue.h"
#include "CoreMinimal.h"

class FTweenValue_Rotator : public ITweenValue
{
public:
    FTweenValue_Rotator(FRotator* InTarget, const FRotator& InStart, const FRotator& InEnd);

    virtual void Initialize() override;
    virtual void Apply(float EasedAlpha) override;
    virtual void ApplyFinal() override;

private:
    FRotator* Target = nullptr;
    FQuat StartQuat;
    FQuat EndQuat;
};

