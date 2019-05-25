// Fill out your copyright notice in the Description page of Project Settings.


#include "GameRuleProcessor.h"

// Sets default values
AGameRuleProcessor::AGameRuleProcessor()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	NetBaseCP = CreateDefaultSubobject<UNetworkBaseCP>(TEXT("NetBaseCP"));
}

void AGameRuleProcessor::StartGame()
{
	if (NetBaseCP->IsMaster()) {
		RPC(NetBaseCP, AGameRuleProcessor, StartGame, ENetRPCType::MULTICAST, true);
	}
	_bOnGame = true;
	OnStartGame();
}

void AGameRuleProcessor::EndGame()
{
	if (NetBaseCP->IsMaster()) {
		RPC(NetBaseCP, AGameRuleProcessor, EndGame, ENetRPCType::MULTICAST, true);
	}
	_timeRemain = 0.0f;
	_bOnGame = false;
	OnEndGame();
}

// Called when the game starts or when spawned
void AGameRuleProcessor::BeginPlay()
{
	Super::BeginPlay();

	_bOnGame = false;

	BindRPCFunction(NetBaseCP, AGameRuleProcessor, StartGame);
	BindRPCFunction(NetBaseCP, AGameRuleProcessor, EndGame);
}

// Called every frame
void AGameRuleProcessor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (_bOnGame) {
		_timeRemain -= DeltaTime;
		if (_timeRemain <= 0.0f) {
			EndGame();
		}
	}
}

