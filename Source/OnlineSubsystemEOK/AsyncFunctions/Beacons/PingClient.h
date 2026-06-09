// Copyright (c) 2024 Betide Studio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "FindSessionsCallbackProxy.h"
#include "Misc/ConfigCacheIni.h"
#include "OnlineBeaconClient.h"
#include "PingClient.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogPingClient, Log, All);

DECLARE_DYNAMIC_DELEGATE_TwoParams(FEOK_PingComplete, int32, PingMS, bool, bSuccess);

UCLASS(Blueprintable, BlueprintType, Transient, NotPlaceable, Config = Engine)
class OnlineSubsystemEOK_API APingClient : public AOnlineBeaconClient

{
	GENERATED_BODY()

public:
	APingClient(const FObjectInitializer& ObjectInitializer);

	virtual void OnFailure() override;

	UFUNCTION(Client, Reliable)
	void ClientPingBegin();

	UFUNCTION(Server, Reliable, WithValidation)
	void ServerPong();

	UFUNCTION(Client, Reliable)
	void ClientPingEnd();

	UFUNCTION(BlueprintCallable, Category=Ping)
	bool ConnectToHost(FString Address, int32 Port, const bool bPortOverride, FEOK_PingComplete Ref);

	UFUNCTION(BlueprintCallable, Category=Ping)
	bool ConnectToSession(FBlueprintSessionResult SearchResult, FEOK_PingComplete Ref);

	UFUNCTION(BlueprintCallable, Category=Ping)
	void Disconnect();

	FEOK_PingComplete OnPingComplete;
protected:
	int64 PingBeginTime;
	int32 PingMS;
};