// Copyright (C) 2024, All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintAsyncActionBase.h"
#include "EOSKitSessionStructs.h"
#include "EOSAcceptInviteAsync.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FAcceptInvite_Delegate);

/**
 * Accept Invite - Async Blueprint node for accepting session invites
 */
UCLASS()
class EOSKITSESSIONS_API UEOSAcceptInviteAsync : public UBlueprintAsyncActionBase
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintAssignable, DisplayName="Success")
	FAcceptInvite_Delegate OnSuccess;
	
	UPROPERTY(BlueprintAssignable, DisplayName="Failure")
	FAcceptInvite_Delegate OnFail;

	/**
	 * Accept an invite from another player
	 * 
	 * @param InviteId - The invite ID to accept (obtained from Query Invites)
	 */
	UFUNCTION(BlueprintCallable, DisplayName="Accept Invite", meta = (BlueprintInternalUseOnly = "true"), Category="EOSKit|Sessions")
	static UEOSAcceptInviteAsync* AcceptInvite(FString InviteId);

	virtual void Activate() override;

private:
	void AcceptSessionInvite();
	
	FString InviteId;
	bool bDelegateCalled = false;
};
