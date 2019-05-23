/*
	Main Network System
*/

#pragma once

#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include "JHNET.h"
#include "NetworkModule/GameInfo.h"
#include "AllowWindowsPlatformTypes.h"
#include <WinSock2.h>
#include "HideWindowsPlatformTypes.h"
#include <mutex>
#include <thread>
#include "Containers/Queue.h"
#include "ReciveDataPool.h"
#include "NetworkSystem.generated.h"

class AInGameManager;

enum ESocketInitState : uint8 {
	INIT_START, // 초기화중인 상태
	INIT_SUCCESS, // 초기화 성공
	INIT_FAILED // 초기화 실패(초기화를 포기함) 또는 소켓 종료
};

UCLASS(Config=HackedNetwork)
class JHNET_API UNetworkSystem : public UObject
{
	friend class ACommonNetworkProcessor;

	GENERATED_BODY()
public:
	UNetworkSystem();
	~UNetworkSystem();

	UFUNCTION(BlueprintCallable)
	bool OnSteam();

	UFUNCTION(BlueprintCallable)
	bool OnServer();

	void GetData(FReciveData*& outData);

	void Send(const char* buf, int len, bool isTCP, bool withoutLen = false);

	void ReturnObject(FReciveData* object);

	UINT64 GetSteamID();

	UFUNCTION(BlueprintCallable)
	FString GetSteamIDofString();

	UFUNCTION(BlueprintCallable)
	void Init();
	UFUNCTION(BlueprintCallable)
	void Shutdown();

	void SetSteamID_DUBGE(const UINT64& steamID);

	// Steam
	UTexture2D* GetSteamAvartar(const UINT64 steamID);
	UFUNCTION(BlueprintCallable)
	UTexture2D* GetSteamAvartar(const FString steamID);
	FString GetSteamName(const UINT64 steamID);
	UFUNCTION(BlueprintCallable)
	FString GetSteamName(const FString steamID);
	UFUNCTION(BlueprintCallable)
	FString GetMySteamName();

	void SetSteamID(int32 slot, UINT64 steamID);
	UINT64 GetSteamID(int32 slot);

	// InGameManager
	void SetInGameManager(AInGameManager* inGameManager);
	AInGameManager* GetInGameManager();
	UFUNCTION(BlueprintCallable)
	int32 GetCurrentSlot();
	bool ConnectUDP();

	// Automatically true on reconnect.
	bool bNeedToResync = false;

	// 소켓 연결

	// 비동기 연결 시작
	void PostInitSocket(bool isReconnect = false);
	ESocketInitState GetSocketState();

	// 서버 연결을 포기한다.
	UFUNCTION(BlueprintCallable, Category = "Socket")
	void StopToInit();

private:
	UPROPERTY()
	UReciveDataPool* _dataPool;

	// Steam
	bool InitSteam();
	void ShutdownSteam();
	bool bOnSteam;
	UINT64 defaultSteamID = 0;
	UINT64 steamIds[MAX_PLAYER];

	UPROPERTY(Config)
	FString ipAdress;

	// Socket

	// 서버에 연결을 시도하는 쓰레드
	std::thread* socketInitThread;

	// 서버 연결 상태
	ESocketInitState currentInitState;

	// isReconnect는 서버가 단절되어 재접속하는지 여부를 나타냅니다.
	void InitSocket(bool isReconnect = false);
	bool InitTCP(bool isReconnect = false);
	bool InitUDP();

	std::mutex shutdownLocker;
	void ShutdownSocket();
	int RecvUDP(char* buf, int size = BUFSIZE);
	bool bOnServer;

	// UDP
	SOCKET udpSock = NULL;
	SOCKADDR_IN udpAddr;
	void UDPRecvThread();
	// Used by TCPRecvThread only buffer!! Do not use in other function.
	char udpRecvBuf[BUFSIZE];
	std::thread* hUdpThread = nullptr;

	// TCP
	SOCKET tcpSock = NULL;
	void TCPRecvThread();
	// Used by TCPRecvThread only buffer!! Do not use in other function.
	char tcpRecvBuf[BUFSIZE];
	std::thread* hTcpThread = nullptr;

	// Data saver
	std::recursive_mutex recvMutex;
	TQueue<FReciveData*> recvQueue;

	// Game System
	int32 _currentSlot = 0;

	// Managers
	AInGameManager* InGameManager;
	void SetCurrentSlot(int32 slot);

	void AddSavedData(FReciveData*& newData, FReciveData*& savedData, const char* recvBuf, const int& retval);
	void DelayData(FReciveData*& newData, FReciveData*& savedData, const int& delayIndex);
};