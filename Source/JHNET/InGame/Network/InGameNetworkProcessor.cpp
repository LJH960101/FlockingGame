// Fill out your copyright notice in the Description page of Project Settings.
//#define DEBUG_RPC
#include "InGameNetworkProcessor.h"
#include "Core/JHNETGameInstance.h"
#include "Core/Network/NetworkSystem.h"
#include "InGame/InGameManager.h"
#include "NetworkModule/Serializer.h"
#include "NetworkModule/MyTool.h"
#include "NetworkModule/GameInfo.h"
#include "Blueprint/UserWidget.h"
#include "NetworkModule/Serializer.h"
#include "InGame/Interface/Networkable.h"

using namespace MyTool;
using namespace std;
using namespace MySerializer;

// Sets default values
AInGameNetworkProcessor::AInGameNetworkProcessor()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	NetBaseCP = CreateDefaultSubobject<UNetworkBaseCP>(TEXT("Network Base"));
	_slotAndName.Init(TTuple<bool,FString>(false,""), MAX_PLAYER);
	
	static ConstructorHelpers::FClassFinder<UUserWidget> DisConnectWidget(TEXT("WidgetBlueprint'/Game/NetworkUI/WB_Connect.WB_Connect_C'"));
	if (DisConnectWidget.Succeeded()) {
		_disconnectWGClass = DisConnectWidget.Class;
	}
}

// FString name, int slot
void AInGameNetworkProcessor::_RPCSetSlotSteamName(FString name, int slot)
{
	RPC(NetBaseCP, AInGameNetworkProcessor, _RPCSetSlotSteamName, ENetRPCType::MULTICAST, true, name, slot);
	if(slot >=0 && slot <MAX_PLAYER){
		_slotAndName[slot] = TTuple<bool, FString>(true, name);
	}
}

void AInGameNetworkProcessor::_SetSlotSteamName()
{
	_RPCSetSlotSteamName(GetMyName(), GetCurrentSlot());
}

void AInGameNetworkProcessor::_OnRefreshSlot(bool isMaster)
{
	for (auto i : _networkActors) {
		AActor* actor = i.Value;
		if (actor &&
			actor->GetClass()->ImplementsInterface(UNetworkable::StaticClass())) {
			auto networkableActor = Cast<INetworkable>(actor);
			if (networkableActor) networkableActor->OnSlotChange(isMaster);
		}
	}
}

void AInGameNetworkProcessor::OnConnected()
{
	// Not OnSteam But OnServer, Try force Join to game room.
	if (_networkSystem && _networkSystem->OnServer() && !_networkSystem->OnSteam()) {
		EnterToGame_DEBUG();
		LOG(Warning, "Try Force Game Join...");
	}
}

AInGameNetworkProcessor::~AInGameNetworkProcessor()
{
	_savedList.Empty();
}

void AInGameNetworkProcessor::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	_gameInstance = GetGameInstance();
	CHECK(_gameInstance);
	_networkSystem = _gameInstance->GetNetworkSystem();
	CHECK(_networkSystem);
	InGameManager = _gameInstance->GetInGameManager();
	CHECK(InGameManager);
}

void AInGameNetworkProcessor::BeginPlay()
{
	Super::BeginPlay();
	BindRPCFunction(NetBaseCP, AInGameNetworkProcessor, _RPCSetSlotSteamName);
	if (_networkSystem && _networkSystem->OnServer()) {
		OnRefreshRoom();
	}
	GetWorldTimerManager().SetTimer(_reconnectCheckTimerHandle, this, &AInGameNetworkProcessor::_ReconnectCheck, 0.1f, false);
}

void AInGameNetworkProcessor::OnRefreshRoom()
{
	Super::OnRefreshRoom();
	_SetSlotSteamName();
}

