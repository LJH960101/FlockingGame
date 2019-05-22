// Fill out your copyright notice in the Description page of Project Settings.


#include "Boid.h"
#include "DrawDebugHelpers.h"
#include "InGame/Player/MyPlayer.h"
#include "GameFramework/FloatingPawnMovement.h"

// Sets default values
ABoid::ABoid()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	FloatingPawnMovement = CreateDefaultSubobject<UFloatingPawnMovement>(TEXT("Floating Pawn Movement Component"));
}

void ABoid::SetLeader(AMyPlayer* newLeader)
{
	if (leader) {
		leader->DetachBoid(this);
		leader->OnBoidNumChange();
	}
	leader = newLeader;
	leader->OnBoidNumChange();
	OnLeaderChange();
}

void ABoid::DestroyBoid()
{
	if (leader) {
		leader->OnBoidNumChange();
		leader->DetachBoid(this);
	}
	Destroy();
}

// Called when the game starts or when spawned
void ABoid::BeginPlay()
{
	Super::BeginPlay();
}

// Called every frame
void ABoid::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (!leader) {
		FloatingPawnMovement->Velocity = FVector::ZeroVector;
	}
	else {
		Flocking();
		FloatingPawnMovement->Velocity = direction * currentSpeed;
	}
}

void ABoid::DrawDebug(float DeltaTime)
{
	DrawDebugLine(GetWorld(), GetActorLocation(), 
		GetActorLocation() + direction * currentSpeed * DeltaTime, FColor::Red, false, DeltaTime);
}

void ABoid::CheckObjects()
{
	neighbors.Empty();

	FCollisionShape MySphere = FCollisionShape::MakeSphere(checkRadius);
	TArray<FHitResult> OutResults;
	GetWorld()->SweepMultiByChannel(OutResults, GetActorLocation(), GetActorLocation() + FVector::UpVector,
		FQuat::Identity, ECC_GameTraceChannel1, MySphere);

	for (FHitResult n : OutResults) {
		if (!(n.GetActor()) || n.GetActor() == this) continue;
		neighbors.Add(n);
	}

	GetWorld()->SweepMultiByChannel(OutResults, GetActorLocation(), GetActorLocation() + FVector::UpVector,
		FQuat::Identity, ECC_GameTraceChannel3, MySphere);

	for (FHitResult o : OutResults) {
		if (!(o.GetActor()) || o.GetActor() == this) continue;
		obstacles.Add(o);
	}
}

FVector ABoid::Seperation()
{
	FVector sepDirResult = FVector::ZeroVector;
	for (FHitResult n : neighbors) {
		AActor* neighborActor = n.GetActor();

		if (FVector::Dist(GetActorLocation(), 
			neighborActor->GetActorLocation()) < closeDistance)
		{
			sepDirResult += (GetActorLocation() - 
				neighborActor->GetActorLocation());
			isSeperation = true;
		}
	}

	return sepDirResult.GetSafeNormal(SMALL_NUMBER);
}

FVector ABoid::Alignment()
{
	FVector alignDirResult = FVector::ZeroVector;
	for (FHitResult n : neighbors) {
		AActor* neighborActor = n.GetActor();

		alignDirResult += neighborActor->GetActorForwardVector();
	}

	FVector leaderDir = FollowLeader();
	alignDirResult += (5.0f * leaderDir.GetSafeNormal(SMALL_NUMBER));
	return alignDirResult.GetSafeNormal(SMALL_NUMBER);
}

FVector ABoid::Cohesion()
{
	FVector targetPos = FVector::ZeroVector;
	for (FHitResult n : neighbors) {
		AActor* neighborActor = n.GetActor();

		targetPos += neighborActor->GetActorLocation();
		targetPos /= neighbors.Num();
	}

	return (targetPos - GetActorLocation()).GetSafeNormal(SMALL_NUMBER);
}

FVector ABoid::FollowLeader()
{
	return (leader->GetActorLocation() - GetActorLocation()).GetSafeNormal(SMALL_NUMBER);
}

FVector ABoid::Avoid()
{
	FVector avoidDir = FVector::ZeroVector;
	for (FHitResult o : obstacles) {
		AActor* obstacleActor = o.GetActor();

		avoidDir += (GetActorLocation() - obstacleActor->GetActorLocation())
			.GetSafeNormal(SMALL_NUMBER);
	}

	return avoidDir;
}

void ABoid::Flocking()
{
	currentSpeed = speed * 0.5f;
	CheckObjects();
	if (neighbors.Num() == 0)
	{
		direction = FollowLeader();
	}
	else
	{
		isSeperation = false;
		FVector sepDir = Seperation();
		if (isSeperation)
		{
			if (FVector::Dist(leader->GetActorLocation(), GetActorLocation()) > closeDistance)
			{
				direction = (sepDir + FollowLeader() * 5).GetSafeNormal(SMALL_NUMBER);
				currentSpeed = speed;
			}
			else {
				direction = sepDir.GetSafeNormal(SMALL_NUMBER);
				currentSpeed = speed;
			}
		}
		else
		{
			if (FVector::Dist(leader->GetActorLocation(), GetActorLocation()) > leaderDistance)
			{
				direction = FollowLeader();
				currentSpeed = speed;
			}
			else
			{
				FVector cohDir = Cohesion();
				FVector alignDir = Alignment();
				direction = (1.0f * cohDir + 3.0f * alignDir).GetSafeNormal(SMALL_NUMBER);
			}
		}
	}
}

