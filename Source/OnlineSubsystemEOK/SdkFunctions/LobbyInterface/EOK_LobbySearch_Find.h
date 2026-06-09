// Copyright (c) 2023 Betide Studio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintAsyncActionBase.h"
#include "OnlineSessionEOS.h"
#include "OnlineSubsystemEOS.h"
#include "Async/Async.h"
#include "OnlineSubsystemEOK/SdkFunctions/ConnectInterface/EOK_ConnectSubsystem.h"
#include "OnlineSubsystemEOK/SdkFunctions/EOK_SharedFunctionFile.h"
#include "EOK_LobbySearch_Find.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FEOKLobbySearchFind, const TEnumAsByte<EEOK_Result>&, ResultCode);
UCLASS()
class OnlineSubsystemEOK_API UEOK_LobbySearch_Find : public UBlueprintAsyncActionBase
{
	GENERATED_BODY()
	
public:

	//Find lobbies matching the search criteria setup via this lobby search handle. When the operation completes, this handle will have the search results that can be parsed
	UFUNCTION(BlueprintCallable, Category = "Epic Online Services-Kit V2 | SDK Functions | Lobby Interface", DisplayName="EOS_LobbySearch_Find")
	static UEOK_LobbySearch_Find* EOK_LobbySearch_Find(FEOK_HLobbySearch Handle, FEOK_ProductUserId LocalUserId);

	UPROPERTY(BlueprintAssignable)
	FEOKLobbySearchFind OnCallback;
	
private:
	virtual void Activate() override;
	static void EOS_CALL OnFindComplete(const EOS_LobbySearch_FindCallbackInfo* Data);
	FEOK_ProductUserId Var_LocalUserId;
	FEOK_HLobbySearch Var_Handle;
};
