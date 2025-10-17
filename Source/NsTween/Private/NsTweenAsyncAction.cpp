#include "NsTweenAsyncAction.h"
#include "NsTweenFunctionLibrary.h"

UNsTweenAsyncAction* UNsTweenAsyncAction::PlayFloatTween(UObject* WorldContextObject, float StartValue, float EndValue, float& Target, const FNsTweenSpec& Spec)
{
    UNsTweenAsyncAction* Action = NewObject<UNsTweenAsyncAction>();
    Action->WorldContext = WorldContextObject;
    Action->Spec = Spec;
    Action->StartValue = StartValue;
    Action->EndValue = EndValue;
    Action->TargetPtr = &Target;
    Action->RegisterWithGameInstance(WorldContextObject);
    return Action;
}

void UNsTweenAsyncAction::Activate()
{
    if (bActivated)
    {
        return;
    }
    bActivated = true;

    BindDelegates();

    if (!TargetPtr)
    {
        SetReadyToDestroy();
        return;
    }

    Handle = UNsTweenBlueprintLibrary::PlayFloatTween(WorldContext, StartValue, EndValue, *TargetPtr, Spec);
    if (!Handle.IsValid())
    {
        SetReadyToDestroy();
    }
}

void UNsTweenAsyncAction::SetReadyToDestroy()
{
    if (Handle.IsValid())
    {
        UNsTweenBlueprintLibrary::CancelTween(WorldContext, Handle, false);
        Handle.Reset();
    }

    Super::SetReadyToDestroy();
}

void UNsTweenAsyncAction::BindDelegates()
{
    Spec.OnUpdate.BindUObject(this, &UNsTweenAsyncAction::HandleUpdate);
    Spec.OnComplete.BindUObject(this, &UNsTweenAsyncAction::HandleComplete);
    Spec.OnLoop.BindUObject(this, &UNsTweenAsyncAction::HandleLoop);
    Spec.OnPingPong.BindUObject(this, &UNsTweenAsyncAction::HandlePingPong);
}

void UNsTweenAsyncAction::HandleUpdate(float NormalizedAlpha)
{
    OnUpdate.Broadcast(NormalizedAlpha);
}

void UNsTweenAsyncAction::HandleComplete()
{
    OnComplete.Broadcast();
    SetReadyToDestroy();
}

void UNsTweenAsyncAction::HandleLoop()
{
    OnLoop.Broadcast();
}

void UNsTweenAsyncAction::HandlePingPong()
{
    OnPingPong.Broadcast();
}

