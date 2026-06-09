// Copyright (c) 2023 Betide Studio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintAsyncActionBase.h"
#include "OnlineSubsystemEOK/Subsystem/EOK_Subsystem.h"
#include "EOK_AcceptFriendInvite_AsyncFunction.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnAcceptFriendInviteComplete, const FString&, Error);
UCLASS()
class OnlineSubsystemEOK_API UEOK_AcceptFriendInvite_AsyncFunction : public UBlueprintAsyncActionBase
{
	GENERATED_BODY()
public:
	
	UFUNCTION(BlueprintCallable, meta = (BlueprintInternalUseOnly = "true"), Category="Epic Online Services-Kit V2 || Friends", DisplayName = "Accept Friend Invite")
	static UEOK_AcceptFriendInvite_AsyncFunction* AcceptFriendInvite(FEOKUniqueNetId FriendId);

private:
	void OnAcceptComplete(int32 LocalUserNum, bool bWasSuccessful, const FUniqueNetId& FriendId, const FString& ListName, const FString& ErrorStr);
	virtual void Activate() override;
	FEOKUniqueNetId Var_FriendId;

	UPROPERTY(BlueprintAssignable)
	FOnAcceptFriendInviteComplete OnSuccess;

	UPROPERTY(BlueprintAssignable)
	FOnAcceptFriendInviteComplete OnFailure;
	
};
