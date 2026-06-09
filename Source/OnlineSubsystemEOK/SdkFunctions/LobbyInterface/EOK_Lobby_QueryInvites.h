// Copyright (c) 2023 Betide Studio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintAsyncActionBase.h"
#include "OnlineSessionEOS.h"
#include "OnlineSubsystemEOS.h"
#include "Async/Async.h"
#include "OnlineSubsystemEOK/SdkFunctions/ConnectInterface/EOK_ConnectSubsystem.h"
#include "OnlineSubsystemEOK/SdkFunctions/EOK_SharedFunctionFile.h"
#include "EOK_Lobby_QueryInvites.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FEOKLobbyQueryInvites, const TEnumAsByte<EEOK_Result>&, ResultCode, const FEOK_LobbyId&, LobbyId);

UCLASS()
class OnlineSubsystemEOK_API UEOK_Lobby_QueryInvites : public UBlueprintAsyncActionBase
{
	GENERATED_BODY()

public:

	//Retrieve all existing invites for a single user
	UFUNCTION(BlueprintCallable, Category = "Epic Online Services-Kit V2 | SDK Functions | Lobby Interface", DisplayName="EOS_Lobby_QueryInvites")
	static UEOK_Lobby_QueryInvites* EOK_Lobby_QueryInvites(FEOK_ProductUserId LocalUserId);

	UPROPERTY(BlueprintAssignable)
	FEOKLobbyQueryInvites OnCallback;

private:
	static void EOS_CALL OnQueryInvitesComplete(const EOS_Lobby_QueryInvitesCallbackInfo* Data);
	virtual void Activate() override;
	FEOK_ProductUserId Var_LocalUserId;
};
