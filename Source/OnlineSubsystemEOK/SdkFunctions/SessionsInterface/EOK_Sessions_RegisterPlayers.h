// Copyright (c) 2023 Betide Studio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintAsyncActionBase.h"
#include "OnlineSubsystemEOK/SdkFunctions/ConnectInterface/EOK_ConnectSubsystem.h"
#include "OnlineSubsystemEOS.h"
#include "OnlineSubsystemEOK/SdkFunctions/EOK_SharedFunctionFile.h"
THIRD_PARTY_INCLUDES_START
#include "eos_sessions.h"
#include "eos_sessions_types.h"
THIRD_PARTY_INCLUDES_END
#include "EOK_Sessions_RegisterPlayers.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FEOK_OnRegisterPlayersCallback, const TEnumAsByte<EEOK_Result>&, ResultCode, const TArray<FEOK_ProductUserId>&, RegisteredPlayers, const TArray<FEOK_ProductUserId>&, SanctionedPlayers);

UCLASS()
class OnlineSubsystemEOK_API UEOK_Sessions_RegisterPlayers : public UBlueprintAsyncActionBase
{
	GENERATED_BODY()

public:
	//Register a group of players with the session, allowing them to invite others or otherwise indicate they are part of the session for determining a full session
	UFUNCTION(BlueprintCallable, Category = "Epic Online Services-Kit V2 | SDK Functions | Sessions Interface", DisplayName="EOS_Sessions_RegisterPlayers")
	static UEOK_Sessions_RegisterPlayers* EOK_Sessions_RegisterPlayers(FString SessionName, TArray<FEOK_ProductUserId> PlayersToRegister);

	UPROPERTY(BlueprintAssignable, Category = "Epic Online Services-Kit V2 | SDK Functions | Sessions Interface")
	FEOK_OnRegisterPlayersCallback OnCallback;

private:
	FString Var_SessionName;
	TArray<FEOK_ProductUserId> Var_PlayersToRegister;
	virtual void Activate() override;
	static void EOS_CALL OnRegisterPlayersCallback(const EOS_Sessions_RegisterPlayersCallbackInfo* Data);
};
