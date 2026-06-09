// Copyright (c) 2023 Betide Studio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintAsyncActionBase.h"
#include "OnlineSubsystemEOK/Subsystem/EOK_Subsystem.h"
#include "EOK_RejectFriendInvite_AsyncFunction.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnRejectFriendInviteComplete, const FString&, Error);
UCLASS()
class OnlineSubsystemEOK_API UEOK_RejectFriendInvite_AsyncFunction : public UBlueprintAsyncActionBase
{
	GENERATED_BODY()

public:

	UFUNCTION(BlueprintCallable, meta = (BlueprintInternalUseOnly = "true"), Category="Epic Online Services-Kit V2 || Friends", DisplayName = "Reject Friend Invite")
	static UEOK_RejectFriendInvite_AsyncFunction* RejectFriendInvite(FEOKUniqueNetId FriendId);

private:
	FEOKUniqueNetId Var_FriendId;
	virtual void Activate() override;
	
	UPROPERTY(BlueprintAssignable)
	FOnRejectFriendInviteComplete OnSuccess;

	UPROPERTY(BlueprintAssignable)
	FOnRejectFriendInviteComplete OnFailure;
};
