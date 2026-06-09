// Copyright (c) 2023 Betide Studio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintAsyncActionBase.h"
#include "OnlineSessionEOS.h"
#include "OnlineSubsystemEOS.h"
#include "Async/Async.h"
#include "OnlineSubsystemEOK/SdkFunctions/ConnectInterface/EOK_ConnectSubsystem.h"
#include "OnlineSubsystemEOK/SdkFunctions/EOK_SharedFunctionFile.h"
#include "EOK_Lobby_KickMember.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FEOKLobbyKickMember, const TEnumAsByte<EEOK_Result>&, ResultCode, const FEOK_LobbyId&, LobbyId);

UCLASS()
class OnlineSubsystemEOK_API UEOK_Lobby_KickMember : public UBlueprintAsyncActionBase
{
	GENERATED_BODY()

public:
	//Kick an existing member from the lobby
	UFUNCTION(BlueprintCallable, Category = "Epic Online Services-Kit V2 | SDK Functions | Lobby Interface", DisplayName="EOS_Lobby_KickMember")
	static UEOK_Lobby_KickMember* EOK_Lobby_KickMember(FEOK_LobbyId LobbyId, FEOK_ProductUserId ProductUserId, FEOK_ProductUserId TargetUserId);

	UPROPERTY(BlueprintAssignable)
	FEOKLobbyKickMember OnCallback;

private:
	static void EOS_CALL OnKickMemberComplete(const EOS_Lobby_KickMemberCallbackInfo* Data);
	virtual void Activate() override;
	FEOK_LobbyId Var_LobbyId;
	FEOK_ProductUserId Var_ProductUserId;
	FEOK_ProductUserId Var_TargetUserId;
	// Store the converted LobbyId ANSI string to keep it alive during async operation
	TArray<ANSICHAR> LobbyIdAnsiStorage;
};
