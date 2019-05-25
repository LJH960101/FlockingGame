// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "EngineMinimal.h"
#include "Blueprint/UserWidget.h"
#include "WGGameResult.generated.h"

/**
 * 
 */
UCLASS()
class FLOCKINGEXAMPLE_API UWGGameResult : public UUserWidget
{
	GENERATED_BODY()
	
public:
	UFUNCTION(BlueprintPure, Category = "Game Result")
	FString GetGameResult();
};
