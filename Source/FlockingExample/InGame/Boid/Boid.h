// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "EngineMinimal.h"
#include "GameFramework/Pawn.h"
#include "Boid.generated.h"

UCLASS()
class FLOCKINGEXAMPLE_API ABoid : public APawn
{
	GENERATED_BODY()
	
public:	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Flocking")
	class UFloatingPawnMovement* FloatingPawnMovement;

	// Sets default values for this actor's properties
	ABoid();

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Flocking")
	class AMyPlayer* leader;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Flocking")
	FVector direction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Flocking")
	float speed = 1000.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Flocking")
	float currentSpeed;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Flocking")
	float checkRadius = 500.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Flocking")
	float closeDistance = 500.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Flocking")
	float leaderDistance = 1000.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Flocking")
	bool isSeperation = false;

	UFUNCTION(BlueprintCallable, Category = "Flocking")
	void SetLeader(AMyPlayer* newLeader);

	// 리더가 바뀌었다는것을 블루프린트에 통지.
	// 리더와 같은 마테리얼로 바꿉니다.
	UFUNCTION(BlueprintImplementableEvent, Category = "Flocking")
	void OnLeaderChange();

	void DestroyBoid();

	TArray<FHitResult> neighbors;
	TArray<FHitResult> obstacles;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;
	virtual void DrawDebug(float DeltaTime);

private:
	void CheckObjects();
	FVector Seperation();
	FVector Alignment();
	FVector Cohesion();
	FVector FollowLeader();
	FVector Avoid();
	void Flocking();
};