void AInGameNetworkProcessor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// Object에 도달하지 않는 메세지를 다시 처리한다.
	for (int i = 0; i < _savedList.Num(); ++i) {
		FReplicationData& data = _savedList[i];
		if (CheckSavedObject(data, DeltaTime)) {
			if (data.delayTime >= MAX_SAVED_TIME) {
				LOG(Error, "data removed by timeout... : object : %s, usage : %s", *(data.objectHandle), *(data.usageHandle));
			}
			_savedList.RemoveAt(i);
			--i;
			continue;
		}
	}
}

bool AInGameNetworkProcessor::CheckSavedObject(FReplicationData& data, float deltaTime)
{
	data.delayTime += deltaTime;
	if (data.delayTime >= MAX_SAVED_TIME) return true;

	return SendToObject(data);
}

bool AInGameNetworkProcessor::SendToObject(FReplicationData & data)
{
	AActor** targetObject = _networkActors.Find(data.objectHandle);
	if (targetObject == nullptr || (*targetObject) == nullptr || !((*targetObject)->HasActorBegunPlay())) {
		//LOG(Error, "Not exist object... : Object Name %s, UsageHandle %s", *(data.objectHandle), *(data.usageHandle));
		return false;
	}
	// Get network base cp
	auto cp = (*targetObject)->GetComponentByClass(UNetworkBaseCP::StaticClass());
	if (cp == nullptr) {
		LOG(Error, "Can't cast to NetBaseCP");
		return false;
	}
	UNetworkBaseCP* networkCP = Cast<UNetworkBaseCP>(cp);
	if (networkCP == nullptr) {
		LOG(Error, "Can't cast to NetBaseCP");
		return false;
	}

	if (!networkCP->OnSync()) return false;
	if(data.isRPC)	networkCP->ExecutedNetFunc(data.usageHandle, data.len, data.buf.get());
	else	networkCP->RecvSyncVar(data.usageHandle, data.len, data.buf.get());
	return true;
}

void AInGameNetworkProcessor::_Disconnect(int32 slot)
{
	if(slot < 0 || slot >= MAX_PLAYER) return;
	if (!_disconnectWG) _disconnectWG = CreateWidget<UUserWidget>(GetWorld(), _disconnectWGClass);
	if (_disconnectWG && !_disconnectWG->IsInViewport()) _disconnectWG->AddToViewport();
	_slotAndName[slot] = TTuple<bool, FString>(false, _slotAndName[slot].Get<1>());
	UGameplayStatics::SetGlobalTimeDilation(GetWorld(), 0.0001f);
	GetWorld()->GetFirstPlayerController()->bShowMouseCursor = true;
	for (auto i : _networkActors) {
		AActor* actor = i.Value;
		if (actor &&
			actor->GetClass()->ImplementsInterface(UNetworkable::StaticClass())) {
			auto networkableActor = Cast<INetworkable>(actor);
			if (networkableActor) networkableActor->Disconnected();
		}
	}
}

void AInGameNetworkProcessor::_Reconnected(int32 slot)
{
	if(slot < 0 || slot >= MAX_PLAYER) return;
	_slotAndName[slot] = TTuple<bool, FString>(true, _slotAndName[slot].Get<1>());
	if(GetPlayerNumbs() == MAX_PLAYER){
		// All user has been reconnected.
		if (_disconnectWG && _disconnectWG->IsInViewport()) _disconnectWG->RemoveFromParent();
		UGameplayStatics::SetGlobalTimeDilation(GetWorld(), 1.0f);
		GetWorld()->GetFirstPlayerController()->bShowMouseCursor = false;
	}
	for (auto i : _networkActors) {
		AActor* actor = i.Value;
		if (actor &&
			actor->HasActorBegunPlay() &&
			actor->GetClass()->ImplementsInterface(UNetworkable::StaticClass())) {
			auto networkableActor = Cast<INetworkable>(actor);
			if(networkableActor) networkableActor->Reconnected();
		}
	}
}

void AInGameNetworkProcessor::_ReconnectProcess()
{
	if (_networkSystem) _networkSystem->bNeedToResync = false;
	for (auto i : _networkActors) {
		AActor* actor = i.Value;
		if (actor &&
			actor->GetClass()->ImplementsInterface(UNetworkable::StaticClass())) {
			auto networkableActor = Cast<INetworkable>(actor);
			if (networkableActor) networkableActor->Reconnect();
		}
	}
}

