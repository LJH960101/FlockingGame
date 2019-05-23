// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "JHNET.h"
#include "UObject/NoExportTypes.h"
#include "RoomPlayer.generated.h"

/**
 * Class that contains information of players of current room.
 */
UCLASS()
class JHNET_API URoomPlayer : public UObject {
	GENERATED_BODY()
public:
	URoomPlayer() {}
	~URoomPlayer() {}
	UPROPERTY()
	FString name;
	UPROPERTY()
	UTexture2D* image;
	UPROPERTY()
	FString steamID;

	UPROPERTY()
	bool onReady;
};