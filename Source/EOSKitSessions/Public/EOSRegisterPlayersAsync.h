// Copyright (C) 2024, All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintAsyncActionBase.h"
#include "EOSRegisterPlayersAsync.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FRegisterPlayers_Delegate, const TArray<FString>&, RegisteredPlayerIds, const TArray<FString>&, SanctionedPlayerIds);

/**
 * Register Players - Async Blueprint node for registering players in a session
 */
UCLASS()
class EOSKITSESSIONS_API UEOSRegisterPlayersAsync : public UBlueprintAsyncActionBase
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintAssignable, DisplayName="Success")
	FRegisterPlayers_Delegate OnSuccess;
	
	UPROPERTY(BlueprintAssignable, DisplayName="Failure")
	FRegisterPlayers_Delegate OnFail;

	/**
	 * Register a group of players with the session
	 * This allows them to invite others or indicate they are part of the session
	 * 
	 * @param SessionName - Name of the session to register players to
	 * @param PlayersToRegister - Array of player Product User IDs to register (can pass empty array to register local player)
	 */
	UFUNCTION(BlueprintCallable, DisplayName="Register Players", meta = (BlueprintInternalUseOnly = "true", AutoCreateRefTerm="PlayersToRegister"), Category="EOSKit|Sessions")
	static UEOSRegisterPlayersAsync* RegisterPlayers(
		FName SessionName,
		const TArray<FString>& PlayersToRegister
	);

	virtual void Activate() override;

private:
	void RegisterPlayersInSession();
	
	FName VSessionName;
	TArray<FString> PlayerIds;
	bool bDelegateCalled = false;
};
