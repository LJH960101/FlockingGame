// Fill out your copyright notice in the Description page of Project Settings.

#include "CommonNetworkProcessor.h"
#include "Core/HACKEDGameInstance.h"
#include "Core/Network/NetworkSystem.h"
#include "NetworkModule/Serializer.h"
#include "NetworkModule/GameInfo.h"
#include "Blueprint/UserWidget.h"
#include "memory"

using namespace MySerializer;
using std::shared_ptr;

// Sets default values
ACommonNetworkProcessor::ACommonNetworkProcessor()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	static ConstructorHelpers::FClassFinder<UUserWidget> ServerConnectWidget(TEXT("WidgetBlueprint'/Game/Blueprint/UI/Common/WB_Connect.WB_Connect_C'"));
	if (ServerConnectWidget.Succeeded()) {
		_serverConnectWGClass = ServerConnectWidget.Class;
	}
}

UHACKEDGameInstance* ACommonNetworkProcessor::GetGameInstance() {
	return _gameInstance;
}

UNetworkSystem* ACommonNetworkProcessor::GetNetworkSystem()
{
	return _netSystem;
}

// Called when the game starts or when spawned
void ACommonNetworkProcessor::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	_gameInstance = Cast<UHACKEDGameInstance>(AActor::GetGameInstance());
	CHECK(_gameInstance);
	_netSystem = _gameInstance->GetNetworkSystem();
	CHECK(_netSystem);

	if (_serverConnectWGClass) {
		if (!_serverConnectWG) _serverConnectWG = CreateWidget<UUserWidget>(GetWorld(), _serverConnectWGClass);
		if (_serverConnectWG && !_serverConnectWG->IsInViewport()) _serverConnectWG->AddToViewport(1000);
	}
}

void ACommonNetworkProcessor::Send(const std::shared_ptr<char[]>& buf, const int& len, bool isTCP)
{
	// 연결이 안되어있을 경우에는 보내지 않는다.
	if (!_gameInstance->GetNetworkSystem()->OnServer()) return;
	if (isTCP) {
		_savedPackets[0].Enqueue(TPacket(buf, len));
	}
	else {
		_savedPackets[1].Enqueue(TPacket(buf, len));
	}
}

void ACommonNetworkProcessor::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	CHECK(_gameInstance);
	if(_gameInstance->GetNetworkSystem()->OnServer()) SendProc(DeltaSeconds);
	GetServerState(DeltaSeconds);

	while (true)
	{
		FReciveData* newData;
		_gameInstance->GetNetworkSystem()->GetData(newData);
		if (newData == nullptr) break;
		RecvProc(*newData);
		_gameInstance->GetNetworkSystem()->ReturnObject(newData);
	}
}

void ACommonNetworkProcessor::OnConnected()
{

}

void ACommonNetworkProcessor::BeginPlay()
{
	Super::BeginPlay();

	if (bWithoutNetwork) _gameInstance->GetNetworkSystem()->StopToInit();
	_netSystem = _gameInstance->GetNetworkSystem();
}

void ACommonNetworkProcessor::SendProc(float DeltaSeconds)
{
	// i = 0 -> TCP
	// i = 1 -> UDP
	for (int i = 0; i <= 1; ++i) {
		// 1틱 동안 쌓인 패킷을 합쳐서 보낸다.
		TQueue<TPacket> _tempQueue;

		int allLen = 0; // 총 len 수, MAX_BUF를 넘어선 안된다.
		int packetCount = 0;
		while (!_savedPackets[i].IsEmpty()) {
			TPacket packet;
			_savedPackets[i].Peek(packet);
			// 1회 버퍼 한도를 넘었다면 더이상 담지 않는다.
			if (allLen + packet.Get<1>() +
				((packetCount + 5) * sizeof(int))
				>= BUFSIZE) {
				LOG_WARNING("Buffer limit over!!");
				break;
			}
			else {
				// 넘지 않았다면 저장.
				allLen += packet.Get<1>();
				++packetCount;
				_savedPackets[i].Pop();
				_tempQueue.Enqueue(packet);
			}
		}

		// 저장한 큐의 내용을 모두 합친다.
		shared_ptr<char[]> sendBuf(new char[BUFSIZE]);
		char* pSendBuf = sendBuf.get();
		int cursor = 0;
		while (!_tempQueue.IsEmpty()) {
			TPacket packet;
			_tempQueue.Dequeue(packet);

			// 큐의 내용에 패킷 크기를 합쳐서 최종 버퍼에 합친다.
			cursor += IntSerialize(pSendBuf + cursor, packet.Get<1>());
			memcpy(pSendBuf + cursor, packet.Get<0>().get(), packet.Get<1>());
			cursor += packet.Get<1>();
			if (cursor >= BUFSIZE ||
				cursor > allLen + sizeof(int) * packetCount) {
				LOG(Error, "Critical Error : Not matched packet...");
				return;
			}
		}

		// 합친 버퍼를 전송한다.
		if (cursor > 0) {
			if (i == 0) _gameInstance->GetNetworkSystem()->Send(sendBuf.get(), cursor, true, true);
			else _gameInstance->GetNetworkSystem()->Send(sendBuf.get(), cursor, false, true);
		}
	}
}

