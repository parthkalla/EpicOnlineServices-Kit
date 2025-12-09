// Copyright (C) 2024, All Rights Reserved.

#include "SocketSubsystemEOSUtils_OnlineSubsystemEOSKit.h"
#include "OnlineSessionEOSKit.h"
#include "OnlineIdentityEOSKit.h"
#include "OnlineSubsystemEOSKit.h"
#include "EOSKitSubsystem.h"

FSocketSubsystemEOSUtils_OnlineSubsystemEOSKit::FSocketSubsystemEOSUtils_OnlineSubsystemEOSKit(FOnlineSubsystemEOSKit& InSubsystemEOSKit)
	: SubsystemEOSKit(InSubsystemEOSKit)
{
}

FSocketSubsystemEOSUtils_OnlineSubsystemEOSKit::~FSocketSubsystemEOSUtils_OnlineSubsystemEOSKit()
{
}

#if WITH_EOS_SDK
EOS_ProductUserId FSocketSubsystemEOSUtils_OnlineSubsystemEOSKit::GetLocalUserId()
{
	EOS_ProductUserId Result = nullptr;

	FOnlineIdentityEOSKitPtr IdentityInterface = StaticCastSharedPtr<FOnlineIdentityEOSKit>(SubsystemEOSKit.GetIdentityInterface());
	if (IdentityInterface.IsValid())
	{
		// Get the local user's ProductUserId
		TSharedPtr<const FUniqueNetId> UniqueId = IdentityInterface->GetUniquePlayerId(0);
		if (UniqueId.IsValid())
		{
			// Try to get ProductUserId from EOSKitSubsystem
			if (GEngine)
			{
				for (const FWorldContext& Context : GEngine->GetWorldContexts())
				{
					if (Context.World() && Context.World()->GetGameInstance())
					{
						UEOSKitSubsystem* EOSKitSubsystem = Context.World()->GetGameInstance()->GetSubsystem<UEOSKitSubsystem>();
						if (EOSKitSubsystem)
						{
							Result = EOSKitSubsystem->GetProductUserId();
							break;
						}
					}
				}
			}
		}
	}

	return Result;
}

void* FSocketSubsystemEOSUtils_OnlineSubsystemEOSKit::GetPlatformHandle()
{
	IEOSKitPlatformHandlePtr PlatformHandle = SubsystemEOSKit.GetEOSPlatformHandle();
	if (PlatformHandle.IsValid())
	{
#if WITH_EOS_SDK
		// Access the PlatformHandle member directly
		return static_cast<void*>(PlatformHandle->PlatformHandle);
#else
		return PlatformHandle.Get();
#endif
	}
	return nullptr;
}
#endif

FString FSocketSubsystemEOSUtils_OnlineSubsystemEOSKit::GetSessionId()
{
	FString Result;

	const IOnlineSessionPtr DefaultSessionInt = SubsystemEOSKit.GetSessionInterface();
	if (DefaultSessionInt.IsValid())
	{
		if (const FNamedOnlineSession* const NamedSession = DefaultSessionInt->GetNamedSession(NAME_GameSession))
		{
			Result = NamedSession->GetSessionIdStr();
		}
	}

	return Result;
}

FName FSocketSubsystemEOSUtils_OnlineSubsystemEOSKit::GetSubsystemInstanceName()
{
	return SubsystemEOSKit.GetInstanceName();
}

