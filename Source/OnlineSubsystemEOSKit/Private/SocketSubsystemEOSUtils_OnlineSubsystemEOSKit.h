// Copyright (C) 2024, All Rights Reserved.

#pragma once 

#include "SocketSubsystemEOSUtils.h"

class FOnlineSubsystemEOSKit;

class ONLINESUBSYSTEMEOSKIT_API FSocketSubsystemEOSUtils_OnlineSubsystemEOSKit : public ISocketSubsystemEOSUtils
{
public:
	FSocketSubsystemEOSUtils_OnlineSubsystemEOSKit(FOnlineSubsystemEOSKit& InSubsystemEOSKit);
	virtual ~FSocketSubsystemEOSUtils_OnlineSubsystemEOSKit() override;

#if WITH_EOS_SDK
	virtual EOS_ProductUserId GetLocalUserId() override;
	virtual void* GetPlatformHandle() override;
#endif
	virtual FString GetSessionId() override;
	virtual FName GetSubsystemInstanceName() override;

private:
	FOnlineSubsystemEOSKit& SubsystemEOSKit;
};

