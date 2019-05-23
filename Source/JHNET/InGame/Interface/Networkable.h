// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "Networkable.generated.h"

// This class does not need to be modified.
UINTERFACE(MinimalAPI)
class UNetworkable : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class JHNET_API INetworkable
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:
	// 재접속한 유저에서 호출됨.(슬레이브)
	virtual void Reconnect() {}
	// 재접속당한 유저에서 호출됨.(마스터)
	virtual void Reconnected() {}
	// 단절됬을때 함수
	virtual void Disconnected() {}
	// 슬롯이 바뀌었을때 함수
	virtual void OnSlotChange(bool isMaster) {}
};
