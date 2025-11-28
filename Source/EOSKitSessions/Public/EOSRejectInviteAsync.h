// Copyright (C) 2024, All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintAsyncActionBase.h"
#include "EOSRejectInviteAsync.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FRejectInvite_Delegate);

/**
 * Reject Invite - Async Blueprint node for rejecting session invites
 */
UCLASS()
class EOSKITSESSIONS_API UEOSRejectInviteAsync : public UBlueprintAsyncActionBase
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintAssignable, DisplayName="Success")
	FRejectInvite_Delegate OnSuccess;
	
	UPROPERTY(BlueprintAssignable, DisplayName="Failure")
	FRejectInvite_Delegate OnFail;

	/**
	 * Reject an invite from another player
	 * 
	 * @param InviteId - The invite ID to reject (obtained from Query Invites)
	 */
	UFUNCTION(BlueprintCallable, DisplayName="Reject Invite", meta = (BlueprintInternalUseOnly = "true"), Category="EOSKit|Sessions")
	static UEOSRejectInviteAsync* RejectInvite(FString InviteId);

	virtual void Activate() override;

private:
	void RejectSessionInvite();
	
	FString InviteId;
	bool bDelegateCalled = false;
};
