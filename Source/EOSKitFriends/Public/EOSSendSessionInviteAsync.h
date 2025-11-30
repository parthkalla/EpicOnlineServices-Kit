// Copyright (C) 2024, All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintAsyncActionBase.h"
#include "EOSKitSharedTypes.h"
#include "EOSSendSessionInviteAsync.generated.h"

/**
 * Delegate for send session invite completion
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnEOSSendSessionInviteComplete, const FString&, Error);

/**
 * Async node to send a session invite to a friend
 * This is a convenience function that sends a session invite to a friend using their Epic Account ID
 */
UCLASS()
class EOSKITFRIENDS_API UEOSSendSessionInviteAsync : public UBlueprintAsyncActionBase
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintAssignable, Category = "EOSKit|Friends")
	FOnEOSSendSessionInviteComplete OnSuccess;

	UPROPERTY(BlueprintAssignable, Category = "EOSKit|Friends")
	FOnEOSSendSessionInviteComplete OnFailure;

	/**
	 * Send a session invite to a friend
	 * @param WorldContextObject - World context object
	 * @param TargetEpicAccountId - Epic Account ID of the friend to invite
	 * @param SessionName - Name of the session to invite to
	 * @return Async node instance
	 */
	UFUNCTION(BlueprintCallable, DisplayName = "Send Session Invite to Friend",
		meta = (BlueprintInternalUseOnly = "true", WorldContext = "WorldContextObject",
		ToolTip = "Send a session invite to a friend using their Epic Account ID"),
		Category = "EOSKit|Friends")
	static UEOSSendSessionInviteAsync* SendSessionInvite(UObject* WorldContextObject, 
		const FEOSKitEpicAccountId& TargetEpicAccountId, 
		const FString& SessionName);

	virtual void Activate() override;

private:
	UObject* WorldContextObject;
	FEOSKitEpicAccountId TargetEpicAccountId;
	FString SessionNameString;
};

