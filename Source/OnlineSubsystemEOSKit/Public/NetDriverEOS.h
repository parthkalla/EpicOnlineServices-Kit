// Copyright (C) 2024, All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "IpNetDriver.h"
#include "NetDriverEOS.generated.h"

/**
 * Net driver implementation for EOS P2P networking
 * Handles multiplayer replication using Epic Online Services P2P interface
 */
UCLASS(transient, config=Engine)
class ONLINESUBSYSTEMEOSKIT_API UNetDriverEOS : public UIpNetDriver
{
	GENERATED_BODY()

public:
	//~ Begin UNetDriver Interface
	virtual bool IsAvailable() const override;
	virtual bool InitBase(bool bInitAsClient, FNetworkNotify* InNotify, const FURL& URL, bool bReuseAddressAndPort, FString& Error) override;
	virtual bool InitConnect(FNetworkNotify* InNotify, const FURL& ConnectURL, FString& Error) override;
	virtual bool InitListen(FNetworkNotify* InNotify, FURL& LocalURL, bool bReuseAddressAndPort, FString& Error) override;
	virtual void TickDispatch(float DeltaTime) override;
	virtual void ProcessRemoteFunction(class AActor* Actor, class UFunction* Function, void* Parameters, FOutParmRec* OutParms, FFrame* Stack, class UObject* SubObject = nullptr) override;
	virtual void LowLevelSend(TSharedPtr<const FInternetAddr> Address, void* Data, int32 CountBits, FOutPacketTraits& Traits) override;
	virtual void Shutdown() override;
	//~ End UNetDriver Interface

protected:
	/** Initialize the socket subsystem for EOS */
	bool InitializeSocketSubsystem(FString& Error);

private:
	/** Cached socket subsystem for EOS */
	class ISocketSubsystem* EOSSocketSubsystem;

	/** Is the driver initialized */
	bool bIsInitialized;
};
