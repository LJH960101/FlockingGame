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
	// �������� �������� ȣ���.(�����̺�)
	virtual void Reconnect() {}
	// �����Ӵ��� �������� ȣ���.(������)
	virtual void Reconnected() {}
	// ���������� �Լ�
	virtual void Disconnected() {}
	// ������ �ٲ������ �Լ�
	virtual void OnSlotChange(bool isMaster) {}
};
