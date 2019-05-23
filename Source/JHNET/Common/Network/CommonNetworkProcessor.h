// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

//#define DEBUG_RECV
#include "JHNET.h"
#include "GameFramework/Actor.h"
#include <memory>
#include "CommonNetworkProcessor.generated.h"

struct FReciveData;
class UHACKEDGameInstance;
class UNetworkSystem;

typedef TTuple<std::shared_ptr<char[]>, int> TPacket;
UCLASS()
class JHNET_API ACommonNetworkProcessor : public AActor
{
	GENERATED_BODY()
	
public:
	// When true, Network connection will be cancle automatically.
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bWithoutNetwork = false;

	// Sets default values for this actor's properties
	ACommonNetworkProcessor();
	virtual void PostInitializeComponents() override;

	// SEND
	void Send(const std::shared_ptr<char[]>& buf, const int& len, bool isTCP = true);

	virtual void Tick(float DeltaSeconds) override;

	// 패킷을 모아서 보낸다.
	void SendProc(float DeltaSeconds);

	// 연결 상태를 파악한다.
	void GetServerState(float DeltaSeconds);
	bool _lastIsOn = false;

protected:
	// Called when the game starts or when spawned
	virtual void RecvProc(FReciveData& data);
	UHACKEDGameInstance* GetGameInstance();
	UNetworkSystem* GetNetworkSystem();

	// Called when room is changed.
	virtual void OnRefreshRoom();

	// 서버에 접속되었을때 호출됩니다.
	virtual void OnConnected();

	virtual void BeginPlay() override;

private:
	UHACKEDGameInstance* _gameInstance;
	class UNetworkSystem* _netSystem;

	// 배열 0은 TCP, 1은 UDP로 사용한다.
	TQueue<TPacket>  _savedPackets[2];

	UPROPERTY()
	TSubclassOf<class UUserWidget> _serverConnectWGClass;
	UPROPERTY()
	class UUserWidget* _serverConnectWG = nullptr;

	// 메세지 처리 함수

	// Common
	void Common_Echo(FReciveData& data, int& cursor, int& bufLen);
	void Common_Ping(FReciveData& data, int& cursor, int& bufLen);
	void Common_RequestID(FReciveData& data, int& cursor, int& bufLen);

	void Room_RoomInfo(FReciveData& data, int& cursor, int& bufLen);
	void Reconnect(FReciveData& data, int& cursor, int& bufLen);
};
