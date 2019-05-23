// Fill out your copyright notice in the Description page of Project Settings.

#include "NetworkSystem.h"
#include "Steamworks/Steamv139/sdk/public/steam/steam_api.h"
#include "NetworkModule/Serializer.h"
#include "NetworkModule/MyTool.h"
#include "InGame/InGameManager.h"
#include "InGame/Network/InGameNetworkProcessor.h"
#include <string>
#include <fstream>
#include <memory>
#include <process.h>

using std::mutex;
using std::shared_ptr;
using namespace MyTool;
using namespace MySerializer;

void UNetworkSystem::SetCurrentSlot(int32 slot)
{
	_currentSlot = slot;
	// If ingame manager is exist, Change the setting.
	if (InGameManager) {
		AInGameNetworkProcessor* networkProcessor = InGameManager->GetNetworkProcessor();
		networkProcessor->RefreshSlot();
	}
}

void UNetworkSystem::AddSavedData(FReciveData*& newData, FReciveData*& savedData, const char* recvBuf, const int& retval)
{
	if (!savedData) return;
	// 붙이면 한계를 초과한다면?
	if (savedData->len + retval > BUFSIZE) {
		// 수신받은 데이터 뒷부분을 때서 delay를 새로 만들어준다.
		int cursor = 0, len = 0;

		// 마지막으로 넘치는 구간을 찾아냄
		while (true) {
			len = IntDeserialize(tcpRecvBuf, &cursor);
			if (len < 0 || len > BUFSIZE) {
				JHNET_LOG(Error, "wrong len....");
				ReturnObject(savedData);
				savedData = nullptr;
				break;
			}
			// 넘쳤다면?
			if (savedData->len + cursor + len >= BUFSIZE) {
				FReciveData* newSavedData = _dataPool->GetObject();

				// 넘쳐버리는 커서부터 끝까지 newSavedData에 넣어준다.
				memcpy(newSavedData->buf, tcpRecvBuf + cursor, retval - cursor);
				newSavedData->len = retval - cursor;

				// savedData를 앞에다 붙이고,
				memcpy(newData->buf, savedData->buf, savedData->len);
				// 뒤에는 수신받은 데이터를 커서 전까지 붙여 넣는다.
				memcpy(newData->buf + savedData->len, tcpRecvBuf, cursor);
				newData->len = savedData->len + cursor;

				// savedData를 교체해준다.
				ReturnObject(savedData);
				savedData = newSavedData;

				break;
			}
			cursor += len;
		}
	}
	// 아니라면
	else {
		// 꺼내서 앞에다 붙인다.
		memcpy(newData->buf, savedData->buf, savedData->len);
		// 뒤에는 기존 데이터로
		memcpy(newData->buf + savedData->len, tcpRecvBuf, retval);
		newData->len = savedData->len + retval;
		ReturnObject(savedData);
		savedData = nullptr;
	}
}

void UNetworkSystem::DelayData(FReciveData*& newData, FReciveData*& savedData, const int& delayIndex)
{
	if (!savedData) {
		FReciveData* newSavedData = _dataPool->GetObject();
		newSavedData->len = 0;
		savedData = newSavedData;
	}
	
	// 불가능한 크기
	if (savedData->len + (newData->len - delayIndex) > BUFSIZE) {
		// 그냥 잘라버린다.
		newData->len = delayIndex;
		if (savedData->len == 0) {
			ReturnObject(savedData);
			savedData = nullptr;
		}
		JHNET_LOG_ERROR("Too many delayed buffer.. something will be ignored.. need len = %d, savedLen = %d, delayedLen = %d", 
			savedData->len + (newData->len - delayIndex), savedData->len, newData->len - delayIndex);
	}
	else {
		memcpy(savedData->buf + savedData->len, newData->buf + delayIndex, newData->len - delayIndex);
		savedData->len = savedData->len + (newData->len - delayIndex);
		newData->len = delayIndex;
	}
}

