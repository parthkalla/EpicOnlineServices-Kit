// Copyright (C) 2024, All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

/**
 * Lightweight helpers that expose common OnlineSubsystem interactions without depending on the EOSKit module.
 * These mirror the functionality that previously lived in UEOSKitGameInstanceSubsystem so other modules can
 * safely access the data without creating module cycles.
 */
namespace FEOSKitOnlineHelpers
{
	EOSKITSHARED_API FString GetPlayerNickname(int32 LocalUserNum = 0);
	EOSKITSHARED_API bool IsPlayerLoggedIn(int32 LocalUserNum = 0);
	EOSKITSHARED_API FString GetProductUserId(int32 LocalUserNum = 0);
	EOSKITSHARED_API FString GetEpicAccountId(int32 LocalUserNum = 0);

	EOSKITSHARED_API FString GenerateSessionCode(int32 CodeLength = 9);

	EOSKITSHARED_API void RegisterLocalPlayer(UWorld* World, FName SessionName, bool bWasInvited);
	EOSKITSHARED_API void UnregisterLocalPlayer(UWorld* World, FName SessionName);
	EOSKITSHARED_API bool StartSession(UWorld* World, FName SessionName);
	EOSKITSHARED_API bool EndSession(UWorld* World, FName SessionName);

	EOSKITSHARED_API bool ShowFriendsInterface();
	EOSKITSHARED_API bool IsEOSKitInitialized();
}

