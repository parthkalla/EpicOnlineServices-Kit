// Copyright (C) 2024, All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintAsyncActionBase.h"
#include "EOSKitSessionStructs.h"
#include "EOSPromoteMemberAsync.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FPromoteMember_Delegate, bool, bSuccess, FString, LobbyId);

/**
 * Promote Member - Transfer lobby ownership to another member
 * 
 * Only the current lobby owner can promote members
 */
UCLASS()
class EOSKITSESSIONS_API UEOSPromoteMemberAsync : public UBlueprintAsyncActionBase
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintAssignable, DisplayName="On Complete")
	FPromoteMember_Delegate OnComplete;

	/**
	 * Promote an existing member of the lobby to owner
	 * 
	 * Important:
	 * - Only the current owner can promote members
	 * - The promoted member becomes the new owner
	 * - The original owner becomes a regular member
	 * - New owner gains full lobby modification rights
	 * 
	 * Use Cases:
	 * - Voluntary ownership transfer
	 * - Owner leaving the lobby (manual host migration)
	 * - Delegating lobby management
	 * 
	 * @param LobbyId - ID of the lobby
	 * @param LocalUserId - Product User ID of the current owner
	 * @param TargetUserId - Product User ID of the member to promote
	 */
	UFUNCTION(BlueprintCallable, DisplayName="Promote Member", meta = (BlueprintInternalUseOnly = "true"), Category="EOSKit|Lobby")
	static UEOSPromoteMemberAsync* PromoteMember(FString LobbyId, FString LocalUserId, FString TargetUserId);

	virtual void Activate() override;

private:
	void PerformPromote();
	
	FString VarLobbyId;
	FString VarLocalUserId;
	FString VarTargetUserId;
};
