// Copyright (c) 2023 Betide Studio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintAsyncActionBase.h"
#include "OnlineSessionEOS.h"
#include "OnlineSubsystemEOS.h"
#include "Async/Async.h"
#include "OnlineSubsystemEOK/SdkFunctions/ConnectInterface/EOK_ConnectSubsystem.h"
#include "OnlineSubsystemEOK/SdkFunctions/EOK_SharedFunctionFile.h"
#include "EOK_Lobby_RejectInvite.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FEOKLobbyRejectInvite, const TEnumAsByte<EEOK_Result>&, ResultCode, const FString&, InviteId);

UCLASS()
class OnlineSubsystemEOK_API UEOK_Lobby_RejectInvite : public UBlueprintAsyncActionBase
{
	GENERATED_BODY()
	
public:

	//Reject an invite from another user.
	UFUNCTION(BlueprintCallable, Category = "Epic Online Services-Kit V2 | SDK Functions | Lobby Interface", DisplayName="EOS_Lobby_RejectInvite")
	static UEOK_Lobby_RejectInvite* EOK_Lobby_RejectInvite(FEOK_ProductUserId LocalUserId, FString InviteId);

	UPROPERTY(BlueprintAssignable)
	FEOKLobbyRejectInvite OnCallback;

private:
	static void EOS_CALL OnRejectInviteComplete(const EOS_Lobby_RejectInviteCallbackInfo* Data);
	virtual void Activate() override;
	FEOK_ProductUserId Var_LocalUserId;
	FString Var_InviteId;
};