void AInGameNetworkProcessor::_ReconnectCheck()
{
	if (_networkSystem && _networkSystem->bNeedToResync) {
		_ReconnectProcess();
	}
}

UNetworkSystem * AInGameNetworkProcessor::GetNetworkSystem()
{
	return _networkSystem;
}

bool AInGameNetworkProcessor::AddObject(AActor * networkActor)
{
	UActorComponent* cp = networkActor->GetComponentByClass(UNetworkBaseCP::StaticClass());
	if (!cp) return false;
	UNetworkBaseCP* netCP = Cast<UNetworkBaseCP>(cp);
	if (!netCP) return false;

	AActor** mapActor = _networkActors.Find(netCP->GetObjectIDByString());
	if (mapActor != nullptr && *mapActor != nullptr) return false;
	_networkActors.Add(netCP->GetObjectIDByString(), networkActor);
	return true;
}

void AInGameNetworkProcessor::RemoveObject(AActor * networkActor)
{
	UActorComponent* cp = networkActor->GetComponentByClass(UNetworkBaseCP::StaticClass());
	if (!cp) return;
	UNetworkBaseCP* netCP = Cast<UNetworkBaseCP>(cp);
	if (!netCP) return;

	AActor** mapActor = _networkActors.Find(netCP->GetObjectIDByString());
	if (mapActor != nullptr && *mapActor == networkActor) {
		_networkActors.Remove(netCP->GetObjectIDByString());
	}
}

void AInGameNetworkProcessor::GetSyncDataForce(AActor* networkActor)
{
	// Object에 도달하지 않는 메세지를 다시 처리한다.
	
	for (int i = 0; i < _savedList.Num(); ++i) {
		FReplicationData& data = _savedList[i];
		if (*(_networkActors.Find(data.objectHandle)) == networkActor) {
			if (CheckSavedObject(data, 0.0f)) {
				if (data.delayTime >= MAX_SAVED_TIME) {
					LOG(Error, "data removed by timeout... : object : %s, usage : %s", *(data.objectHandle), *(data.usageHandle));
				}
				_savedList.RemoveAt(i);
				--i;
				continue;
			}
		}
	}
}

int32 AInGameNetworkProcessor::GetCurrentSlot()
{
	if(_networkSystem) return _networkSystem->GetCurrentSlot();
	else return 0;
}

FString AInGameNetworkProcessor::GetMyName()
{
	if(_networkSystem) return _networkSystem->GetMySteamName();
	else return FString("NoName");
}

FString AInGameNetworkProcessor::GetSlotName(int32 slot)
{
	if(slot >= 0 && slot < MAX_PLAYER) return _slotAndName[slot].Get<1>();
	else return FString("Not exist slot.");
}

int AInGameNetworkProcessor::GetPlayerNumbs()
{
	int count = 0;
	for(int i=0; i<MAX_PLAYER; ++i){
		if(_slotAndName[i].Get<1>() != "") ++count;
	}
	return count;
}

void AInGameNetworkProcessor::EndGame()
{
	shared_ptr<char[]> buf(new char[sizeof(EMessageType)]);
	SerializeEnum(EMessageType::C_END_GAME, buf.get());
	Send(buf, sizeof(EMessageType), true);
}

void AInGameNetworkProcessor::RefreshSlot()
{
	_OnRefreshSlot(IsMaster());
}

bool AInGameNetworkProcessor::IsMaster()
{
	return GetCurrentSlot() == 0;
}

bool AInGameNetworkProcessor::IsSlave()
{
	return !IsMaster();
}

uint64 AInGameNetworkProcessor::GetHashNumber(const FString& actorName)
{
	uint32_t hash, i, len = actorName.Len();
	for (hash = i = 0; i < len; ++i)
	{
		hash += actorName[i];
		hash += (hash << 10);
		hash ^= (hash >> 6);
	}
	hash += (hash << 3);
	hash ^= (hash >> 11);
	hash += (hash << 15);
	uint64 res = static_cast<uint64>(hash);
	while (res >= SPAWN_COUNT_START) {
		res /= 2;
	}
	return res;
}

