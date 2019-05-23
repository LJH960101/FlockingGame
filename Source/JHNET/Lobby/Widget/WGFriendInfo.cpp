// Fill out your copyright notice in the Description page of Project Settings.

#include "WGFriendInfo.h"
#include "Steamworks/Steamv139/sdk/public/steam/steam_api.h"
#include "Core/JHNETGameInstance.h"
#include "Core/Network/NetworkSystem.h"
#include "Engine/Texture2D.h"
#include <string>

UFriendInformation::UFriendInformation() {}
UFriendInformation::~UFriendInformation() {
}
UWGFriendInfo::UWGFriendInfo() {
}
UWGFriendInfo::~UWGFriendInfo() {
	InitArray();
}
void UWGFriendInfo::RefreshFriendInfo() {
	auto world = GetWorld();
	if (world == nullptr) {
		JHNET_LOG(Error, "Get World Error");
		return;
	}
	UJHNETGameInstance* gameInstance = Cast<UJHNETGameInstance>(world->GetGameInstance());
	if (gameInstance == nullptr) {
		JHNET_LOG(Error, "GameInstance Casting Error");
		return;
	}
	if (gameInstance->GetNetworkSystem()->OnSteam()) {
		InitArray();

		// Get Friends Info
		uint64 myId = SteamUser()->GetSteamID().ConvertToUint64();
		ISteamFriends* steamFriends = SteamFriends();
		int friendCount = steamFriends->GetFriendCount(k_EFriendFlagAll);
		for (int i = 0; i < friendCount; ++i) {
			CSteamID id = steamFriends->GetFriendByIndex(i, k_EFriendFlagAll);

			FriendGameInfo_t info;
			steamFriends->GetFriendGamePlayed(id, &info);
			if (info.m_gameID.ToUint64() == 480) { // Playing same game?
				FString name = gameInstance->GetNetworkSystem()->GetSteamName(id.ConvertToUint64());

				UFriendInformation* newInformation = NewObject<UFriendInformation>(this);
				newInformation->name = name;
				newInformation->steamID = id.ConvertToUint64();
				newInformation->avatar = gameInstance->GetNetworkSystem()->GetSteamAvartar(newInformation->steamID);
				FriendInformations.Add(newInformation);
			}
		}
	}
	else {
		JHNET_LOG(Warning, "Steam is not running.");
	}
}
UFriendInformation * UWGFriendInfo::GetFriendInformationBySteamID(const UINT64 & id)
{
	for (int i = 0; i < FriendInformations.Num(); ++i) {
		if (FriendInformations[i]->steamID == id) {
			return FriendInformations[i];
		}
	}
	return nullptr;
}

void UWGFriendInfo::InitArray()
{
	FriendInformations.Empty();
}
