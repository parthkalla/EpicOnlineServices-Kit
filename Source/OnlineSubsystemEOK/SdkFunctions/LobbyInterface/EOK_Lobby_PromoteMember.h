// Copyright (c) 2023 Betide Studio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintAsyncActionBase.h"
#include "OnlineSessionEOS.h"
#include "OnlineSubsystemEOS.h"
#include "Async/Async.h"
#include "OnlineSubsystemEOK/SdkFunctions/ConnectInterface/EOK_ConnectSubsystem.h"
#include "OnlineSubsystemEOK/SdkFunctions/EOK_SharedFunctionFile.h"
#include "EOK_Lobby_PromoteMember.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FEOKLobbyPromoteMember, const TEnumAsByte<EEOK_Result>&, ResultCode, const FEOK_LobbyId&, LobbyId);

UCLASS()
class OnlineSubsystemEOK_API UEOK_Lobby_PromoteMember : public UBlueprintAsyncActionBase
{
	GENERATED_BODY()
	
public:
	//Promote an existing member of the lobby to owner, allowing them to make lobby data modifications
	UFUNCTION(BlueprintCallable, Category = "Epic Online Services-Kit V2 | SDK Functions | Lobby Interface", DisplayName="EOS_Lobby_PromoteMember")
	static UEOK_Lobby_PromoteMember* EOK_Lobby_PromoteMember(FEOK_LobbyId LobbyId, FEOK_ProductUserId ProductUserId, FEOK_ProductUserId TargetUserId);

	UPROPERTY(BlueprintAssignable)
	FEOKLobbyPromoteMember OnCallback;
	
private:
	static void EOS_CALL OnPromoteMemberComplete(const EOS_Lobby_PromoteMemberCallbackInfo* Data);
	virtual void Activate() override;
	FEOK_LobbyId Var_LobbyId;
	FEOK_ProductUserId Var_ProductUserId;
	FEOK_ProductUserId Var_TargetUserId;
};
