// Fill out your copyright notice in the Description page of Project Settings.

#include "MyPlayer.h"
#include "DrawDebugHelpers.h"
#include "InGame/Boid/Boid.h"
#include "GameFramework/FloatingPawnMovement.h"
#include "GameFramework/SpringArmComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/SphereComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Camera/CameraComponent.h"
#include "Components/ArrowComponent.h"

float AMyPlayer::GetSprintRate()
{
	return _currentSprintGuage / SprintLimit;
}

void AMyPlayer::OnBoidNumChange()
{

}

void AMyPlayer::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);
	if (NewController == UGameplayStatics::GetPlayerController(GetWorld(), 0)) {
		_bOnMove = true;
	}
}

// Sets default values
AMyPlayer::AMyPlayer()
{
 	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	CapsuleCP = CreateDefaultSubobject<UCapsuleComponent>(TEXT("CapsuleCP"));
	SphereCP = CreateDefaultSubobject<USphereComponent>(TEXT("SphereCP"));
	SkeletalMeshCP = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("SkeletalMeshCP"));
	ArrowCP = CreateDefaultSubobject<UArrowComponent>(TEXT("ArrowCP"));
	SpringArmCP = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArmCP"));
	CameraCP = CreateDefaultSubobject<UCameraComponent>(TEXT("CameraCP"));
	FloatingPawnMovementCP = CreateDefaultSubobject<UFloatingPawnMovement>(TEXT("FloatingPawnMovement"));
	NetBaseCP = CreateDefaultSubobject<UNetworkBaseCP>(TEXT("NetBaseCP"));

	RootComponent = CapsuleCP;
	SkeletalMeshCP->AttachToComponent(CapsuleCP, FAttachmentTransformRules::KeepRelativeTransform);
	ArrowCP->AttachToComponent(CapsuleCP, FAttachmentTransformRules::KeepRelativeTransform);
	SpringArmCP->AttachToComponent(CapsuleCP, FAttachmentTransformRules::KeepRelativeTransform);
	CameraCP->AttachToComponent(SpringArmCP, FAttachmentTransformRules::KeepRelativeTransform);
	SphereCP->AttachToComponent(CapsuleCP, FAttachmentTransformRules::KeepRelativeTransform);

	SphereCP->SetSphereRadius(250.0f);
	CapsuleCP->InitCapsuleSize(42.0f, 96.0f);

	TurnSpeed = 20.f;
	LookUpSpeed = 20.f;
	MoveSpeed = 1000.f;
	ShootRange = 3000.0f;
	SprintLimit = 3.0f;

	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = true;
	bUseControllerRotationRoll = false;
	
	SphereCP->OnComponentBeginOverlap.AddDynamic(this, &AMyPlayer::OnOverlap);
}

// Called when the game starts or when spawned
void AMyPlayer::BeginPlay()
{
	Super::BeginPlay();
	_currentSprintGuage = SprintLimit;
	_bCanSprint = true;

	BindRPCFunction(NetBaseCP, AMyPlayer, RPCShootBoid);
	BindRPCFunction(NetBaseCP, AMyPlayer, RPCShootVector);
}

void AMyPlayer::OnMoveUp(float Value)
{
	_upInputRate = Value;
}

void AMyPlayer::OnMoveForward(float Value)
{
	_forwardInputRate = Value;
}

void AMyPlayer::OnMoveRight(float Value)
{
	_rightInputRate = Value;
}

void AMyPlayer::OnTurn(float Rate)
{
	AddControllerYawInput(Rate * TurnSpeed * GetWorld()->GetDeltaSeconds());
}

void AMyPlayer::OnLookUp(float Rate)
{
	AddControllerPitchInput(Rate * LookUpSpeed * GetWorld()->GetDeltaSeconds());
}

void AMyPlayer::OnSprint()
{
	_bOnSprint = true;
}

void AMyPlayer::OffSprint()
{
	_bOnSprint = false;
}

