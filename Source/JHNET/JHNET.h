// Copyright 1998-2018 Epic Games, Inc. All Rights Reserved.

#pragma once
#include "Engine.h"
JHNET_API DECLARE_LOG_CATEGORY_EXTERN(LOG_JHNET, Log, All);

#define JHNET_LOG_CALLINFO (FString(__FUNCTION__) + TEXT("(") + FString::FromInt(__LINE__) + TEXT(")"))
#define JHNET_LOG_S(Verbosity) UE_LOG(LOG_JHNET, Verbosity, TEXT("%s"), *JHNET_LOG_CALLINFO)
#define JHNET_LOG(Verbosity, Format, ...) UE_LOG(LOG_JHNET, Verbosity, TEXT("%s %s"), *JHNET_LOG_CALLINFO, *FString::Printf(TEXT(Format), ##__VA_ARGS__))
#define JHNET_LOG_STRING(Verbosity, STRING) UE_LOG(LOG_JHNET, Verbosity, TEXT("%s %s"), *JHNET_LOG_CALLINFO, *STRING)
#define	JHNET_LOG_SCREEN(Format, ...) GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, FString::Printf(TEXT(Format), ##__VA_ARGS__))
#define	JHNET_LOG_SCREEN_T(Time, Format, ...) GEngine->AddOnScreenDebugMessage(-1, Time, FColor::Red, FString::Printf(TEXT(Format), ##__VA_ARGS__))
#define JHNET_LOG_WARNING(Format, ...) UE_LOG(LOG_JHNET, Warning, TEXT("%s %s"), *JHNET_LOG_CALLINFO, *FString::Printf(TEXT(Format), ##__VA_ARGS__))
#define JHNET_LOG_ERROR(Format, ...) UE_LOG(LOG_JHNET, Error, TEXT("%s %s"), *JHNET_LOG_CALLINFO, *FString::Printf(TEXT(Format), ##__VA_ARGS__))
#define JHNET_CHECK(Expr, ...) {if(!(Expr)) {JHNET_LOG(Error, "ASSERTION : %s", TEXT("'"#Expr"'")); return __VA_ARGS__;}}