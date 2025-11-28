// Copyright (C) 2024, All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintAsyncActionBase.h"
#include "EOSKitSessionStructs.h"
#include "EOSLeaveLobbyAsync.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FLeaveLobby_Delegate, bool, bSuccess, FString, LobbyId);

/**
 * Leave Lobby - Leave a lobby that the user is currently in
 * 
 * Automatically leaves RTC room if enabled
 */
UCLASS()
class EOSKITSESSIONS_API UEOSLeaveLobbyAsync : public UBlueprintAsyncActionBase
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintAssignable, DisplayName="On Complete")
	FLeaveLobby_Delegate OnComplete;

	/**
	 * Leave a lobby given a lobby ID
	 * 
	 * Important:
	 * - Any member can leave a lobby
	 * - If owner leaves, ownership may transfer (host migration)
	 * - RTC room will be automatically left if enabled
	 * - User's presence will be updated
	 * 
	 * @param LocalUserId - Product User ID of the local user leaving
	 * @param LobbyId - ID of the lobby to leave
	 */
	UFUNCTION(BlueprintCallable, DisplayName="Leave Lobby", meta = (BlueprintInternalUseOnly = "true"), Category="EOSKit|Lobby")
	static UEOSLeaveLobbyAsync* LeaveLobby(FString LocalUserId, FString LobbyId);

	virtual void Activate() override;

private:
	void PerformLeave();
	
	FString VarLocalUserId;
	FString VarLobbyId;
};