void AMyPlayer::OnShoot()
{
	FHitResult Hit;
	FVector StartPos = CameraCP->GetComponentLocation();
	FVector EndPos = StartPos + CameraCP->GetForwardVector() * ShootRange;
	FCollisionQueryParams TraceParams(NAME_None, false, this);
	GetWorld()->LineTraceSingleByChannel(Hit, StartPos, EndPos, ECC_GameTraceChannel1, TraceParams);
	DrawDebugLine(GetWorld(), StartPos, EndPos, Hit.bBlockingHit ? FColor::Green : FColor::Red,
		false, 1.0f);

	if (Hit.GetActor() && Cast<ABoid>(Hit.GetActor())) {
		ABoid* boid = Cast<ABoid>(Hit.GetActor());
		RPCShootBoid(boid->GetName());
	}
	else {
		RPCShootVector(EndPos);
	}
}

void AMyPlayer::DetachBoid(ABoid* boid)
{
	_boids.Remove(boid);
}

void AMyPlayer::StopMovement()
{
	_bOnMove = false;
}

// Called every frame
void AMyPlayer::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (!_bOnMove || !(NetBaseCP->HasAuthority())) return;

	FVector moveVector = (CameraCP->GetForwardVector() * _forwardInputRate +
		CameraCP->GetRightVector() * _rightInputRate +
		CameraCP->GetUpVector() * _upInputRate).GetSafeNormal(SMALL_NUMBER);

	if (moveVector.Size() == 0.0f) moveVector = CameraCP->GetForwardVector();

	// 달리기 처리
	if (_bCanSprint && _bOnSprint) {
		_currentSprintGuage -= DeltaTime;
		moveVector = moveVector * 2.0f;

		// 소진시 잠시동안 달리기 금지
		if (_bCanSprint && _currentSprintGuage <= 0.0f) {
			_bCanSprint = false;
		}
	}
	else {
		_currentSprintGuage += DeltaTime;

		// 소진 상태일경우 모두 회복되면 달리기 가능
		if (!_bCanSprint && _currentSprintGuage >= SprintLimit) {
			_bCanSprint = true;
		}
	}
	_currentSprintGuage = FMath::Clamp(_currentSprintGuage, 0.0f, SprintLimit);

	// 최종 속도 넣기!
	FloatingPawnMovementCP->Velocity = moveVector * MoveSpeed;
}

// Called to bind functionality to input
void AMyPlayer::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	PlayerInputComponent->BindAxis("MoveUp", this, &AMyPlayer::OnMoveUp);
	PlayerInputComponent->BindAxis("MoveForward", this, &AMyPlayer::OnMoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &AMyPlayer::OnMoveRight);
	PlayerInputComponent->BindAxis("Turn", this, &AMyPlayer::OnTurn);
	PlayerInputComponent->BindAxis("LookUp", this, &AMyPlayer::OnLookUp);
	PlayerInputComponent->BindAction("Sprint", IE_Pressed, this, &AMyPlayer::OnSprint);
	PlayerInputComponent->BindAction("Sprint", IE_Released, this, &AMyPlayer::OffSprint);
	PlayerInputComponent->BindAction("Shoot", IE_Pressed, this, &AMyPlayer::OnShoot);
}

void AMyPlayer::RPCShootBoid(FString boidName)
{
	RPC(NetBaseCP, AMyPlayer, RPCShootBoid, ENetRPCType::MULTICAST, true, boidName);
	TArray<AActor*> outActors;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), ABoid::StaticClass(), outActors);
	for (auto i : outActors) {
		if (i->GetName() == boidName) {
			ABoid* boid = Cast<ABoid>(i);
			if (!boid) break;
			ShootParticleToActor(boid);
			boid->DestroyBoid();
			break;
		}
	}
}

void AMyPlayer::RPCShootVector(FVector pos)
{
	RPC(NetBaseCP, AMyPlayer, RPCShootVector, ENetRPCType::MULTICAST, true, pos);
	ShootParticleToVector(pos);
}

void AMyPlayer::AttachBoid(ABoid* boid)
{
	_boids.Add(boid);
	_currentSprintGuage += SprintLimit * 0.5f;
	_currentSprintGuage = FMath::Clamp(_currentSprintGuage, 0.0f, SprintLimit);
	OnBoidNumChange();
}

void AMyPlayer::OnOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	ABoid* boid = Cast<ABoid>(OtherActor);
	if(boid) boid->MASTERSetLeader(NetBaseCP->GetCurrentAuthority());
}