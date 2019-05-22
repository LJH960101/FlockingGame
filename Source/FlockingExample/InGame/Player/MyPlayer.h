// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "MyPlayer.generated.h"

class ABoid;

UCLASS()
class FLOCKINGEXAMPLE_API AMyPlayer : public APawn
{
	GENERATED_BODY()

public:
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

	// 카메라 좌우 Turn Speed
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Camera)
	float TurnSpeed;

	// 카메라 상하 LookUp Speed
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Camera)
	float LookUpSpeed;

	// 이동 속도
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Camera)
	float MoveSpeed;

	// 공격 거리
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Camera)
	float ShootRange;

	// 달리기 가능 시간
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Camera)
	float SprintLimit;

	UFUNCTION(BlueprintPure, Category = Sprint)
	float GetSprintRate();

	UFUNCTION(BlueprintPure, Category = Sprint)
	bool GetSprintEnable() { return _bCanSprint; }

	UFUNCTION(BlueprintPure, Category = Boid)
	int GetBoidNumbs() { return _boids.Num(); }

	// 보이드의 수가 변했을때의 처리입니다.
	void OnBoidNumChange();

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
	// 파티클을 재생합니다.
	UFUNCTION(BlueprintImplementableEvent, Category = "Shoot")
	void ShootParticle(FVector pos);

	UFUNCTION(BlueprintCallable, Category = "Boid")
	void DetachBoid(ABoid* boid);

	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

private:
	UFUNCTION()
	void OnOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	// 입력된 이동 비율
	float _upInputRate, _forwardInputRate, _rightInputRate;

	// 남은 스프린트 게이지
	float _currentSprintGuage;

	// 스프린트 중인가?
	bool _bOnSprint;

	bool _bCanSprint;

	// 따라오는 보이드들
	TArray<ABoid*> _boids;
};
