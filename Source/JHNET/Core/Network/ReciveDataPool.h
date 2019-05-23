// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "JHNET.h"
#include "UObject/NoExportTypes.h"
#include "Containers/Queue.h"
#include "NetworkModule/GameInfo.h"
#include <mutex>
#include "ReciveDataPool.generated.h"

/**
 * 수신 데이터를 오브젝트 풀로 관리하는 클래스 입니다.
 */

struct FReciveData
{
	char buf[BUFSIZE];
	int len;
};

UCLASS()
class JHNET_API UReciveDataPool : public UObject
{
	GENERATED_BODY()
public:
	UReciveDataPool();
	void ReturnObject(FReciveData* object);
	FReciveData* GetObject();
private:
	// Enqueue newObject by _size
	void Resize();
	TQueue<FReciveData*> _objects;
	int _size;
	std::recursive_mutex _mt;
};
