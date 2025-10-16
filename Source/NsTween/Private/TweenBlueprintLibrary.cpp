#include "TweenBlueprintLibrary.h"
#include "TweenManager.h"
#include "ValueStrategies/TweenValue_Float.h"
#include "ValueStrategies/TweenValue_Vector.h"
#include "ValueStrategies/TweenValue_Rotator.h"
#include "ValueStrategies/TweenValue_Transform.h"
#include "ValueStrategies/TweenValue_Color.h"
#include "Engine/World.h"
#include "Engine/Engine.h"

UNsTweenManagerSubsystem* UNsTweenBlueprintLibrary::GetManager(UObject* WorldContextObject)
{
    if (!GEngine)
    {
        return nullptr;
    }

    return GEngine->GetEngineSubsystem<UNsTweenManagerSubsystem>();
}

FNovaTweenHandle UNsTweenBlueprintLibrary::PlayFloatTween(UObject* WorldContextObject, float StartValue, float EndValue, float& Target, const FNovaTweenSpec& Spec)
{
    if (UNsTweenManagerSubsystem* Manager = GetManager(WorldContextObject))
    {
        TSharedPtr<ITweenValue> Strategy = MakeShared<FTweenValue_Float>(&Target, StartValue, EndValue);
        return Manager->EnqueueSpawn(Spec, Strategy);
    }

    return FNovaTweenHandle();
}

FNovaTweenHandle UNsTweenBlueprintLibrary::PlayVectorTween(UObject* WorldContextObject, const FVector& StartValue, const FVector& EndValue, FVector& Target, const FNovaTweenSpec& Spec)
{
    if (UNsTweenManagerSubsystem* Manager = GetManager(WorldContextObject))
    {
        TSharedPtr<ITweenValue> Strategy = MakeShared<FTweenValue_Vector>(&Target, StartValue, EndValue);
        return Manager->EnqueueSpawn(Spec, Strategy);
    }

    return FNovaTweenHandle();
}

FNovaTweenHandle UNsTweenBlueprintLibrary::PlayRotatorTween(UObject* WorldContextObject, const FRotator& StartValue, const FRotator& EndValue, FRotator& Target, const FNovaTweenSpec& Spec)
{
    if (UNsTweenManagerSubsystem* Manager = GetManager(WorldContextObject))
    {
        TSharedPtr<ITweenValue> Strategy = MakeShared<FTweenValue_Rotator>(&Target, StartValue, EndValue);
        return Manager->EnqueueSpawn(Spec, Strategy);
    }

    return FNovaTweenHandle();
}

FNovaTweenHandle UNsTweenBlueprintLibrary::PlayTransformTween(UObject* WorldContextObject, const FTransform& StartValue, const FTransform& EndValue, FTransform& Target, const FNovaTweenSpec& Spec)
{
    if (UNsTweenManagerSubsystem* Manager = GetManager(WorldContextObject))
    {
        TSharedPtr<ITweenValue> Strategy = MakeShared<FTweenValue_Transform>(&Target, StartValue, EndValue);
        return Manager->EnqueueSpawn(Spec, Strategy);
    }

    return FNovaTweenHandle();
}

FNovaTweenHandle UNsTweenBlueprintLibrary::PlayColorTween(UObject* WorldContextObject, const FLinearColor& StartValue, const FLinearColor& EndValue, FLinearColor& Target, const FNovaTweenSpec& Spec)
{
    if (UNsTweenManagerSubsystem* Manager = GetManager(WorldContextObject))
    {
        TSharedPtr<ITweenValue> Strategy = MakeShared<FTweenValue_Color>(&Target, StartValue, EndValue);
        return Manager->EnqueueSpawn(Spec, Strategy);
    }

    return FNovaTweenHandle();
}

void UNsTweenBlueprintLibrary::PauseTween(UObject* WorldContextObject, FNovaTweenHandle Handle)
{
    if (UNsTweenManagerSubsystem* Manager = GetManager(WorldContextObject))
    {
        Manager->EnqueuePause(Handle);
    }
}

void UNsTweenBlueprintLibrary::ResumeTween(UObject* WorldContextObject, FNovaTweenHandle Handle)
{
    if (UNsTweenManagerSubsystem* Manager = GetManager(WorldContextObject))
    {
        Manager->EnqueueResume(Handle);
    }
}

void UNsTweenBlueprintLibrary::CancelTween(UObject* WorldContextObject, FNovaTweenHandle Handle, bool bApplyFinal)
{
    if (UNsTweenManagerSubsystem* Manager = GetManager(WorldContextObject))
    {
        Manager->EnqueueCancel(Handle, bApplyFinal);
    }
}

bool UNsTweenBlueprintLibrary::IsTweenActive(UObject* WorldContextObject, FNovaTweenHandle Handle)
{
    if (UNsTweenManagerSubsystem* Manager = GetManager(WorldContextObject))
    {
        return Manager->IsActive(Handle);
    }
    return false;
}

