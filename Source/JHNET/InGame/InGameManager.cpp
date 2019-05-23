// Fill out your copyright notice in the Description page of Project Settings.

#include "InGameManager.h"
#include "InGame/JHNETTool.h"

using namespace ActorFinderTool;

// Sets default values
AInGameManager::AInGameManager()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;
}

void AInGameManager::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	// Get Objects
	if(!_networkProcessor) GetNetworkProcessor();
}

AInGameNetworkProcessor* AInGameManager::GetNetworkProcessor()
{
	if (!_networkProcessor) _networkProcessor = CreateOrGetObject<AInGameNetworkProcessor>(GetWorld());
	return _networkProcessor;
}