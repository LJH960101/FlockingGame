// Network receive process class in Lobby.

#pragma once

#include "JHNET.h"
#include "GameFramework/Actor.h"
#include "Common/Network/CommonNetworkProcessor.h"
#include "LobbyNetworkProcessor.generated.h"

class ALobbyManager;

UCLASS()
class JHNET_API ALobbyNetworkProcessor : public ACommonNetworkProcessor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ALobbyNetworkProcessor();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	virtual void RecvProc(FReciveData& data) override;

private:

	ALobbyManager* LobbyManager;

public:	
	UPROPERTY(EditAnywhere, Category ="Lobby")
	FName GameName;

private:
	// 메세지 처리 함수

	// Room
	void Room_Info(FReciveData& data, int& cursor, int& bufLen);
	void Room_ChangeState(FReciveData& data, int& cursor, int& bufLen);

	// Lobby
	void Lobby_InviteFriendRequest(FReciveData& data, int& cursor, int& bufLen);
	void Lobby_InviteFriendFailed(FReciveData& data, int& cursor, int& bufLen);
	void Lobby_MatchAnswer(FReciveData& data, int& cursor, int& bufLen);
	void Lobby_GameStart(FReciveData& data, int& cursor, int& bufLen);

	void Reconnect(FReciveData& data, int& cursor, int& bufLen);
};