void AInGameNetworkProcessor::EnterToGame_DEBUG()
{
	shared_ptr<char[]> buf(new char[sizeof(EMessageType)]);
	SerializeEnum(EMessageType::C_Debug_GameStart, buf.get());
	Send(buf, sizeof(EMessageType), true);
}

void AInGameNetworkProcessor::SetSteamID_DEBUG(const FString & steamID)
{
	_networkSystem->SetSteamID_DUBGE(FStringToUINT64(steamID));
}

AActor * AInGameNetworkProcessor::NetworkSpawn(ENetSpawnType type, FVector position, FQuat rotation)
{
	if (!IsMaster()) {
		LOG_SCREEN("NetworkSpawn only work on master.");
		return nullptr;
	}
	uint64 currentSpawnCount = _spawnCount++;
	AActor* retval = NetworkSpawned(type, position, rotation, FString::Printf(TEXT("NetworkSpawn%llu"), currentSpawnCount));

	// Send to other client
	if (_networkSystem->OnServer() && retval) {
		shared_ptr<char[]> buf(new char[sizeof(EMessageType) + sizeof(uint64) + sizeof(int32) + sizeof(float) * 6]);
		int cursor = 0;

		// Create Buffer
		cursor += SerializeEnum(EMessageType::C_INGAME_SPAWN, buf.get() + cursor);
		cursor += UInt64Serialize(buf.get() + cursor, currentSpawnCount);
		cursor += IntSerialize(buf.get() + cursor, static_cast<int32>(type));
		cursor += Vector3Serialize(buf.get() + cursor, position);
		cursor += Vector3Serialize(buf.get() + cursor, rotation.Euler());

		Send(buf, cursor, true);
	}
	return retval;
}

UClass * AInGameNetworkProcessor::GetClassByType(const ENetSpawnType& type)
{
	if (classes.Find(type) == nullptr) {
		LOG(Error, "Not registered Type.");
		return nullptr;
	}
	else return classes[type];
}

AActor* AInGameNetworkProcessor::NetworkSpawned(ENetSpawnType type, FVector position, FQuat rotation, const FString& objectID)
{
	AActor* retval;
	UClass* spawnClass = GetClassByType(type);
	CHECK(spawnClass, nullptr);
	FTransform newTransform = FTransform::Identity;
	newTransform.SetLocation(position);
	newTransform.SetRotation(rotation);
	retval = GetWorld()->SpawnActor(spawnClass);
	if (!retval) {
		LOG(Error, "Can't spawn actor!!!");
		return nullptr;
	}
	retval->SetActorTransform(newTransform);

	UActorComponent* actCmp = retval->GetComponentByClass(UNetworkBaseCP::StaticClass());
	if (actCmp) {
		UNetworkBaseCP* netBaseCmp = Cast<UNetworkBaseCP>(actCmp);
		netBaseCmp->SetObjectIDByNetworkID(objectID);
	}

	return retval;
}

