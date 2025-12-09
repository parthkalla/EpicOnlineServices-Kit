// Copyright (C) 2024, All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "IpConnection.h"
#include "NetConnectionEOS.generated.h"

UCLASS(Transient, Config=Engine)
class ONLINESUBSYSTEMEOSKIT_API UNetConnectionEOS
	: public UIpConnection
{
	GENERATED_BODY()

public:
	explicit UNetConnectionEOS(const FObjectInitializer& ObjectInitializer);

//~ Begin UNetConnection Interface
	virtual void InitLocalConnection(UNetDriver* InDriver, FSocket* InSocket, const FURL& InURL, EConnectionState InState, int32 InMaxPacket = 0, int32 InPacketOverhead = 0) override;
	virtual void InitRemoteConnection(UNetDriver* InDriver, FSocket* InSocket, const FURL& InURL, const FInternetAddr& InRemoteAddr, EConnectionState InState, int32 InMaxPacket = 0, int32 InPacketOverhead = 0) override;
	virtual void CleanUp() override;
//~ End UNetConnection Interface

protected:
	// Override to bypass IP-based validation for EOS P2P connections
	// This method is called during PreLogin validation to check if the remote UniqueNetId is valid
	// Note: Method name may vary by UE version - try LowLevelValidateRemoteUniqueId first
	virtual bool LowLevelValidateRemoteUniqueId(FUniqueNetIdRepl& UniqueId);

	void DestroyEOSConnection();

public:
	bool bIsPassthrough;

protected:
	bool bHasP2PSession;
};

