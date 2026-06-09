// Copyright (c) 2023 Betide Studio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintAsyncActionBase.h"
#include "OnlineSubsystemEOK/Subsystem/EOK_Subsystem.h"
#include "EOK_InviteFriend_AsyncFunction.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnInviteFriendComplete, const FString&, Error);
UCLASS()
class OnlineSubsystemEOK_API UEOK_InviteFriend_AsyncFunction : public UBlueprintAsyncActionBase
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, meta = (BlueprintInternalUseOnly = "true"), Category="Epic Online Services-Kit V2 || Friends", DisplayName = "Add New Friend")
	static UEOK_InviteFriend_AsyncFunction* InviteFriend(FEOKUniqueNetId FriendId);
	
private:
	FEOKUniqueNetId Var_FriendId;
	void OnInviteComplete(int32 LocalUserNum, bool bWasSuccessful, const FUniqueNetId& FriendId, const FString& ListName, const FString& ErrorStr);
	virtual void Activate() override;

	UPROPERTY(BlueprintAssignable)
	FOnInviteFriendComplete OnSuccess;

	UPROPERTY(BlueprintAssignable)
	FOnInviteFriendComplete OnFailure;
};
