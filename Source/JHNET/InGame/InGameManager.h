#pragma once

#include "JHNET.h"
#include "GameFramework/Actor.h"
#include "Core/Network/NetworkSystem.h"
#include "Core/JHNETGameInstance.h"
#include "InGame/Network/InGameNetworkProcessor.h"
#include "InGameManager.generated.h"

UCLASS()
class JHNET_API AInGameManager : public AActor
{
	GENERATED_BODY()

public:
	AInGameManager();

	UFUNCTION(BlueprintPure, Category = "Manager")
	AInGameNetworkProcessor* GetNetworkProcessor();

	virtual void PostInitializeComponents() override;

private:
	AInGameNetworkProcessor* _networkProcessor;
};