UNetworkSystem::UNetworkSystem()
{
	_dataPool = NewObject<UReciveDataPool>();

	for (int i = 0; i < MAX_PLAYER; ++i)
		steamIds[i] = 0;
}
UNetworkSystem::~UNetworkSystem()
{

}
bool UNetworkSystem::OnSteam()
{
	if (!bOnSteam)
	{
		return false;
	}
	else bOnSteam = SteamAPI_IsSteamRunning();

	return bOnSteam;
}

UTexture2D* UNetworkSystem::GetSteamAvartar(const UINT64 steamID)
{
	uint32 Width;
	uint32 Height;

	if (OnSteam())
	{
		CSteamID PlayerRawID(steamID);
		//Getting the PictureID from the SteamAPI and getting the Size with the ID
		int Picture = SteamFriends()->GetMediumFriendAvatar(PlayerRawID);
		if (Picture == 0) return nullptr;
		SteamUtils()->GetImageSize(Picture, &Width, &Height);


		if (Width > 0 && Height > 0)
		{
			//Creating the buffer "oAvatarRGBA" and then filling it with the RGBA Stream from the Steam Avatar
			BYTE *oAvatarRGBA = new BYTE[Width * Height * 4];


			//Filling the buffer with the RGBA Stream from the Steam Avatar and creating a UTextur2D to parse the RGBA Steam in
			SteamUtils()->GetImageRGBA(Picture, (uint8*)oAvatarRGBA, 4 * Height * Width * sizeof(char));

			//Swap R and B channels because for some reason the games whack
			for (uint32 i = 0; i < (Width * Height * 4); i += 4)
			{
				uint8 Temp = oAvatarRGBA[i + 0];
				oAvatarRGBA[i + 0] = oAvatarRGBA[i + 2];
				oAvatarRGBA[i + 2] = Temp;
			}

			UTexture2D* Avatar = UTexture2D::CreateTransient(Width, Height, PF_B8G8R8A8);

			//MAGIC!
			uint8* MipData = (uint8*)Avatar->PlatformData->Mips[0].BulkData.Lock(LOCK_READ_WRITE);
			FMemory::Memcpy(MipData, (void*)oAvatarRGBA, Height * Width * 4);
			Avatar->PlatformData->Mips[0].BulkData.Unlock();

			//Setting some Parameters for the Texture and finally returning it
			Avatar->PlatformData->NumSlices = 1;
			Avatar->NeverStream = true;
			//Avatar->CompressionSettings = TC_EditorIcon;

			Avatar->UpdateResource();
			delete[] oAvatarRGBA;
			return Avatar;
		}
		return nullptr;
	}
	return nullptr;
}

UTexture2D* UNetworkSystem::GetSteamAvartar(const FString steamID)
{
	return GetSteamAvartar(FStringToUINT64(steamID));
}

FString UNetworkSystem::GetSteamName(const UINT64 steamID)
{
	if (OnSteam()) {
		if (SteamFriends()->GetFriendPersonaName(steamID) == NULL) return "???";
		std::string retval = SteamFriends()->GetFriendPersonaName(steamID);
		return UTF8_TO_TCHAR(retval.c_str());
	}
	else {
		return FString::Printf(TEXT("%llu"), steamID);
	}
}

FString UNetworkSystem::GetSteamName(const FString steamID)
{
	return GetSteamName(FStringToUINT64(steamID));
}

FString UNetworkSystem::GetMySteamName()
{
	return GetSteamName(GetSteamID());
}

void UNetworkSystem::SetSteamID(int32 slot, UINT64 steamID)
{
	steamIds[slot] = steamID;
}

void UNetworkSystem::SetInGameManager(AInGameManager * inGameManager)
{
	InGameManager = inGameManager;
}

AInGameManager * UNetworkSystem::GetInGameManager()
{
	if (InGameManager == nullptr) JHNET_LOG(Error, "No InGameManager Registered.");
	return InGameManager;
}

bool UNetworkSystem::OnServer()
{
	if(GetSocketState() == INIT_SUCCESS && bOnServer) return true;
	else return false;
}

void UNetworkSystem::GetData(FReciveData*& outData)
{
	FReciveData* queueData;
	recvMutex.lock();
	if (recvQueue.IsEmpty()) queueData = nullptr;
	else recvQueue.Dequeue(queueData);
	recvMutex.unlock();
	outData = queueData;
}

