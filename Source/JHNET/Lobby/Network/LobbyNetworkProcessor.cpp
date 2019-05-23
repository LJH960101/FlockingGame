// Fill out your copyright notice in the Description page of Project Settings.

#include "LobbyNetworkProcessor.h"
#include "Core/HACKEDGameInstance.h"
#include "Core/Network/NetworkSystem.h"
#include "Lobby/LobbyManager.h"
#include "Lobby/FriendInfo.h"
#include "Blueprint/UserWidget.h"
#include "NetworkModule/Serializer.h"
#include "NetworkModule/MyTool.h"

using namespace MyTool;
using namespace MySerializer;

// Sets default values
ALobbyNetworkProcessor::ALobbyNetworkProcessor()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
}

// Called when the game starts or when spawned
void ALobbyNetworkProcessor::BeginPlay()
{
	Super::BeginPlay();
	TArray<AActor*> actors;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), ALobbyManager::StaticClass(), actors);
	if (actors.Num() == 0) {
		LOG_SCREEN("LobbyManager must be placed in level!!!!");
		LOG(Error, "LobbyManager must be placed in level!!!!");
	}
	else if (actors.Num() > 1) {
		LOG_SCREEN("Only one of LobbyManager must be placed in level!!!!");
		LOG(Error, "Only one of LobbyManager must be placed in level!!!!");
	}
	else {
		LobbyManager = (ALobbyManager*)actors[0];
	}
}

void ALobbyNetworkProcessor::RecvProc(FReciveData& data)
{
	Super::RecvProc(data);
	int cursor = 0;
	while (cursor < data.len) {
		int bufLen = IntDeserialize(data.buf, &cursor) - sizeof(EMessageType);
		EMessageType type = GetEnum(data.buf, &cursor);
		UINT64 sid = Cast<UHACKEDGameInstance>(GetGameInstance())->GetNetworkSystem()->GetSteamID();
		switch (type)
		{
		case EMessageType::S_Room_Info:
		{
			Room_Info(data, cursor, bufLen);
			break;
		}
		case EMessageType::S_Room_ChangeState:
		{
			Room_ChangeState(data, cursor, bufLen);
			break;
		}
		case EMessageType::S_Lobby_InviteFriend_Request:
		{
			Lobby_InviteFriendRequest(data, cursor, bufLen);
			break;
		}
		case EMessageType::S_Lobby_InviteFriend_Failed:
		{
			Lobby_InviteFriendFailed(data, cursor, bufLen);
			break;
		}
		case EMessageType::S_Lobby_MatchAnswer:
		{
			Lobby_MatchAnswer(data, cursor, bufLen);
			break;
		}
		case EMessageType::S_Lobby_GameStart:
		{
			Lobby_GameStart(data, cursor, bufLen);
			return;
		}
		case EMessageType::S_RECONNECT:
		{
			Reconnect(data, cursor, bufLen);
			return;
		}
		default:
		{
			cursor += bufLen;
			break;
		}
		}
	}
}

void ALobbyNetworkProcessor::Room_Info(FReciveData& data, int& cursor, int& bufLen)
{
	// Get 4 UINT64, and insert to slot.
	UINT64 slot;
	for (int i = 0; i < MAX_PLAYER; ++i) {
		slot = UInt64Deserialize(data.buf, &cursor);
		LobbyManager->ChangeSlot(i, slot);
	}
	LobbyManager->RefreshLobby();
}

void ALobbyNetworkProcessor::Room_ChangeState(FReciveData& data, int& cursor, int& bufLen)
{
	// Get 1 int and 1 UINT64, and modify the slot.
	int slot = IntDeserialize(data.buf, &cursor);
	UINT64 steamID = IntDeserialize(data.buf, &cursor);
	LobbyManager->ChangeSlot(slot, steamID);
	LobbyManager->RefreshLobby();
}

void ALobbyNetworkProcessor::Lobby_InviteFriendRequest(FReciveData& data, int& cursor, int& bufLen)
{
	UINT64 senderID = UInt64Deserialize(data.buf, &cursor);
	FString stringSenderID = UINT64ToFString(senderID);
	FString name;
	if (LobbyManager->GetFriendBySteamId(senderID)) name = LobbyManager->GetFriendBySteamId(senderID)->name;
	else name = "Unknown";
	LobbyManager->OpenRequestInviteWG(stringSenderID, name);
}

void ALobbyNetworkProcessor::Lobby_InviteFriendFailed(FReciveData& data, int& cursor, int& bufLen)
{
	FSerializableString msg = StringDeserialize(data.buf, &cursor);
	LobbyManager->OpenLobbyFailedWG(msg.buf);
}

void ALobbyNetworkProcessor::Lobby_MatchAnswer(FReciveData& data, int& cursor, int& bufLen)
{
	bool isOn = BoolDeserialize(data.buf, &cursor);
	int slot = IntDeserialize(data.buf, &cursor);
	LobbyManager->SetReadyButton(slot, isOn);
}

void ALobbyNetworkProcessor::Lobby_GameStart(FReciveData& data, int& cursor, int& bufLen)
{
	// EDIT HERE TO CHANGE LEVEL TARGET
	UGameplayStatics::OpenLevel(GetWorld(), GameName);
}

void ALobbyNetworkProcessor::Reconnect(FReciveData& data, int& cursor, int& bufLen)
{
	// Get 4 UINT64, and insert to slot.
	UGameplayStatics::OpenLevel(GetWorld(), GameName);
}