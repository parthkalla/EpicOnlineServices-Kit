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
#include "EOK_Sessions_UnregisterPlayers.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FEOK_OnUnregisterPlayersCallback, const TEnumAsByte<EEOK_Result>&, ResultCode, const TArray<FEOK_ProductUserId>&, UnregisteredPlayers);
UCLASS()
class OnlineSubsystemEOK_API UEOK_Sessions_UnregisterPlayers : public UBlueprintAsyncActionBase
{
	GENERATED_BODY()

public:
	//Unregister a group of players with the session, freeing up space for others to join
	UFUNCTION(BlueprintCallable, Category = "Epic Online Services-Kit V2 | SDK Functions | Sessions Interface", DisplayName="EOS_Sessions_UnregisterPlayers")
	static UEOK_Sessions_UnregisterPlayers* EOK_Sessions_UnregisterPlayers(FString SessionName, TArray<FEOK_ProductUserId> PlayersToUnregister);

	UPROPERTY(BlueprintAssignable, Category = "Epic Online Services-Kit V2 | SDK Functions | Sessions Interface")
	FEOK_OnUnregisterPlayersCallback OnCallback;

private:
	FString Var_SessionName;
	TArray<FEOK_ProductUserId> Var_PlayersToUnregister;
	virtual void Activate() override;
	static void EOS_CALL OnUnregisterPlayersCallback(const EOS_Sessions_UnregisterPlayersCallbackInfo* Data);
};
