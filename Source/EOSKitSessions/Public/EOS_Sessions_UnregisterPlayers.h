// Copyright (C) 2024, All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintAsyncActionBase.h"
#include "EOSKitSessionsSDKShared.h"
#include "EOSKitSubsystem.h"
#include "EOS_Sessions_UnregisterPlayers.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FEOSKit_OnUnregisterPlayersCallback, const TEnumAsByte<EEOSKitResult>&, ResultCode, const TArray<FEOSKitProductUserId>&, UnregisteredPlayers);

UCLASS()
class EOSKITSESSIONS_API UEOS_Sessions_UnregisterPlayers : public UBlueprintAsyncActionBase
{
	GENERATED_BODY()

public:
	// Unregister a group of players with the session
	UFUNCTION(BlueprintCallable, Category = "EOSKit | SDK Functions | Sessions Interface", DisplayName = "EOS_Sessions_UnregisterPlayers", meta = (BlueprintInternalUseOnly = "true"))
	static UEOS_Sessions_UnregisterPlayers* EOS_Sessions_UnregisterPlayers(const FString& SessionName, const TArray<FEOSKitProductUserId>& PlayersToUnregister);

	UPROPERTY(BlueprintAssignable, Category = "EOSKit | SDK Functions | Sessions Interface")
	FEOSKit_OnUnregisterPlayersCallback OnCallback;

private:
	FString Var_SessionName;
	TArray<FEOSKitProductUserId> Var_PlayersToUnregister;

	virtual void Activate() override;
	static void EOS_CALL OnUnregisterPlayersCallback(const EOS_Sessions_UnregisterPlayersCallbackInfo* Data);
};

