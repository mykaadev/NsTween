// Copyright (C) 2024 mykaa. All rights reserved.

#include "Classes/NsTweenUObject.h"
#include "Classes/NsTweenInstance.h"

UNsTweenUObject::UNsTweenUObject()
{
	Tween = nullptr;
}
void UNsTweenUObject::BeginDestroy()
{
	if (Tween != nullptr)
	{
		Tween->Destroy();
		Tween = nullptr;
	}
	UObject::BeginDestroy();
}

void UNsTweenUObject::SetTweenInstance(NsTweenInstance* InTween)
{
	this->Tween = InTween;
	// destroy when we are destroyed
	this->Tween->SetAutoDestroy(false);
}

void UNsTweenUObject::Destroy()
{
	this->Tween->Destroy();
	this->Tween = nullptr;
	ConditionalBeginDestroy();
}
