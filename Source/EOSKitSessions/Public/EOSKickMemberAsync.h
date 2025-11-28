// Copyright (C) 2024, All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintAsyncActionBase.h"
#include "EOSKitSessionStructs.h"
#include "EOSKickMemberAsync.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FKickMember_Delegate, bool, bSuccess, FString, LobbyId);

/**
 * Kick Member - Remove a member from the lobby
 * 
 * Only the lobby owner or moderators can kick members
 */
UCLASS()
class EOSKITSESSIONS_API UEOSKickMemberAsync : public UBlueprintAsyncActionBase
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintAssignable, DisplayName="On Complete")
	FKickMember_Delegate OnComplete;

	/**
	 * Kick an existing member from the lobby
	 * 
	 * Important:
	 * - Only the lobby owner can kick members
	 * - Kicked member will be immediately disconnected
	 * - Kicked member can rejoin if invited (unless settings prevent it)
	 * - All lobby members will be notified of the kick
	 * 
	 * @param LobbyId - ID of the lobby
	 * @param LocalUserId - Product User ID of the local user (must be owner)
	 * @param TargetUserId - Product User ID of the member to kick
	 */
	UFUNCTION(BlueprintCallable, DisplayName="Kick Member", meta = (BlueprintInternalUseOnly = "true"), Category="EOSKit|Lobby")
	static UEOSKickMemberAsync* KickMember(FString LobbyId, FString LocalUserId, FString TargetUserId);

	virtual void Activate() override;

private:
	void PerformKick();
	
	FString VarLobbyId;
	FString VarLocalUserId;
	FString VarTargetUserId;
};