void UNetworkSystem::Send(const char* buf, int len, bool isTCP, bool withoutLen /*= false*/)
{
	if (!OnServer()) return;
	if (isTCP) {
		if (withoutLen) MyTool::SendWithoutLen(tcpSock, buf, len, 0);
		else MyTool::Send(tcpSock, buf, len, 0);
	}
	else {
		if (withoutLen) MyTool::SendToWithoutLen(udpSock, buf, len, (SOCKADDR*)& udpAddr, sizeof(udpAddr));
		else MyTool::SendTo(udpSock, buf, len, (SOCKADDR *)&udpAddr, sizeof(udpAddr));
	}
}

void UNetworkSystem::ReturnObject(FReciveData* object)
{
	_dataPool->ReturnObject(object);
}

bool UNetworkSystem::InitSteam()
{
	if (SteamAPI_Init()) {
		JHNET_LOG(Warning, "Steam is Running!");
		bOnSteam = true;
	}
	else {
		JHNET_LOG(Warning, "Steam not working.");
		bOnSteam = false;
	}
	return bOnSteam;
}

void UNetworkSystem::ShutdownSteam()
{
	if (bOnSteam) {
		JHNET_LOG(Warning, "Steam shutdown.");
		SteamAPI_Shutdown();
		bOnSteam = false;
	}
}

void UNetworkSystem::InitSocket(bool isReconnect) {
	if (bOnServer) return;

	/*WSAData wsa;
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
		JHNET_LOG_ERROR("InitSocket : WSAStartup error.");
		currentInitState = INIT_FAILED;
		return;
	}*/

	bOnServer = true;
	JHNET_LOG_WARNING("TCP Init Start.");
	if (!InitTCP(isReconnect)) {
		ShutdownSocket();
		bOnServer = false;
		JHNET_LOG(Error, "Failed to connect TCP");
		currentInitState = INIT_FAILED;
		return;
	}
	JHNET_LOG_WARNING("UDP Init Start.");
	if (!InitUDP()) {
		ShutdownSocket();
		bOnServer = false;
		JHNET_LOG(Error, "Failed to connect UDP");
		currentInitState = INIT_FAILED;
		return;
	}

	currentInitState = INIT_SUCCESS;
	JHNET_LOG_WARNING("Socket Init Success!!");
}

