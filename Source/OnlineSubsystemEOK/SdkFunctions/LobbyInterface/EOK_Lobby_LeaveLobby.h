// Copyright (c) 2023 Betide Studio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintAsyncActionBase.h"
#include "OnlineSessionEOS.h"
#include "OnlineSubsystemEOS.h"
#include "Async/Async.h"
#include "OnlineSubsystemEOK/SdkFunctions/ConnectInterface/EOK_ConnectSubsystem.h"
#include "OnlineSubsystemEOK/SdkFunctions/EOK_SharedFunctionFile.h"
#include "EOK_Lobby_LeaveLobby.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FEOKLobbyLeaveLobby, const TEnumAsByte<EEOK_Result>&, ResultCode, const FEOK_LobbyId&, LobbyId);

UCLASS()
class OnlineSubsystemEOK_API UEOK_Lobby_LeaveLobby : public UBlueprintAsyncActionBase
{
	GENERATED_BODY()
	
public:

	//Leave a lobby given a lobby ID If the lobby you are leaving had an RTC Room enabled, leaving the lobby will also automatically leave the RTC room.
	UFUNCTION(BlueprintCallable, Category = "Epic Online Services-Kit V2 | SDK Functions | Lobby Interface", DisplayName="EOS_Lobby_LeaveLobby")
	static UEOK_Lobby_LeaveLobby* EOK_Lobby_LeaveLobby(FEOK_ProductUserId LocalUserId, FEOK_LobbyId LobbyId);

	UPROPERTY(BlueprintAssignable)
	FEOKLobbyLeaveLobby OnCallback;
	
private:
	static void EOS_CALL OnLeaveLobbyComplete(const EOS_Lobby_LeaveLobbyCallbackInfo* Data);
	virtual void Activate() override;
	FEOK_ProductUserId Var_LocalUserId;
	FEOK_LobbyId Var_LobbyId;
};
