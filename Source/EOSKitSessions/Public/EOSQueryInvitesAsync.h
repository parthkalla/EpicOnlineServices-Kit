// Copyright (C) 2024, All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintAsyncActionBase.h"
#include "EOSQueryInvitesAsync.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FQueryInvites_Delegate, int32, InviteCount);

/**
 * Query Invites - Async Blueprint node for retrieving session invites
 */
UCLASS()
class EOSKITSESSIONS_API UEOSQueryInvitesAsync : public UBlueprintAsyncActionBase
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintAssignable, DisplayName="Success")
	FQueryInvites_Delegate OnSuccess;
	
	UPROPERTY(BlueprintAssignable, DisplayName="Failure")
	FQueryInvites_Delegate OnFail;

	/**
	 * Retrieve all existing invites for the local user
	 */
	UFUNCTION(BlueprintCallable, DisplayName="Query Invites", meta = (BlueprintInternalUseOnly = "true"), Category="EOSKit|Sessions")
	static UEOSQueryInvitesAsync* QueryInvites();

	virtual void Activate() override;

private:
	void QuerySessionInvites();
	bool bDelegateCalled = false;
};