bool UNetworkSystem::InitTCP(bool isReconnect)
{
	if (ipAdress == "") ipAdress = "127.0.0.1";
	tcpSock = NULL;
	// Addr Define
	SOCKADDR_IN serveraddr;
	ZeroMemory(&serveraddr, sizeof(serveraddr));
	serveraddr.sin_family = AF_INET;

	// Get IP by Absolute Text file
	std::ifstream ipText(*(FPaths::ProjectDir() + FString(IP_TEXT_PATH)));
	std::ifstream ipText2(*(FPaths::RootDir() + FString(IP_TEXT_PATH)));
	std::ifstream ipText3(*(FString("C:\\") + FString(IP_TEXT_PATH)));
	if (ipText.is_open()) {
		char buf[100];
		ipText >> buf;
		serveraddr.sin_addr.s_addr = inet_addr(buf);
	}
	else if (ipText2.is_open())
	{
		char buf[100];
		ipText2 >> buf;
		serveraddr.sin_addr.s_addr = inet_addr(buf);
	}
	else if (ipText3.is_open())
	{
		char buf[100];
		ipText3 >> buf;
		serveraddr.sin_addr.s_addr = inet_addr(buf);
	}
	else serveraddr.sin_addr.s_addr = inet_addr(TCHAR_TO_ANSI(*ipAdress));

	if (ipText.is_open()) ipText.close();
	if (ipText2.is_open()) ipText2.close();
	if (ipText3.is_open()) ipText3.close();

	serveraddr.sin_port = htons(TCP_SERVER_PORT);

	SOCKET client_sock = socket(AF_INET, SOCK_STREAM, 0);
	if (client_sock == INVALID_SOCKET) {
		JHNET_LOG_ERROR("SOCKET()");
		return false;
	}

	int optval = 50; // Timeout : 50ms
	if (setsockopt(client_sock, SOL_SOCKET, SO_RCVTIMEO, (char*)&optval, sizeof(optval)) == SOCKET_ERROR) {
		JHNET_LOG_ERROR("sockopt(RCVTIMEO)");
		return false;
	}
	optval = true; // Turn off Nagle Algorithm
	if (setsockopt(client_sock, IPPROTO_TCP, TCP_NODELAY, (char*)&optval, sizeof(optval)) == SOCKET_ERROR) {
		JHNET_LOG_ERROR("sockopt(NODELAY)");
		return false;
	}
	optval = BUFSIZE; // Set Buf size
	if (setsockopt(client_sock, SOL_SOCKET, SO_RCVBUF, (char*)&optval, sizeof(optval)) == SOCKET_ERROR) {
		JHNET_LOG_ERROR("sockopt(RCVBUF)");
		return false;
	}


	while (GetSocketState() != INIT_FAILED) {
		JHNET_LOG_WARNING("Try to connect TCP.");
		if (connect(client_sock, (SOCKADDR *)&serveraddr, sizeof(serveraddr)) != INVALID_SOCKET) break;
	}
	if (GetSocketState() == INIT_FAILED) {
		JHNET_LOG_ERROR("Failed by SOCKET STATE change to failed.");
		return false;
	}

	tcpSock = client_sock;
	if (isReconnect) {
		// 서버가 단절되서 다시 재접속 했다면 현재 방을 복구하라고 명령한다.
		UINT64 steamID = GetSteamID();
		char buf[sizeof(EMessageType) + sizeof(UINT64) * (MAX_PLAYER + 1) + sizeof(bool)];
		int len = SerializeWithEnum(EMessageType::C_RECONNECT_SERVER, nullptr, 0, buf);

		FString levelName = UGameplayStatics::GetCurrentLevelName(GetWorld());

		if (levelName == TEXT("Lobby"))
			len += BoolSerialize(buf + len, false);
		else
			len += BoolSerialize(buf + len, true);

		len += UInt64Serialize(buf + len, GetSteamID());

		for(int i=0; i<MAX_PLAYER; ++i)
			len += UInt64Serialize(buf + len, GetSteamID(i));

		// 여기서는 서버가 완전히 켜져있는 상태가 아니므로 멤버가 아닌 직접적인 함수로 통신한다.
		MyTool::Send(tcpSock, buf, len, 0);
	}
	else {
		// Reconnect가 아니라면 ID를 알려주고 넘어간다.
		UINT64 steamID = GetSteamID();
		char sendBuf[sizeof(EMessageType) + sizeof(UINT64)], idBuf[sizeof(UINT64)];
		int len = UInt64Serialize(idBuf, steamID);
		int totalLen = SerializeWithEnum(EMessageType::C_Common_AnswerId, idBuf, len, sendBuf);

		// 여기서는 서버가 완전히 켜져있는 상태가 아니므로 멤버가 아닌 직접적인 함수로 통신한다.
		MyTool::Send(tcpSock, sendBuf, totalLen, 0);
	}

	hTcpThread = new std::thread(&UNetworkSystem::TCPRecvThread, this);
	JHNET_LOG(Warning, "TCP Init Success");
	return true;
}