void ACommonNetworkProcessor::GetServerState(float DeltaSeconds)
{
	CHECK(_gameInstance);

	if (_netSystem) {
		ESocketInitState state = _netSystem->GetSocketState();
		switch (state) {
		case INIT_START:
			if (_serverConnectWG && !_serverConnectWG->IsInViewport()) _serverConnectWG->AddToViewport(1000);
			_lastIsOn = false;
			UGameplayStatics::SetGlobalTimeDilation(GetWorld(), 0.0001f);
			break;
		case INIT_SUCCESS:
			if (_serverConnectWG && _serverConnectWG->IsInViewport()) _serverConnectWG->RemoveFromParent();
			if (!_lastIsOn) {
				OnConnected();
				_lastIsOn = true;
				UGameplayStatics::SetGlobalTimeDilation(GetWorld(), 1.0f);
			}
			break;
		case INIT_FAILED:
			if (_serverConnectWG && _serverConnectWG->IsInViewport()) _serverConnectWG->RemoveFromParent();
			_lastIsOn = false;
			UGameplayStatics::SetGlobalTimeDilation(GetWorld(), 1.0f);
			break;
		default:
			LOG_ERROR("Unknown state.");
			break;
		}
	}
}

void ACommonNetworkProcessor::RecvProc(FReciveData& data)
{
	CHECK(_gameInstance);

	int cursor = 0;
	while (cursor < data.len) {
		CHECK(cursor >= 0 && cursor <= BUFSIZE);
		int bufLen = IntDeserialize(data.buf, &cursor) - sizeof(EMessageType);
		if (!(bufLen >= 0 && bufLen <= BUFSIZE)) {
			LOG_ERROR("ASSERTION : 'bufLen >= 0 && bufLen <= BUFSIZE' buflen = %d, cursor = %d", bufLen, cursor)
			return;
		}
		EMessageType type = GetEnum(data.buf, &cursor);
		switch (type)
		{
		case EMessageType::COMMON_ECHO:
		{
			Common_Echo(data, cursor, bufLen);
			break;
		}
		case EMessageType::COMMON_PING:
		{
			Common_Ping(data, cursor, bufLen);
			break;
		}
		case EMessageType::S_Common_RequestId:
		{
			Common_RequestID(data, cursor, bufLen);
			break;
		}
		case EMessageType::S_Room_Info:
		{
			Room_RoomInfo(data, cursor, bufLen);
			break;
		}
		case EMessageType::S_RECONNECT:
		{
			Reconnect(data, cursor, bufLen);
			break;
		}
		default:
		{
			cursor += bufLen;
			break;
		}
		}
	}
}

void ACommonNetworkProcessor::OnRefreshRoom()
{

}

void ACommonNetworkProcessor::Common_Echo(FReciveData& data, int& cursor, int& bufLen)
{
	FSerializableString res = StringDeserialize(data.buf, &cursor);
	LOG(Warning, "%s", res.buf);
}

void ACommonNetworkProcessor::Common_Ping(FReciveData& data, int& cursor, int& bufLen)
{
#ifdef DEBUG_RECV
	LOG(Warning, "recv : COMMON_PING");
#endif
	shared_ptr<char[]> newBuf(new char[sizeof(EMessageType)]);
	int len = SerializeEnum(EMessageType::COMMON_PING, newBuf.get());
	Send(newBuf, len, true);
}

void ACommonNetworkProcessor::Common_RequestID(FReciveData& data, int& cursor, int& bufLen)
{
#ifdef DEBUG_RECV
	LOG(Warning, "recv : S_Common_RequestId");
#endif
	UINT64 steamID = _gameInstance->GetNetworkSystem()->GetSteamID();
	if (steamID == 0) return;
	shared_ptr<char[]> sendBuf(new char[sizeof(EMessageType) + sizeof(UINT64)]);
	shared_ptr<char[]> idBuf(new char[sizeof(UINT64)]);
	int len = UInt64Serialize(idBuf.get(), steamID);
	int totalLen = SerializeWithEnum(EMessageType::C_Common_AnswerId, idBuf.get(), len, sendBuf.get());
	Send(sendBuf, totalLen, true);
}

void ACommonNetworkProcessor::Room_RoomInfo(FReciveData& data, int& cursor, int& bufLen)
{
	// Get 4 UINT64, find my slot and registered.
	UINT64 slot;
	for (int i = 0; i < MAX_PLAYER; ++i) {
		slot = UInt64Deserialize(data.buf, &cursor);
		_netSystem->SetSteamID(i, slot);

		// 나의 슬롯이라면 SetCurrentSlot 호출.
		if (slot == _netSystem->GetSteamID())
			_netSystem->SetCurrentSlot(i);
	}
	OnRefreshRoom();
}

void ACommonNetworkProcessor::Reconnect(FReciveData& data, int& cursor, int& bufLen)
{
	_netSystem->bNeedToResync = true;
	UINT64 slot;
	for (int i = 0; i < MAX_PLAYER; ++i) {
		slot = UInt64Deserialize(data.buf, &cursor);
		if (slot == _netSystem->GetSteamID())
		{
			_netSystem->SetCurrentSlot(i);
		}
	}
	OnRefreshRoom();
}
