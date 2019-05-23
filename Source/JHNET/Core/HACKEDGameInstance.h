// Copyrightⓒ2019 by 블랙말랑카우. All Data cannot be copied without permission. 

#pragma once

#include "JHNET.h"
#include "Engine/GameInstance.h"
#include "HACKEDGameInstance.generated.h"

class UNetworkSystem;
class AInGameManager;

UCLASS()
class JHNET_API UHACKEDGameInstance : public UGameInstance
{
	GENERATED_BODY()
public:
	UHACKEDGameInstance();
	~UHACKEDGameInstance();
	UFUNCTION(BlueprintCallable)
	UNetworkSystem* GetNetworkSystem();
	AInGameManager* GetInGameManager();

private:
	virtual void Init();
	virtual void Shutdown();
	UPROPERTY()
	UNetworkSystem* _networkSystem;
	AInGameManager* InGameManager;
};