bool UNetworkSystem::InitUDP()
{
	if (ipAdress == "") ipAdress = "127.0.0.1";
	udpSock = NULL;

	// Create Socket
	SOCKET sock = socket(AF_INET, SOCK_DGRAM, 0);
	if (sock == INVALID_SOCKET) {
		JHNET_LOG(Error, "UDP :: Can't create sock.");
		return false;
	}

	// Addr Define
	SOCKADDR_IN serveraddr;
	ZeroMemory(&serveraddr, sizeof(serveraddr));
	serveraddr.sin_family = AF_INET;

	// Get IP by Absolute Text file
	std::ifstream ipText(*(FPaths::ProjectDir() + FString(IP_TEXT_PATH)));
	std::ifstream ipText2(*(FPaths::RootDir() + FString(IP_TEXT_PATH)));
	std::ifstream ipText3(*(FString("C:\\") + FString(IP_TEXT_PATH)));
	if (ipText.is_open()) {
		char buf[100];
		ipText >> buf;
		serveraddr.sin_addr.s_addr = inet_addr(buf);
	}
	else if (ipText2.is_open())
	{
		char buf[100];
		ipText2 >> buf;
		serveraddr.sin_addr.s_addr = inet_addr(buf);
	}
	else if (ipText3.is_open())
	{
		char buf[100];
		ipText3 >> buf;
		serveraddr.sin_addr.s_addr = inet_addr(buf);
	}
	else serveraddr.sin_addr.s_addr = inet_addr(TCHAR_TO_ANSI(*ipAdress));

	if (ipText.is_open()) ipText.close();
	if (ipText2.is_open()) ipText2.close();
	if (ipText3.is_open()) ipText3.close();

	serveraddr.sin_port = htons(UDP_SERVER_PORT);

	udpSock = sock;
	udpAddr = serveraddr;

	// try to connect
	if (!ConnectUDP()) {
		JHNET_LOG(Error, "UDP :: Can't connect.");
		return false;
	}
	int optval = BUFSIZE; // Set Buf size
	if (setsockopt(udpSock, SOL_SOCKET, SO_RCVBUF, (char*)&optval, sizeof(optval)) == SOCKET_ERROR) {
		return false;
	}

	hUdpThread = new std::thread(&UNetworkSystem::UDPRecvThread, this);
	return true;
}

void UNetworkSystem::PostInitSocket(bool isReconnect /*= false*/)
{
	if (socketInitThread) {
		currentInitState = INIT_FAILED;
		socketInitThread->join();
		delete socketInitThread;
		socketInitThread = nullptr;
	}
	currentInitState = INIT_START;
	socketInitThread = new std::thread(&UNetworkSystem::InitSocket, this, isReconnect);
}

ESocketInitState UNetworkSystem::GetSocketState()
{
	return currentInitState;
}

void UNetworkSystem::StopToInit()
{
	if (socketInitThread) {
		currentInitState = INIT_FAILED;
		socketInitThread->join();
		delete socketInitThread;
		socketInitThread = nullptr;
	}
	currentInitState = INIT_FAILED;
}

void UNetworkSystem::ShutdownSocket()
{
	if (shutdownLocker.try_lock()) {
		currentInitState = INIT_FAILED;
		bOnServer = false;
		if (socketInitThread != nullptr && socketInitThread->get_id() != std::this_thread::get_id()) {
			socketInitThread->join();
			delete socketInitThread;
			socketInitThread = nullptr;
		}
		if (hTcpThread != nullptr) {
			hTcpThread->join();
			delete hTcpThread;
			hTcpThread = nullptr;
		}
		if (hUdpThread != nullptr) {
			hUdpThread->join();
			delete hUdpThread;
			hUdpThread = nullptr;

		}
		if (tcpSock != NULL) {
			closesocket(tcpSock);
			tcpSock = NULL;
		}
		if (udpSock != NULL) {
			closesocket(udpSock);
			udpSock = NULL;
		}
		//WSACleanup();
		JHNET_LOG(Warning, "Socket Shutdown Success");
		shutdownLocker.unlock();
	}
}

int UNetworkSystem::RecvUDP(char* buf, int size)
{
	SOCKADDR_IN peeraddr;
	int addrlen = sizeof(peeraddr);
	return recvfrom(udpSock, buf, size, 0, (SOCKADDR*)&peeraddr, &addrlen);
}

unsigned long mix(unsigned long a, unsigned long b, unsigned long c)
{
	a = a - b;  a = a - c;  a = a ^ (c >> 13);
	b = b - c;  b = b - a;  b = b ^ (a << 8);
	c = c - a;  c = c - b;  c = c ^ (b >> 13);
	a = a - b;  a = a - c;  a = a ^ (c >> 12);
	b = b - c;  b = b - a;  b = b ^ (a << 16);
	c = c - a;  c = c - b;  c = c ^ (b >> 5);
	a = a - b;  a = a - c;  a = a ^ (c >> 3);
	b = b - c;  b = b - a;  b = b ^ (a << 10);
	c = c - a;  c = c - b;  c = c ^ (b >> 15);
	return c;
}
UINT64 UNetworkSystem::GetSteamID()
{
	if (OnSteam()) {
		return SteamUser()->GetSteamID().ConvertToUint64();
	}
	else {
		if (defaultSteamID == 0) {
			unsigned long seed = mix(clock(), time(NULL), _getpid());
			srand(seed);
			SetSteamID_DUBGE(FMath::RandRange(1, 1000000));
		}
		return defaultSteamID;
	}
}

