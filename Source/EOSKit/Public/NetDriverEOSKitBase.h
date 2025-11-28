// Copyright (C) 2024, All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "IpNetDriver.h"
#include "NetDriverEOSKitBase.generated.h"

/**
 * Base network driver for EOS Kit
 * Extends IP network driver with EOS-specific functionality
 */
UCLASS(Transient, Config=Engine)
class EOSKIT_API UNetDriverEOSKitBase : public UIpNetDriver
{
	GENERATED_BODY()

public:
	UNetDriverEOSKitBase(const FObjectInitializer& ObjectInitializer);

	// Begin UNetDriver interface
	virtual bool IsAvailable() const override;
	virtual bool InitBase(bool bInitAsClient, FNetworkNotify* InNotify, const FURL& URL, bool bReuseAddressAndPort, FString& Error) override;
	virtual bool InitConnect(FNetworkNotify* InNotify, const FURL& ConnectURL, FString& Error) override;
	virtual bool InitListen(FNetworkNotify* InNotify, FURL& LocalURL, bool bReuseAddressAndPort, FString& Error) override;
	virtual void TickDispatch(float DeltaTime) override;
	virtual void ProcessRemoteFunction(class AActor* Actor, class UFunction* Function, void* Parameters, struct FOutParmRec* OutParms, struct FFrame* Stack, class UObject* SubObject = nullptr) override;
	virtual void LowLevelSend(TSharedPtr<const FInternetAddr> Address, void* Data, int32 CountBits, FOutPacketTraits& Traits) override;
	virtual void Shutdown() override;
	// End UNetDriver interface

protected:
	/** Whether we have been initialized */
	bool bIsInitialized;

	/** Socket subsystem for EOS */
	class ISocketSubsystem* SocketSubsystem;
};
