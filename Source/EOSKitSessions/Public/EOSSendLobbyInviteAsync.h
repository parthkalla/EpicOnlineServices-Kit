// Copyright (C) 2024, All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintAsyncActionBase.h"
#include "EOSKitSessionStructs.h"
#include "EOSSendLobbyInviteAsync.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FSendLobbyInvite_Delegate, bool, bSuccess, FString, LobbyId);

/**
 * Send Lobby Invite - Invite another user to join your lobby
 * 
 * Only lobby members can send invites
 */
UCLASS()
class EOSKITSESSIONS_API UEOSSendLobbyInviteAsync : public UBlueprintAsyncActionBase
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintAssignable, DisplayName="On Complete")
	FSendLobbyInvite_Delegate OnComplete;

	/**
	 * Send an invite to another user
	 * 
	 * Important:
	 * - Local user must be a member of the lobby
	 * - Target user must not already be in the lobby
	 * - Target user will receive notification
	 * - Invite can be accepted or rejected
	 * 
	 * Requirements:
	 * - Lobby must allow invites (bAllowInvites=true)
	 * - Local user must have invite privileges
	 * - Target user must be on friends list (platform-dependent)
	 * 
	 * @param LocalUserId - Product User ID of the inviter (must be lobby member)
	 * @param LobbyId - ID of the lobby to invite to
	 * @param TargetUserId - Product User ID of the user to invite
	 */
	UFUNCTION(BlueprintCallable, DisplayName="Send Lobby Invite", meta = (BlueprintInternalUseOnly = "true"), Category="EOSKit|Lobby")
	static UEOSSendLobbyInviteAsync* SendLobbyInvite(FString LocalUserId, FString LobbyId, FString TargetUserId);

	virtual void Activate() override;

private:
	void PerformSendInvite();
	
	FString VarLocalUserId;
	FString VarLobbyId;
	FString VarTargetUserId;
};
