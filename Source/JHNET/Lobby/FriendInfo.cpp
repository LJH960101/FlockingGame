// Fill out your copyright notice in the Description page of Project Settings.

#include "FriendInfo.h"
#include "Steamworks/Steamv139/sdk/public/steam/steam_api.h"
#include "Core/HACKEDGameInstance.h"
#include "Core/Network/NetworkSystem.h"
#include "Engine/Texture2D.h"
#include <string>

UFriendInformation::UFriendInformation() {}
UFriendInformation::~UFriendInformation() {
}
UFriendInfo::UFriendInfo() {
}
UFriendInfo::~UFriendInfo() {
	InitArray();
}
void UFriendInfo::RefreshFriendInfo() {
	auto world = GetWorld();
	if (world == nullptr) {
		LOG(Error, "Get World Error");
		return;
	}
	UHACKEDGameInstance* gameInstance = Cast<UHACKEDGameInstance>(world->GetGameInstance());
	if (gameInstance == nullptr) {
		LOG(Error, "GameInstance Casting Error");
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
		LOG(Warning, "Steam is not running.");
	}
}
UFriendInformation * UFriendInfo::GetFriendInformationBySteamID(const UINT64 & id)
{
	for (int i = 0; i < FriendInformations.Num(); ++i) {
		if (FriendInformations[i]->steamID == id) {
			return FriendInformations[i];
		}
	}
	return nullptr;
}

void UFriendInfo::InitArray()
{
	FriendInformations.Empty();
}
