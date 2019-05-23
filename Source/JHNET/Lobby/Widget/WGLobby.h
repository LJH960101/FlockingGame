// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Engine/Texture2D.h"
#include "WGLobby.generated.h"

/**
 * 
 */
UCLASS()
class JHNET_API UWGLobby : public UUserWidget
{
	GENERATED_BODY()
	
public:
	UFUNCTION(BlueprintImplementableEvent)
	void SetPartySlot(const int& slotNumber, const FString& name, UTexture2D* image, const FString& steamID, bool onReady);

	UFUNCTION(BlueprintImplementableEvent)
	void OnReady(bool onRead);

	UFUNCTION(BlueprintImplementableEvent)
	void OnMatching(bool isOn);
};
