// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "InGame/Network/Component/NetworkBaseCP.h"
#include "MyPlayer.generated.h"

class ABoid;

UCLASS()
class FLOCKINGEXAMPLE_API AMyPlayer : public APawn
{
	GENERATED_BODY()

public:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Network")
	class UNetworkBaseCP* NetBaseCP;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Flocking")
	class UFloatingPawnMovement* FloatingPawnMovementCP;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Flocking")
	class USpringArmComponent* SpringArmCP;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Flocking")
	class UCapsuleComponent* CapsuleCP;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Flocking")
	class USphereComponent* SphereCP;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Flocking")
	class USkeletalMeshComponent* SkeletalMeshCP;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Flocking")
	class UCameraComponent* CameraCP;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Flocking")
	class UArrowComponent* ArrowCP;

	// ī�޶� �¿� Turn Speed
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Camera)
	float TurnSpeed;

	// ī�޶� ���� LookUp Speed
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Camera)
	float LookUpSpeed;

	// �̵� �ӵ�
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Camera)
	float MoveSpeed;

	// ���� �Ÿ�
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Camera)
	float ShootRange;

	// �޸��� ���� �ð�
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Camera)
	float SprintLimit;

	UFUNCTION(BlueprintPure, Category = Sprint)
	float GetSprintRate();

	UFUNCTION(BlueprintPure, Category = Sprint)
	bool GetSprintEnable() { return _bCanSprint; }

	UFUNCTION(BlueprintPure, Category = Boid)
	int GetBoidNumbs() { return _boids.Num(); }

	// ���̵��� ���� ���������� ó���Դϴ�.
	void OnBoidNumChange();
	
	virtual void PossessedBy(AController* NewController);

	// Sets default values for this pawn's properties
	AMyPlayer();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	void OnMoveUp(float Value);
	void OnMoveForward(float Value);
	void OnMoveRight(float Value);
	void OnTurn(float Rate);
	void OnLookUp(float Rate);
	void OnSprint();
	void OffSprint();
	void OnShoot();

public:	
	// ��ƼŬ�� ����մϴ�.
	UFUNCTION(BlueprintImplementableEvent, Category = "Shoot")
	void ShootParticleToVector(FVector pos);

	// ��ƼŬ�� ����մϴ�.
	UFUNCTION(BlueprintImplementableEvent, Category = "Shoot")
	void ShootParticleToActor(AActor* actor);

	UFUNCTION(BlueprintCallable, Category = "Boid")
	void DetachBoid(ABoid* boid);

	UFUNCTION(BlueprintCallable)
	void StopMovement();

	void AttachBoid(ABoid* boid);

	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

private:
	RPC_FUNCTION(AMyPlayer, RPCShootBoid, FString)
	void RPCShootBoid(FString boidName);

	RPC_FUNCTION(AMyPlayer, RPCShootVector, FVector)
	void RPCShootVector(FVector pos);

	UFUNCTION()
	void OnOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	// �Էµ� �̵� ����
	float _upInputRate, _forwardInputRate, _rightInputRate;

	// ���� ������Ʈ ������
	float _currentSprintGuage;

	// ������Ʈ ���ΰ�?
	bool _bOnSprint;

	bool _bCanSprint;

	bool _bOnMove = false;

	// ������� ���̵��
	TArray<ABoid*> _boids;
};
