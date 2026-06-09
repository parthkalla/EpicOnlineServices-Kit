// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "OnlineSubsystemImpl.h"

class FUniqueNetId;
class IVoiceChatUser;
using IEOKPlatformHandlePtr = TSharedPtr<class IEOKPlatformHandle, ESPMode::ThreadSafe>;

/**
 *	OnlineSubsystemEOS - Implementation of the online subsystem for EOS services
 */
class OnlineSubsystemEOK_API IOnlineSubsystemEOS : 
	public FOnlineSubsystemImpl
{
public:
	IOnlineSubsystemEOS(FName InSubsystemName, FName InInstanceName) : FOnlineSubsystemImpl(InSubsystemName, InInstanceName) {}
	virtual ~IOnlineSubsystemEOS() = default;

	virtual IVoiceChatUser* GetVoiceChatUserInterface(const FUniqueNetId& LocalUserId) = 0;
	virtual IEOKPlatformHandlePtr GetEOSPlatformHandle() const = 0;
};