UINT64 UNetworkSystem::GetSteamID(int32 slot)
{
	return steamIds[slot];
}

FString UNetworkSystem::GetSteamIDofString()
{
	return UINT64ToFString(GetSteamID());
}

int32 UNetworkSystem::GetCurrentSlot()
{
	return _currentSlot;
}

bool UNetworkSystem::ConnectUDP()
{
	int retval, optval;
	optval = 50; // Timeout : 50ms
	retval = setsockopt(udpSock, SOL_SOCKET, SO_RCVTIMEO, (char*)&optval, sizeof(optval));
	if (retval == SOCKET_ERROR) {
		JHNET_LOG(Error, "UDP :: Can't set recvto sock.");
		return false;
	}

	// UDP 연결을 시도한다.
	while(GetSocketState() == INIT_START) {
		JHNET_LOG_WARNING("Try to connect UDP.");
		char buf[sizeof(EMessageType) + sizeof(UINT64)];
		SerializeEnum(EMessageType::C_UDP_Reg, buf);
		UInt64Serialize(buf + sizeof(EMessageType), GetSteamID());
		retval = MyTool::SendTo(udpSock, buf, sizeof(EMessageType) + sizeof(UINT64), (SOCKADDR *)&udpAddr, sizeof(udpAddr));
		if (retval == SOCKET_ERROR) {
			JHNET_LOG(Warning, "UDP send to :: %ld", WSAGetLastError());
			continue;
		}

		retval = RecvUDP(udpRecvBuf, BUFSIZE);
		if (retval != 0) {
			if (GetEnum(udpRecvBuf + sizeof(int)) == EMessageType::S_UDP_Response) {
				JHNET_LOG(Warning, "UDP :: Success!!!");
				break;
			}
		}
		else if (retval == SOCKET_ERROR) {
			int errCode = WSAGetLastError();
			if(errCode != WSAETIMEDOUT) JHNET_LOG(Warning, "UDP RecvUDP FAILED :: %ld", WSAGetLastError());
			continue;
		}
	}
	if (GetSocketState() != INIT_START) return false;

	optval = 50; // Timeout : 50ms
	retval = setsockopt(udpSock, SOL_SOCKET, SO_RCVTIMEO, (char*)&optval, sizeof(optval));
	if (retval == SOCKET_ERROR) {
		JHNET_LOG(Error, "UDP :: Can't set recvto sock.");
		return false;
	}

	return true;
}

void UNetworkSystem::UDPRecvThread()
{
	FReciveData* savedData = nullptr;
	while (bOnServer && GetSocketState() != INIT_FAILED) {
		int retval = RecvUDP(udpRecvBuf, BUFSIZE);
		if (retval == SOCKET_ERROR || retval == 0) {
			if (retval == SOCKET_ERROR) {
				int errNum = WSAGetLastError();
				if (errNum != WSAETIMEDOUT) JHNET_LOG_ERROR("Socket Error! : %d", errNum);
			}
			continue;
		}

		// 수신 처리. 잘렸는지 딜레이 됬는지 처리 후 메세지큐에 넣음.
		if (retval > BUFSIZE) {
			JHNET_LOG(Error, "Recv : Too long buf size..");
			continue;
		}
		FReciveData* newData = _dataPool->GetObject();

		// 딜레이된 데이터가 존재?
		if (savedData) {
			//JHNET_LOG_WARNING("Delay pop! savedLen = %d, newLen = %d", savedData->len, retval);
			AddSavedData(newData, savedData, udpRecvBuf, retval);
		}
		// 없다면 새로운 데이터로 채웁니다.
		else {
			memcpy(newData->buf, udpRecvBuf, retval);
			newData->len = retval;
		}

		// 버퍼가 잘려서 들어오는지 판단합니다.
		int cursor = 0;
		int bufLen;
		while (cursor < newData->len) {
			bufLen = IntDeserialize(newData->buf, &cursor);

			// 이상한 길이가 들어옴...
			if (bufLen < 0 || bufLen > BUFSIZE) {
				newData->len = 0;
				break;
			}

			// 잘려서 들어왔는가?
			if (cursor + bufLen > newData->len) {
				cursor -= (int)sizeof(int32_t);
				//JHNET_LOG_WARNING("DelayData! cursor:%d, buflen:%d, delayedLen:%d", cursor, bufLen, bufLen - cursor);
				DelayData(newData, savedData, cursor);
				break;
			}

			cursor += bufLen;
		}

		recvMutex.lock();
		recvQueue.Enqueue(newData);
		recvMutex.unlock();
	}
}

