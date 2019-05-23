// Copyrightⓒ2019 by 블랙말랑카우. All Data cannot be copied without permission. 

#include "Core/HACKEDGameInstance.h"
#include "InGame/InGameManager.h"
#include "Network/NetworkSystem.h"
#include "InGame/HACKEDTool.h"

using namespace ActorFinderTool;

UHACKEDGameInstance::UHACKEDGameInstance() {
	_networkSystem = CreateDefaultSubobject<UNetworkSystem>(TEXT("NetworkSystem"));
}
UHACKEDGameInstance::~UHACKEDGameInstance() {}

UNetworkSystem* UHACKEDGameInstance::GetNetworkSystem(){
	return _networkSystem;
}

AInGameManager* UHACKEDGameInstance::GetInGameManager()
{
	if (!InGameManager) {
		InGameManager = CreateOrGetObject<AInGameManager>(GetWorld());
		if (_networkSystem) _networkSystem->SetInGameManager(InGameManager);
	}
	return InGameManager;
}

void UHACKEDGameInstance::Init() {
	Super::Init();
	if (_networkSystem) _networkSystem->Init();
	else LOG(Error, "Can't create NetworkSystem");
}

void UHACKEDGameInstance::Shutdown()
{
	Super::Shutdown();
	if (_networkSystem) _networkSystem->Shutdown();
}