// Copyright (C) 2024, All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "OnlineBeaconClient.h"
#include "EOSKitPingClient.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogEOSKitPingClient, Log, All);

DECLARE_DYNAMIC_DELEGATE_TwoParams(FEOSKitPingComplete, int32, PingMS, bool, bSuccess);

/**
 * Ping client for measuring latency using Unreal's beacon system
 */
UCLASS(Blueprintable, BlueprintType, Transient, NotPlaceable, Config = Engine)
class EOSKITSESSIONS_API AEOSKitPingClient : public AOnlineBeaconClient
{
	GENERATED_BODY()

public:
	AEOSKitPingClient(const FObjectInitializer& ObjectInitializer);

	virtual void OnFailure() override;

	UFUNCTION(Client, Reliable)
	void ClientPingBegin();

	UFUNCTION(Server, Reliable, WithValidation)
	void ServerPong();

	UFUNCTION(Client, Reliable)
	void ClientPingEnd();

	/**
	 * Connect to a host by IP address
	 * @param Address - IP address or hostname
	 * @param Port - Port to connect to (0 to use default from config)
	 * @param bPortOverride - If true, use provided port; if false, use default from config
	 * @param Ref - Callback delegate
	 * @return True if connection initiated successfully
	 */
	UFUNCTION(BlueprintCallable, Category = "EOSKit|Ping")
	bool ConnectToHost(FString Address, int32 Port, bool bPortOverride, FEOSKitPingComplete Ref);

	/**
	 * Connect to a session
	 * @param SessionResult - Session result from search
	 * @param Ref - Callback delegate
	 * @return True if connection initiated successfully
	 */
	UFUNCTION(BlueprintCallable, Category = "EOSKit|Ping")
	bool ConnectToSession(const FBlueprintSessionResult& SessionResult, FEOSKitPingComplete Ref);

	/**
	 * Disconnect from the beacon
	 */
	UFUNCTION(BlueprintCallable, Category = "EOSKit|Ping")
	void Disconnect();

	FEOSKitPingComplete OnPingComplete;

protected:
	int64 PingBeginTime;
	int32 PingMS;
};

