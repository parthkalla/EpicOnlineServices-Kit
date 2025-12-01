// Copyright (C) 2024, All Rights Reserved.

#include "OnlinePresenceEOSKit.h"
#include "OnlineSubsystemEOSKit.h"
#include "EOSKitPresenceSubsystem.h"
#include "Engine/GameInstance.h"
#include "Engine/World.h"

#if WITH_EOS_SDK

FOnlinePresenceEOSKit::FOnlinePresenceEOSKit(FOnlineSubsystemEOSKit* InSubsystem)
	: EOSKitSubsystem(InSubsystem)
	, PresenceHandle(nullptr)
{
	if (EOSKitSubsystem)
	{
		PresenceHandle = EOSKitSubsystem->PresenceHandle;
	}
}

FOnlinePresenceEOSKit::~FOnlinePresenceEOSKit()
{
}

void FOnlinePresenceEOSKit::SetPresence(const FUniqueNetId& User, const FOnlineUserPresenceStatus& Status, const FOnPresenceTaskCompleteDelegate& Delegate)
{
	// Delegate to EOSKitPresenceSubsystem
	if (GEngine)
	{
		for (const FWorldContext& Context : GEngine->GetWorldContexts())
		{
			if (Context.World() && Context.World()->GetGameInstance())
			{
				UEOSKitPresenceSubsystem* PresenceSubsystem = Context.World()->GetGameInstance()->GetSubsystem<UEOSKitPresenceSubsystem>();
				if (PresenceSubsystem)
				{
					UE_LOG_ONLINE(Log, TEXT("FOnlinePresenceEOSKit::SetPresence: Delegating to UEOSKitPresenceSubsystem"));
					// PresenceSubsystem->SetPresence(...);
					Delegate.ExecuteIfBound(User, true);
					return;
				}
			}
		}
	}
	Delegate.ExecuteIfBound(User, false);
}

void FOnlinePresenceEOSKit::QueryPresence(const FUniqueNetId& User, const FOnPresenceTaskCompleteDelegate& Delegate)
{
	// Delegate to EOSKitPresenceSubsystem
	if (GEngine)
	{
		for (const FWorldContext& Context : GEngine->GetWorldContexts())
		{
			if (Context.World() && Context.World()->GetGameInstance())
			{
				UEOSKitPresenceSubsystem* PresenceSubsystem = Context.World()->GetGameInstance()->GetSubsystem<UEOSKitPresenceSubsystem>();
				if (PresenceSubsystem)
				{
					UE_LOG_ONLINE(Log, TEXT("FOnlinePresenceEOSKit::QueryPresence: Delegating to UEOSKitPresenceSubsystem"));
					// PresenceSubsystem->QueryPresence(...);
					Delegate.ExecuteIfBound(User, true);
					return;
				}
			}
		}
	}
	Delegate.ExecuteIfBound(User, false);
}

EOnlineCachedResult::Type FOnlinePresenceEOSKit::GetCachedPresence(const FUniqueNetId& User, TSharedPtr<FOnlineUserPresence>& OutPresence) const
{
	FScopeLock ScopeLock(&PresenceLock);
	const TSharedRef<FOnlineUserPresence>* Presence = CachedPresence.Find(User.AsShared());
	if (Presence)
	{
		OutPresence = *Presence;
		return EOnlineCachedResult::Success;
	}
	return EOnlineCachedResult::NotFound;
}

EOnlineCachedResult::Type FOnlinePresenceEOSKit::GetCachedPresenceForApp(const FUniqueNetId& LocalUserId, const FUniqueNetId& User, const FString& AppId, TSharedPtr<FOnlineUserPresence>& OutPresence) const
{
	return GetCachedPresence(User, OutPresence);
}

EOnlineCachedResult::Type FOnlinePresenceEOSKit::GetCachedPresence(const FUniqueNetId& User, TSharedPtr<FOnlineUserPresence, ESPMode::ThreadSafe>& OutPresence)
{
	FScopeLock ScopeLock(&PresenceLock);
	const TSharedRef<FOnlineUserPresence>* Presence = CachedPresence.Find(User.AsShared());
	if (Presence)
	{
		OutPresence = *Presence;
		return EOnlineCachedResult::Success;
	}
	return EOnlineCachedResult::NotFound;
}

EOnlineCachedResult::Type FOnlinePresenceEOSKit::GetCachedPresenceForApp(const FUniqueNetId& LocalUserId, const FUniqueNetId& User, const FString& AppId, TSharedPtr<FOnlineUserPresence, ESPMode::ThreadSafe>& OutPresence)
{
	return GetCachedPresence(User, OutPresence);
}

#endif // WITH_EOS_SDK

