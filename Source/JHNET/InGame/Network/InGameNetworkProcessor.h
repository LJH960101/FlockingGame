// InGame - Network System Class

#pragma once

#include "NetworkModule/GameInfo.h"
#include "JHNET.h"
#include "Common/Network/CommonNetworkProcessor.h"
#include <memory>
#include "Component/NetworkBaseCP.h"
#include "InGameNetworkProcessor.generated.h"

struct FReplicationData
{
	float delayTime;
	bool isRPC;
	FString objectHandle;
	FString usageHandle;
	int32 len;
	std::shared_ptr<char[]> buf;
};

// Spawnable actor's enum.
// You must add enum to add new class spawner.
// also need to add in GetClassByType.
UENUM(BlueprintType)
enum class ENetSpawnType : uint8
{
	MAX			UMETA(DisplayName = "MAX")
};

UCLASS()
class JHNET_API AInGameNetworkProcessor : public ACommonNetworkProcessor
{
	GENERATED_BODY()
	
	// Spawnable Actor classes.
private:
	UPROPERTY()
	TMap<ENetSpawnType, TSubclassOf<class AActor>>  classes;

	// Saved when not exist object.
	TArray<FReplicationData> _savedList;
	const float MAX_SAVED_TIME = 10.0f;

private:
	// Other Network Classes
	class AInGameManager* InGameManager;
	class UNetworkSystem* _networkSystem;
	class UJHNETGameInstance* _gameInstance;
	class UNetworkBaseCP* NetBaseCP;

	// Network Actors
	TMap<FString, AActor*> _networkActors;

	// FString name, int slot
	RPC_FUNCTION(AInGameNetworkProcessor, _RPCSetSlotSteamName, FString, int)
	void _RPCSetSlotSteamName(FString name, int slot);

	void _SetSlotSteamName();
	TArray< TTuple<bool,FString> > _slotAndName;

	// 슬롯이 다시 설정되었을때 통보됨.
	void _OnRefreshSlot(bool isMaster);

	// 서버에 접속되었을때 호출됩니다.
	virtual void OnConnected() override;

	// NetworkSpawn *************
	const uint64 SPAWN_COUNT_START = UINT64_MAX / 2 + 1;
	uint64 _spawnCount = SPAWN_COUNT_START;
	// Get UClass By Type. You must add case to add new class spawner.
	UClass* GetClassByType(const ENetSpawnType& type);
	// Call by receive msg or owner's spawn action.
	AActor* NetworkSpawned(ENetSpawnType type, FVector position, FQuat rotation, const FString& objectID);

	// Network RPC / SyncVar
	// Check delayed data.
	// return : Is Success? (Success or Destroyed)
	bool CheckSavedObject(FReplicationData& data, float deltaTime);
	bool SendToObject(FReplicationData& data);
	
	// Disconnect/Reconnect Proc ******
	void _Disconnect(int32 slot);
	void _Reconnected(int32 slot);

	// Call all Reconnect() of netbase actor
	void _ReconnectProcess();

	// 싱글 플레이일경우 강제로 스킵하는 함수.
	void _ReconnectCheck();
	FTimerHandle _reconnectCheckTimerHandle;

	UPROPERTY()
	TSubclassOf<class UUserWidget> _disconnectWGClass;
	UPROPERTY()
	class UUserWidget* _disconnectWG = nullptr;

public:
	// Sets default values for this actor's properties
	AInGameNetworkProcessor();
	~AInGameNetworkProcessor();
	// Called every frame
	virtual void Tick(float DeltaTime) override;
	UNetworkSystem* GetNetworkSystem();
	
	// Identification
	void RefreshSlot();
	UFUNCTION(BlueprintPure)
	bool IsMaster();
	UFUNCTION(BlueprintPure)
	bool IsSlave();
	uint64 GetHashNumber(const FString& actorName);
	UFUNCTION(BlueprintPure)
	int32 GetCurrentSlot();
	UFUNCTION(BlueprintCallable, Category = "Network")
	FString GetMyName();
	UFUNCTION(BlueprintCallable, Category = "Network")
	FString GetSlotName(int32 slot);
	int GetPlayerNumbs();

	UFUNCTION(BlueprintCallable, Category = "Network")
	void EndGame();

	// Network Actors
	bool AddObject(AActor* networkActor);
	void RemoveObject(AActor* networkActor);

	// 누락된 데이터를 즉시 받습니다.
	void GetSyncDataForce(AActor* networkActor);

	// NetworkSpawn
	// Spawn new actor. Only work on server.
	UFUNCTION(BlueprintCallable)
	AActor* NetworkSpawn(ENetSpawnType type, FVector position, FQuat rotation);

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	virtual void PostInitializeComponents() override;
	virtual void RecvProc(FReciveData& data) override;
	// Called when room is changed.
	virtual void OnRefreshRoom() override;

private:
	// Request force join to game room.
	void EnterToGame_DEBUG();
	// Force create steamID.
	void SetSteamID_DEBUG(const FString& steamID);

private:
	// 메세지 처리 함수

	// InGame
	void InGame_Spawn(FReciveData& data, int& cursor, int& bufLen);
	void InGame_RPC(FReciveData& data, int& cursor, int& bufLen);
	void InGame_SyncVar(FReciveData& data, int& cursor, int& bufLen);
	void InGame_DisconnectSlot(FReciveData& data, int& cursor, int& bufLen);
	void InGame_ReconnectSlot(FReciveData& data, int& cursor, int& bufLen);
	void EndGame(FReciveData& data, int& cursor, int& bufLen);
	void Reconnect(FReciveData& data, int& cursor, int& bufLen);
};
