// Copyright (C) 2024, All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "OnlineSubsystemImpl.h"
#include "IEOSKitPlatformHandle.h"

class FUniqueNetId;
class IVoiceChatUser;

/**
 *	OnlineSubsystemEOSKit - Interface for the online subsystem for EOS services
 */
class ONLINESUBSYSTEMEOSKIT_API IOnlineSubsystemEOSKit : 
	public FOnlineSubsystemImpl
{
public:
	IOnlineSubsystemEOSKit(FName InSubsystemName, FName InInstanceName) 
		: FOnlineSubsystemImpl(InSubsystemName, InInstanceName) 
	{}
	
	virtual ~IOnlineSubsystemEOSKit() = default;

#if WITH_EOS_SDK
	virtual IVoiceChatUser* GetVoiceChatUserInterface(const FUniqueNetId& LocalUserId) = 0;
	virtual IEOSKitPlatformHandlePtr GetEOSPlatformHandle() const = 0;
#endif
};

