// Copyright (c) 2023 Betide Studio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintAsyncActionBase.h"
#include "OnlineSessionEOS.h"
#include "OnlineSubsystemEOS.h"
#include "Async/Async.h"
#include "OnlineSubsystemEOK/SdkFunctions/ConnectInterface/EOK_ConnectSubsystem.h"
#include "OnlineSubsystemEOK/SdkFunctions/EOK_SharedFunctionFile.h"
#include "EOK_Lobby_SendInvite.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FEOKLobbySendInvite, const TEnumAsByte<EEOK_Result>&, ResultCode, const FEOK_LobbyId&, LobbyId);

UCLASS()
class OnlineSubsystemEOK_API UEOK_Lobby_SendInvite : public UBlueprintAsyncActionBase
{
	GENERATED_BODY()
	
public:

	//Send an invite to another user. User must be a member of the lobby or else the call will fail
	UFUNCTION(BlueprintCallable, Category = "Epic Online Services-Kit V2 | SDK Functions | Lobby Interface", DisplayName="EOS_Lobby_SendInvite")
	static UEOK_Lobby_SendInvite* EOK_Lobby_SendInvite(FEOK_ProductUserId LocalUserId, FEOK_LobbyId LobbyId, FEOK_ProductUserId TargetUserId);

	UPROPERTY(BlueprintAssignable)
	FEOKLobbySendInvite OnCallback;
private:
	virtual void Activate() override;
	static void EOS_CALL OnSendInviteComplete(const EOS_Lobby_SendInviteCallbackInfo* Data);
	FEOK_ProductUserId Var_LocalUserId;
	FEOK_LobbyId Var_LobbyId;
	FEOK_ProductUserId Var_TargetUserId;
};
