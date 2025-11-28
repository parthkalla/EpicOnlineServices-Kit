// Copyright (C) 2024, All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintAsyncActionBase.h"
#include "EOSKitSessionStructs.h"
#include "EOSDestroyLobbyAsync.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FDestroyLobby_Delegate, bool, bSuccess, FString, LobbyId);

/**
 * Destroy Lobby - Destroy a lobby given a lobby ID
 * 
 * Only the lobby owner can destroy the lobby
 */
UCLASS()
class EOSKITSESSIONS_API UEOSDestroyLobbyAsync : public UBlueprintAsyncActionBase
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintAssignable, DisplayName="On Complete")
	FDestroyLobby_Delegate OnComplete;

	/**
	 * Destroy a lobby given a lobby ID
	 * 
	 * Important:
	 * - Only the lobby owner can destroy the lobby
	 * - All members will be disconnected
	 * - Lobby data will be permanently deleted
	 * - RTC room will be closed if enabled
	 * 
	 * @param LocalUserId - Product User ID of the local user (must be lobby owner)
	 * @param LobbyId - ID of the lobby to destroy
	 */
	UFUNCTION(BlueprintCallable, DisplayName="Destroy Lobby", meta = (BlueprintInternalUseOnly = "true"), Category="EOSKit|Lobby")
	static UEOSDestroyLobbyAsync* DestroyLobby(FString LocalUserId, FString LobbyId);

	virtual void Activate() override;

private:
	void PerformDestroy();
	
	FString VarLocalUserId;
	FString VarLobbyId;
};
