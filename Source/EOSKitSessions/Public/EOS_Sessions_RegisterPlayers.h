// Copyright (C) 2024, All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintAsyncActionBase.h"
#include "EOSKitSessionsSDKShared.h"
#include "EOSKitSubsystem.h"
#include "EOS_Sessions_RegisterPlayers.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FEOSKit_OnRegisterPlayersCallback, EEOSKitResult, ResultCode, const TArray<FEOSKitProductUserId>&, RegisteredPlayers, const TArray<FEOSKitProductUserId>&, SanctionedPlayers);

UCLASS()
class EOSKITSESSIONS_API UEOS_Sessions_RegisterPlayers : public UBlueprintAsyncActionBase
{
	GENERATED_BODY()

public:
	// Register a group of players with the session
	UFUNCTION(BlueprintCallable, Category = "EOSKit | SDK Functions | Sessions Interface", DisplayName = "EOS_Sessions_RegisterPlayers", meta = (BlueprintInternalUseOnly = "true"))
	static UEOS_Sessions_RegisterPlayers* EOK_Sessions_RegisterPlayers(const FString& SessionName, const TArray<FEOSKitProductUserId>& PlayersToRegister);

	UPROPERTY(BlueprintAssignable, Category = "EOSKit | SDK Functions | Sessions Interface")
	FEOSKit_OnRegisterPlayersCallback OnCallback;

private:
	FString Var_SessionName;
	TArray<FEOSKitProductUserId> Var_PlayersToRegister;

	virtual void Activate() override;
	static void EOS_CALL OnRegisterPlayersCallback(const EOS_Sessions_RegisterPlayersCallbackInfo* Data);
};

