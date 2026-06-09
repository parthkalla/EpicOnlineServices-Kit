// Copyright (c) 2023 Betide Studio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintAsyncActionBase.h"
#include "OnlineSessionEOS.h"
#include "OnlineSubsystemEOS.h"
#include "Async/Async.h"
#include "OnlineSubsystemEOK/SdkFunctions/ConnectInterface/EOK_ConnectSubsystem.h"
#include "OnlineSubsystemEOK/SdkFunctions/EOK_SharedFunctionFile.h"
#include "EOK_Lobby_UpdateLobby.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FEOKLobbyUpdateLobby, const TEnumAsByte<EEOK_Result>&, ResultCode, const FEOK_LobbyId&, LobbyId);
UCLASS()
class OnlineSubsystemEOK_API UEOK_Lobby_UpdateLobby : public UBlueprintAsyncActionBase
{
	GENERATED_BODY()
	
public:

	//Update a lobby given a lobby modification handle created by EOS_Lobby_UpdateLobbyModification
	UFUNCTION(BlueprintCallable, Category = "Epic Online Services-Kit V2 | SDK Functions | Lobby Interface", DisplayName="EOS_Lobby_UpdateLobby")
	static UEOK_Lobby_UpdateLobby* EOK_Lobby_UpdateLobby(FEOK_HLobbyModification LobbyModificationHandle);

	UPROPERTY(BlueprintAssignable)
	FEOKLobbyUpdateLobby OnCallback;
private:
	virtual void Activate() override;
	static void EOS_CALL OnUpdateLobbyComplete(const EOS_Lobby_UpdateLobbyCallbackInfo* Data);
	FEOK_HLobbyModification Var_LobbyModificationHandle;
};
