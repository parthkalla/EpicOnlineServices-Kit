// Copyright (C) 2024, All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintAsyncActionBase.h"
#include "EOSKitSessionStructs.h"
#include "EOSRejectLobbyInviteAsync.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FRejectLobbyInvite_Delegate, bool, bSuccess);

/**
 * Reject Lobby Invite - Decline a lobby invitation from another user
 * 
 * Removes the invite from the user's invite list
 */
UCLASS()
class EOSKITSESSIONS_API UEOSRejectLobbyInviteAsync : public UBlueprintAsyncActionBase
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintAssignable, DisplayName="On Complete")
	FRejectLobbyInvite_Delegate OnComplete;

	/**
	 * Reject an invite from another user
	 * 
	 * Important:
	 * - Permanently declines the invitation
	 * - Removes invite from local cache
	 * - Notifies the inviter (backend)
	 * - Cannot be undone (user must be re-invited)
	 * 
	 * Use Cases:
	 * - User declines lobby invitation
	 * - User is busy/not interested
	 * - Bulk reject old invites
	 * 
	 * @param LocalUserId - Product User ID of the local user
	 * @param InviteId - ID of the invite to reject
	 */
	UFUNCTION(BlueprintCallable, DisplayName="Reject Lobby Invite", meta = (BlueprintInternalUseOnly = "true"), Category="EOSKit|Lobby")
	static UEOSRejectLobbyInviteAsync* RejectLobbyInvite(FString LocalUserId, FString InviteId);

	virtual void Activate() override;

private:
	void PerformReject();
	
	FString VarLocalUserId;
	FString VarInviteId;
};
