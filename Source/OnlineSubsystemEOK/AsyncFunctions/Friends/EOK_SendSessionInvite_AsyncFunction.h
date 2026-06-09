// Copyright (c) 2023 Betide Studio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintAsyncActionBase.h"
#include "OnlineSubsystemEOK/Subsystem/EOK_Subsystem.h"
#include "EOK_SendSessionInvite_AsyncFunction.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnSendSessionInviteComplete, const FString&, Error);
UCLASS()
class OnlineSubsystemEOK_API UEOK_SendSessionInvite_AsyncFunction : public UBlueprintAsyncActionBase
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, meta = (BlueprintInternalUseOnly = "true"), Category="Epic Online Services-Kit V2 || Friends", DisplayName = "Send Session Invite")
	static UEOK_SendSessionInvite_AsyncFunction* SendSessionInvite(
		FName SessionName,
		FEOKUniqueNetId FriendId);

private:
	FEOKUniqueNetId Var_FriendId;
	FName Var_SessionName;
	virtual void Activate() override;

	UPROPERTY(BlueprintAssignable)
	FOnSendSessionInviteComplete OnSuccess;

	UPROPERTY(BlueprintAssignable)
	FOnSendSessionInviteComplete OnFailure;	
};
