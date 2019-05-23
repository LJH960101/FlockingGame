// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "JHNET.h"
#include "UObject/NoExportTypes.h"
#include "WGFriendInfo.generated.h"

// Class that contains info about friend.
UCLASS()
class JHNET_API UFriendInformation : public UObject
{
	GENERATED_BODY()
public:
	UFriendInformation();
	~UFriendInformation();
	uint64 steamID;
	UPROPERTY()
	UTexture2D* avatar;
	FString name;
};

class CSteamID;
/**
 * UObject that include friend list
 */
UCLASS()
class JHNET_API UWGFriendInfo : public UObject
{
	GENERATED_BODY()
public:
	UWGFriendInfo();
	~UWGFriendInfo();
	void RefreshFriendInfo();
	UPROPERTY()
	TArray<UFriendInformation*> FriendInformations;
	UFriendInformation* GetFriendInformationBySteamID(const UINT64& id);
	void InitArray();
};
