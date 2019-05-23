// Copyright 1998-2018 Epic Games, Inc. All Rights Reserved.

#pragma once
#include "Engine.h"
DECLARE_LOG_CATEGORY_EXTERN(JHNET_LOG, Log, All);

#define LOG_CALLINFO (FString(__FUNCTION__) + TEXT("(") + FString::FromInt(__LINE__) + TEXT(")"))
#define LOG_S(Verbosity) UE_LOG(JHNET_LOG, Verbosity, TEXT("%s"), *LOG_CALLINFO)
#define LOG(Verbosity, Format, ...) UE_LOG(JHNET_LOG, Verbosity, TEXT("%s %s"), *LOG_CALLINFO, *FString::Printf(TEXT(Format), ##__VA_ARGS__))
#define LOG_STRING(Verbosity, STRING) UE_LOG(JHNET_LOG, Verbosity, TEXT("%s %s"), *LOG_CALLINFO, *STRING)
#define	LOG_SCREEN(Format, ...) GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, FString::Printf(TEXT(Format), ##__VA_ARGS__))
#define	LOG_SCREEN_T(Time, Format, ...) GEngine->AddOnScreenDebugMessage(-1, Time, FColor::Red, FString::Printf(TEXT(Format), ##__VA_ARGS__))
#define LOG_WARNING(Format, ...) UE_LOG(JHNET_LOG, Warning, TEXT("%s %s"), *LOG_CALLINFO, *FString::Printf(TEXT(Format), ##__VA_ARGS__))
#define LOG_ERROR(Format, ...) UE_LOG(JHNET_LOG, Error, TEXT("%s %s"), *LOG_CALLINFO, *FString::Printf(TEXT(Format), ##__VA_ARGS__))
#define CHECK(Expr, ...) {if(!(Expr)) {LOG(Error, "ASSERTION : %s", TEXT("'"#Expr"'")); return __VA_ARGS__;}}