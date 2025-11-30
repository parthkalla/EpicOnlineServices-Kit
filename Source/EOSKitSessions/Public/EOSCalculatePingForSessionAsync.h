// Copyright (C) 2024, All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintAsyncActionBase.h"
#include "FindSessionsCallbackProxy.h"
#include "EOSKitSessionStructs.h"
#include "EOSCalculatePingForSessionAsync.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnCalculatePingForSessionComplete, int32, Ping);

/**
 * Async node to calculate ping for a session
 * Uses Unreal's beacon system to measure latency
 */
UCLASS()
class EOSKITSESSIONS_API UEOSCalculatePingForSessionAsync : public UBlueprintAsyncActionBase
{
	GENERATED_BODY()

public:
	/**
	 * Calculate ping for a session
	 * @param WorldContextObject - World context
	 * @param SessionResult - Session result from search (contains connection info)
	 * @return Async node instance
	 */
	UFUNCTION(BlueprintCallable, Category = "EOSKit|Beacons|Ping", 
		meta = (BlueprintInternalUseOnly = "true", WorldContext = "WorldContextObject"),
		DisplayName = "Calculate Ping For Session")
	static UEOSCalculatePingForSessionAsync* CalculatePingForSession(
		UObject* WorldContextObject,
		const FBlueprintSessionResult& SessionResult
	);

	UPROPERTY(BlueprintAssignable, Category = "EOSKit|Beacons|Ping")
	FOnCalculatePingForSessionComplete OnSuccess;

	UPROPERTY(BlueprintAssignable, Category = "EOSKit|Beacons|Ping")
	FOnCalculatePingForSessionComplete OnFailure;

private:
	virtual void Activate() override;

	UFUNCTION()
	void OnPingComplete(int32 Ping, bool bSuccess);

	UPROPERTY()
	UObject* WorldContextObject;

	FBlueprintSessionResult SessionResult;
};