void UNetworkSystem::TCPRecvThread()
{
	FReciveData* savedData = nullptr;
	while (bOnServer && GetSocketState() != INIT_FAILED) {
		int retval = recv(tcpSock, tcpRecvBuf, BUFSIZE, 0);
		int lastError = 0;
		
		// 오류가 발생했다면 오류코드를 미리 알아본다.
		if (retval == SOCKET_ERROR) {
			lastError = WSAGetLastError();
		}
		// 단절됬으면 재접속시도한다.
		if (retval == 0 || lastError == WSAECONNRESET) {
			JHNET_LOG_ERROR("Server has been disconnected!!!");

			if (GetSocketState() == INIT_SUCCESS) {
				std::thread* myThread = hTcpThread;
				hTcpThread = nullptr;

				// 소켓을 제거 한다.
				ShutdownSocket();

				// 소켓을 다시 활성화 한다.
				PostInitSocket(true);

				myThread->detach();
				delete myThread;
			}

			return;
		}
		// 에러는 무시한다.
		if (lastError != 0) {
			if(lastError != WSAETIMEDOUT) JHNET_LOG_ERROR("Socket Error!! %d", WSAGetLastError());
			continue;
		}

		// 잘못된 크기
		if (retval > BUFSIZE) {
			JHNET_LOG(Error, "Recv : Too long buf size..");
			continue;
		}

		FReciveData* newData = _dataPool->GetObject();

		// 딜레이된 데이터가 존재?
		if (savedData) {
			//JHNET_LOG_WARNING("Delay pop! savedLen = %d, newLen = %d", savedData->len, retval);
			AddSavedData(newData, savedData, tcpRecvBuf, retval);
		}
		// 없다면 새로운 데이터로 채웁니다.
		else {
			memcpy(newData->buf, tcpRecvBuf, retval);
			newData->len = retval;
		}

		// 버퍼가 잘려서 들어오는지 판단합니다.
		int cursor = 0;
		int bufLen;
		while (cursor < newData->len) {
			bufLen = IntDeserialize(newData->buf, &cursor);

			// 이상한 길이가 들어옴...
			if (bufLen < 0 || bufLen > BUFSIZE) {
				newData->len = 0;
				break;
			}

			// 잘려서 들어왔는가?
			if (cursor + bufLen > newData->len) {
				cursor -= (int)sizeof(int32_t);
				//JHNET_LOG_WARNING("DelayData! cursor:%d, buflen:%d, delayedLen:%d", cursor, bufLen, newData->len - cursor);
				DelayData(newData, savedData, cursor);
				break;
			}

			cursor += bufLen;
		}

		recvMutex.lock();
		recvQueue.Enqueue(newData);
		recvMutex.unlock();
	}
}

void UNetworkSystem::Init() {
	if (!InitSteam()) {
		JHNET_LOG(Warning, "Steam Init Failed");
	}
	PostInitSocket();
}

void UNetworkSystem::Shutdown()
{
	JHNET_LOG(Warning, "SHUTDOWN");
	ShutdownSteam();
	ShutdownSocket();
}

void UNetworkSystem::SetSteamID_DUBGE(const UINT64 & steamID)
{
	if (defaultSteamID == 0) defaultSteamID = steamID;
	else {
		JHNET_LOG(Warning, "Already have fake steam ID!!");
		return;
	}
	ShutdownSteam();
}