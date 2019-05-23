// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "WGFriendRequest.generated.h"

/**
 * Friend WG inside of friend list WG.
 */
UCLASS()
class JHNET_API UWGFriendRequest : public UUserWidget
{
	GENERATED_BODY()
public:
	UFUNCTION(BlueprintImplementableEvent, Category = "Steam")
	void SetUserName(const FString& name);
	void SetSteamID(FString steamID);
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Steam")
	FString _steamID;
};
