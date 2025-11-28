// Copyright (C) 2024, All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintAsyncActionBase.h"
#include "EOSKitSessionStructs.h"
#include "EOSQueryLobbyInvitesAsync.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FQueryLobbyInvites_Delegate, bool, bSuccess);

/**
 * Query Lobby Invites - Retrieve all existing lobby invites for a user
 * 
 * This refreshes the user's lobby invite list from the backend
 */
UCLASS()
class EOSKITSESSIONS_API UEOSQueryLobbyInvitesAsync : public UBlueprintAsyncActionBase
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintAssignable, DisplayName="On Complete")
	FQueryLobbyInvites_Delegate OnComplete;

	/**
	 * Retrieve all existing lobby invites for a single user
	 * 
	 * Important:
	 * - Queries the backend for the latest lobby invite data
	 * - Must be called before accessing invite information
	 * - Updates the local cache of invites
	 * - Required before GetInviteCount or GetInviteIdByIndex
	 * 
	 * Use Cases:
	 * - Refresh invite list when entering lobby browser
	 * - Check for new lobby invites periodically
	 * - Update invite UI
	 * 
	 * @param LocalUserId - Product User ID of the local user
	 */
	UFUNCTION(BlueprintCallable, DisplayName="Query Lobby Invites", meta = (BlueprintInternalUseOnly = "true"), Category="EOSKit|Lobby")
	static UEOSQueryLobbyInvitesAsync* QueryLobbyInvites(FString LocalUserId);

	virtual void Activate() override;

private:
	void PerformQuery();
	
	FString VarLocalUserId;
};
