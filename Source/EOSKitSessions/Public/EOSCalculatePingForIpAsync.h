// Copyright (C) 2024, All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintAsyncActionBase.h"
#include "EOSCalculatePingForIpAsync.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnCalculatePingForIpComplete, int32, Ping);

/**
 * Async node to calculate ping for an IP address
 * Uses Unreal's beacon system to measure latency
 */
UCLASS()
class EOSKITSESSIONS_API UEOSCalculatePingForIpAsync : public UBlueprintAsyncActionBase
{
	GENERATED_BODY()

public:
	/**
	 * Calculate ping for an IP address
	 * @param WorldContextObject - World context
	 * @param IpAddress - IP address to ping
	 * @param Port - Port to connect to (0 to use default from config)
	 * @param bPortOverride - If true, use provided port; if false, use default from config
	 * @return Async node instance
	 */
	UFUNCTION(BlueprintCallable, Category = "EOSKit|Beacons|Ping",
		meta = (BlueprintInternalUseOnly = "true", WorldContext = "WorldContextObject"),
		DisplayName = "Calculate Ping For IP")
	static UEOSCalculatePingForIpAsync* CalculatePingForIp(
		UObject* WorldContextObject,
		const FString& IpAddress,
		int32 Port = 0,
		bool bPortOverride = false
	);

	UPROPERTY(BlueprintAssignable, Category = "EOSKit|Beacons|Ping")
	FOnCalculatePingForIpComplete OnSuccess;

	UPROPERTY(BlueprintAssignable, Category = "EOSKit|Beacons|Ping")
	FOnCalculatePingForIpComplete OnFailure;

private:
	virtual void Activate() override;

	UFUNCTION()
	void OnPingComplete(int32 Ping, bool bSuccess);

	UPROPERTY()
	UObject* WorldContextObject;

	FString IpAddress;
	int32 Port;
	bool bPortOverride;
};

