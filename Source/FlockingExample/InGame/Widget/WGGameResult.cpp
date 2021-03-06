// Fill out your copyright notice in the Description page of Project Settings.


#include "WGGameResult.h"
#include "InGame/Player/MyPlayer.h"
#include "InGame/Network/InGameNetworkProcessor.h"
#include "NetworkModule/GameInfo.h"

FString UWGGameResult::GetGameResult()
{
	TArray<AActor*> outActors;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), AMyPlayer::StaticClass(), outActors);
	
	int scores[MAX_PLAYER];
	AMyPlayer* players[MAX_PLAYER];	

	int count = 0;
	for (auto actor : outActors) {
		AMyPlayer* player = Cast<AMyPlayer>(actor);
		if (!player) continue;
		int score = player->GetBoidNumbs();
		scores[count] = score;
		players[count++] = player;
	}
	if (count != MAX_PLAYER) {
		return FString(TEXT("유저수 부족"));
	}

	// 정렬한다.
	for (int i = 0; i < MAX_PLAYER; ++i) {
		for (int j = i+1; j < MAX_PLAYER; ++j) {
			if (scores[i] < scores[j]) {
				{
					int temp = scores[i];
					scores[i] = scores[j];
					scores[j] = temp;
				}
				{
					AMyPlayer* temp = players[i];
					players[i] = players[j];
					players[j] = temp;
				}
			}
		}

	}

	FString retval;
	for (int i = 0; i < MAX_PLAYER; ++i) {
		retval.Append(FString::Printf(TEXT("%d등 : %s %d점\n"),
			i + 1,
			*(players[i]->NetBaseCP->GetInGameNetworkProcessor()->GetSlotName(
				players[i]->NetBaseCP->GetCurrentAuthority()
			)),
			scores[i]));
	}
	return retval;
}
