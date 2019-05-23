// Fill out your copyright notice in the Description page of Project Settings.

#pragma once
#include "NetworkModule/GameInfo.h"
#include "JHNET.h"
#include "GameFramework/Actor.h"
#include "Widget/WGLobby.h"
#include "Blueprint/UserWidget.h"
#include "Widget/RoomPlayer.h"
#include "LobbyManager.generated.h"

class UWGFriendInfo;
class UFriendInformation;
class UJHNETGameInstance;
class URoomPlayer;
class ALobbyNetworkProcessor;

UCLASS()
class JHNET_API ALobbyManager : public AActor
{
	GENERATED_BODY()
private:
	UPROPERTY()
	UWGFriendInfo* friendInfos;

	UPROPERTY()
	ALobbyNetworkProcessor* _lobbyNetworkProcessor;

	UPROPERTY()
	UJHNETGameInstance* gameInstance;

	UPROPERTY()
	URoomPlayer* slots[MAX_PLAYER];

	UPROPERTY()
	TSubclassOf<class UWGLobby> WG_Lobby_Class;
	UPROPERTY()
	UWGLobby* WG_Lobby;

	UPROPERTY()
	TSubclassOf<class UUserWidget> WG_Setting_Class;
	UPROPERTY()
	UUserWidget* WG_Setting;
	bool onSetting;

	UPROPERTY()
	TSubclassOf<class UUserWidget> WG_Friend_Class;
	UPROPERTY()
	UUserWidget* WG_Friend;
	bool onFriend;

	UPROPERTY()
	TSubclassOf<class UWGFriendRequest> WG_RequestInvite_Class;
	UPROPERTY()
	UWGFriendRequest* WG_RequestInvite;

	UPROPERTY()
	TSubclassOf<class UWGFailed> WG_Failed_Class;
	UPROPERTY()
	UUserWidget* WG_Failed;

public:
	// Sets default values for this actor's properties
	ALobbyManager();

	// Network Function ******
	UFUNCTION(BlueprintCallable, Category = "InviteFriend")
	void InviteFriend(FString steamID);
	UFUNCTION(BlueprintCallable, Category = "InviteFriend")
	void AnswerInviteRequest(bool isYes, const FString& steamID);
	UFUNCTION(BlueprintCallable, Category = "Matching")
	void OnReady(bool isOn);
	// Kick the slot (Only work on party king)
	UFUNCTION(BlueprintCallable, Category = "Kick")
	void Kick(const int32& slot, const FString& steamID);
	// Request transfer party king. (Only work on party king)
	UFUNCTION(BlueprintCallable, Category = "Party")
	void ChangePartyKing(const int32& slot, const FString& steamID);
	// Force registered steamID.
	UFUNCTION(BlueprintCallable, Category = "Debug")
	void SetSteamID_DEBUG(const FString& steamID);
	// Force join to room.
	UFUNCTION(BlueprintCallable, Category = "Debug")
	void EnterToRoom_DEBUG();

	// Wrapping Function relation by friend
	UFUNCTION(BlueprintCallable, Category = "Friend")
	void RefreshList();
	UFUNCTION(BlueprintCallable, Category = "Friend")
	int GetFriendLen();
	UFUNCTION(BlueprintCallable, Category = "Friend")
	FString GetFriendName(const int& number);
	UFUNCTION(BlueprintCallable, Category = "Friend")
	FString GetFriendSteamID(const int& number);
	UFUNCTION(BlueprintCallable, Category = "Friend")
	UTexture2D* GetFriendImage(const int& number);
	UFriendInformation* GetFriendBySteamId(const UINT64& steamID);

	// Insert player information to the slot
	void ChangeSlot(int slotNumber, const UINT64& steamID);
	// Refresh the widget of all slot
	void RefreshLobby();

	// Change Ready button state
	void SetReadyButton(const int& slot, const bool& isOn);

	// Get Local Player Slot.
	UFUNCTION(BlueprintPure)
	int32 PlayerSlotNumber();

	// Logic of clicked setting open/close
	UFUNCTION(BlueprintCallable)
	void OnClickSetting();

	// Logic of clicked friend open/close
	UFUNCTION(BlueprintCallable)
	void OnClickFriend();

	void OpenRequestInviteWG(const FString& steamID, const FString& userName);
	void OpenLobbyWGFailed(const FString& msg = "Unknown error.");

	UFUNCTION(BlueprintCallable)
	int32 GetMaxPlayer();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	virtual void PostInitializeComponents() override;

};
