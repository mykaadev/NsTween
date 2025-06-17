// Copyright (C) 2025 mykaadev. All rights reserved.

#include "Classes/NsTweenUObject.h"
#include "Classes/NsTweenInstance.h"

UNsTweenUObject::UNsTweenUObject()
    : Tween(nullptr)
{}
void UNsTweenUObject::BeginDestroy()
{
    if (Tween)
    {
        // Clean up the managed tween before destruction
        Tween->Destroy();
        Tween = nullptr;
    }
    Super::BeginDestroy();
}
void UNsTweenUObject::SetTweenInstance(NsTweenInstance* InTween)
{
    Tween = InTween;
    // Prevent the tween from destroying itself prematurely
    Tween->SetAutoDestroy(false);
}

void UNsTweenUObject::Destroy()
{
    if (Tween)
    {
        Tween->Destroy();
        Tween = nullptr;
    }
    ConditionalBeginDestroy();
}
