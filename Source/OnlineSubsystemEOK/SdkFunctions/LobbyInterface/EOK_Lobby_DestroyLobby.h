// Copyright (c) 2023 Betide Studio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintAsyncActionBase.h"
#include "OnlineSubsystemEOK/SdkFunctions/EOK_SharedFunctionFile.h"
#include "EOK_Lobby_DestroyLobby.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FEOK_Lobby_DestroyLobbyDelegate, const TEnumAsByte<EEOK_Result>&, Result, const FEOK_LobbyId&, LobbyId);
UCLASS()
class OnlineSubsystemEOK_API UEOK_Lobby_DestroyLobby : public UBlueprintAsyncActionBase
{
	GENERATED_BODY()

public:

	/**Destroy a lobby given a lobby ID */
	UFUNCTION(BlueprintCallable, Category = "Epic Online Services-Kit V2 | SDK Functions | Lobby Interface", DisplayName="EOS_Lobby_DestroyLobby")
	static UEOK_Lobby_DestroyLobby* EOK_Lobby_DestroyLobby(FEOK_ProductUserId LocalUserId, FEOK_LobbyId LobbyId);

	UPROPERTY(BlueprintAssignable, Category = "Epic Online Services-Kit V2 | SDK Functions | Lobby Interface")
	FEOK_Lobby_DestroyLobbyDelegate OnCallback;

private:
	static void EOS_CALL OnDestroyLobbyComplete(const EOS_Lobby_DestroyLobbyCallbackInfo* Data);
	virtual void Activate() override;
	FEOK_ProductUserId Var_LocalUserId;
	FEOK_LobbyId Var_LobbyId;
};
