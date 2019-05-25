// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "InGame/Network/Component/NetworkBaseCP.h"
#include "GameRuleProcessor.generated.h"

UCLASS()
class FLOCKINGEXAMPLE_API AGameRuleProcessor : public AActor
{
	GENERATED_BODY()
	
public:
	UFUNCTION(BlueprintPure, Category = "GameRule")
	bool OnGame() { return _bOnGame; }

	UFUNCTION(BlueprintPure, Category = "GameRule")
	float GetRemainTime() { return _timeRemain; }

	RPC_FUNCTION(AGameRuleProcessor, StartGame);
	UFUNCTION(BlueprintCallable, Category = "GameRule")
	void StartGame();

	RPC_FUNCTION(AGameRuleProcessor, EndGame);
	UFUNCTION(BlueprintCallable, Category = "GameRule")
	void EndGame();

	UFUNCTION(BlueprintImplementableEvent, Category = "GameRule")
	void OnStartGame();

	UFUNCTION(BlueprintImplementableEvent, Category = "GameRule")
	void OnEndGame();

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	float _timeRemain = 60.0f;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Network")
	UNetworkBaseCP* NetBaseCP;

	// Sets default values for this actor's properties
	AGameRuleProcessor();

private:
	bool _bOnGame = false;
};
