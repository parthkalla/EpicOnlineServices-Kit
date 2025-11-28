// Copyright (C) 2024, All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintAsyncActionBase.h"
#include "EOSSendInviteAsync.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FSendInvite_Delegate);

/**
 * Send Invite - Async Blueprint node for sending session invites to friends
 */
UCLASS()
class EOSKITSESSIONS_API UEOSSendInviteAsync : public UBlueprintAsyncActionBase
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintAssignable, DisplayName="Success")
	FSendInvite_Delegate OnSuccess;
	
	UPROPERTY(BlueprintAssignable, DisplayName="Failure")
	FSendInvite_Delegate OnFail;

	/**
	 * Send an invite to another player
	 * User must have created the session or be registered in the session
	 * 
	 * @param SessionName - Name of the session to invite to
	 * @param TargetUserId - Product User ID of the player to invite (as string)
	 */
	UFUNCTION(BlueprintCallable, DisplayName="Send Invite", meta = (BlueprintInternalUseOnly = "true"), Category="EOSKit|Sessions")
	static UEOSSendInviteAsync* SendInvite(
		FName SessionName,
		FString TargetUserId
	);

	virtual void Activate() override;

private:
	void SendSessionInvite();
	
	FName VSessionName;
	FString TargetProductUserId;
	bool bDelegateCalled = false;
};
