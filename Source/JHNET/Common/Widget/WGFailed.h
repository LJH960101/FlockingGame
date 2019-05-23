// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "WGFailed.generated.h"

/**
 * 
 */
UCLASS()
class JHNET_API UWGFailed : public UUserWidget
{
	GENERATED_BODY()
	
public:
	UFUNCTION(BlueprintImplementableEvent)
	void SetMessage(const FString& msg);
};
