// Copyright (C) 2024, All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintAsyncActionBase.h"
#include "EOSUnregisterPlayersAsync.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FUnregisterPlayers_Delegate, const TArray<FString>&, UnregisteredPlayerIds);

/**
 * Unregister Players - Async Blueprint node for unregistering players from a session
 * Frees up space for others to join
 */
UCLASS()
class EOSKITSESSIONS_API UEOSUnregisterPlayersAsync : public UBlueprintAsyncActionBase
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintAssignable, DisplayName="Success")
	FUnregisterPlayers_Delegate OnSuccess;
	
	UPROPERTY(BlueprintAssignable, DisplayName="Failure")
	FUnregisterPlayers_Delegate OnFail;

	/**
	 * Unregister a group of players from the session
	 * Frees up space for others to join
	 * 
	 * @param SessionName - Name of the session to unregister players from
	 * @param PlayersToUnregister - Array of player Product User IDs to unregister (can pass empty array to unregister local player)
	 */
	UFUNCTION(BlueprintCallable, DisplayName="Unregister Players", meta = (BlueprintInternalUseOnly = "true", AutoCreateRefTerm="PlayersToUnregister"), Category="EOSKit|Sessions")
	static UEOSUnregisterPlayersAsync* UnregisterPlayers(
		FName SessionName,
		const TArray<FString>& PlayersToUnregister
	);

	virtual void Activate() override;

private:
	void UnregisterPlayersFromSession();
	
	FName VSessionName;
	TArray<FString> PlayerIds;
	bool bDelegateCalled = false;
};