void AInGameNetworkProcessor::RecvProc(FReciveData& data)
{
	Super::RecvProc(data);

	int cursor = 0;
	while (cursor < data.len) {
		CHECK(cursor >= 0 && cursor <= BUFSIZE);
		int bufLen = IntDeserialize(data.buf, &cursor) - sizeof(EMessageType);
		CHECK(bufLen >= 0 && bufLen <= BUFSIZE);
		EMessageType type = GetEnum(data.buf, &cursor);
		UINT64 sid = _networkSystem->GetSteamID();
		switch (type)
		{
		case EMessageType::S_INGAME_SPAWN:
		{
			InGame_Spawn(data, cursor, bufLen);
			break;
		}
		case EMessageType::S_INGAME_RPC:
		{
			InGame_RPC(data, cursor, bufLen);
			break;
		}
		case EMessageType::S_INGAME_SyncVar:
		{
			InGame_SyncVar(data, cursor, bufLen);
			break;
		}
		case EMessageType::S_DISCONNECT_SLOT:
		{
			InGame_DisconnectSlot(data, cursor, bufLen);
			break;
		}
		case EMessageType::S_RECONNECT_SLOT:
		{
			InGame_ReconnectSlot(data, cursor, bufLen);
			break;
		}
		case EMessageType::S_END_GAME:
		{
			EndGame(data, cursor, bufLen);
			return;
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

void AInGameNetworkProcessor::InGame_Spawn(FReciveData& data, int& cursor, int& bufLen)
{
	uint64 currentSpawnCount = UInt64Deserialize(data.buf, &cursor);
	ENetSpawnType spawnType = static_cast<ENetSpawnType>(IntDeserialize(data.buf, &cursor));
	FVector location = Vector3Deserialize(data.buf, &cursor);
	FVector rotationVector = Vector3Deserialize(data.buf, &cursor);
	FQuat rotation = FQuat::Identity;
	rotation.MakeFromEuler(rotationVector);
	NetworkSpawned(spawnType, location, rotation, FString::Printf(TEXT("NetworkSpawn%llu"), currentSpawnCount));
}

void AInGameNetworkProcessor::InGame_RPC(FReciveData& data, int& cursor, int& bufLen)
{
	// Parse Data
	FString objectID = FStringDeserialize(data.buf, &cursor);
	FString functionHandle = FStringDeserialize(data.buf, &cursor);
	int32 len = IntDeserialize(data.buf, &cursor);
	CHECK(len >= 0 && len < BUFSIZE);
	shared_ptr<char[]> buf(new char[len]);
	memcpy(buf.get(), data.buf + cursor, len);
	cursor += len;

	FReplicationData newData;
	newData.buf = buf;
	newData.delayTime = 0.0f;
	newData.isRPC = true;
	newData.len = len;
	newData.objectHandle = objectID;
	newData.usageHandle = functionHandle;

#ifdef DEBUG_RPC
	LOG_WARNING("RPC : %s %s", *objectID, *functionHandle);
#endif

	if (!SendToObject(newData)) {
		_savedList.Add(newData);
	}
}

void AInGameNetworkProcessor::InGame_SyncVar(FReciveData& data, int& cursor, int& bufLen)
{
	// Parse Data
	FString objectID = FStringDeserialize(data.buf, &cursor);
	FString handle = FStringDeserialize(data.buf, &cursor);
	int32 len = IntDeserialize(data.buf, &cursor);
	CHECK(len >= 0 && len < BUFSIZE);
	shared_ptr<char[]> buf(new char[len]);
	memcpy(buf.get(), data.buf + cursor, len);
	cursor += len;

	FReplicationData newData;
	newData.buf = buf;
	newData.delayTime = 0.0f;
	newData.isRPC = false;
	newData.len = len;
	newData.objectHandle = objectID;
	newData.usageHandle = handle;

#ifdef DEBUG_RPC
	LOG_WARNING("RPC : %s %s", *objectID, *handle);
#endif

	if (!SendToObject(newData)) {
		_savedList.Add(newData);
	}
}

void AInGameNetworkProcessor::InGame_DisconnectSlot(FReciveData& data, int& cursor, int& bufLen)
{
	int32 targetSlot = IntDeserialize(data.buf, &cursor);
	_Disconnect(targetSlot);
}

void AInGameNetworkProcessor::InGame_ReconnectSlot(FReciveData& data, int& cursor, int& bufLen)
{
	int32 targetSlot = IntDeserialize(data.buf, &cursor);
	_Reconnected(targetSlot);
}

void AInGameNetworkProcessor::EndGame(FReciveData& data, int& cursor, int& bufLen)
{
	UGameplayStatics::OpenLevel(GetWorld(), "Lobby");
}

void AInGameNetworkProcessor::Reconnect(FReciveData& data, int& cursor, int& bufLen)
{
	_ReconnectProcess();
	cursor += bufLen;
}