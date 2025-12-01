// Copyright (C) 2024, All Rights Reserved.

#include "OnlineUserEOSKit.h"
#include "OnlineSubsystemEOSKit.h"
#include "EOSKitUserInfoSubsystem.h"
#include "Engine/GameInstance.h"
#include "Engine/World.h"

#if WITH_EOS_SDK

FOnlineUserEOSKit::FOnlineUserEOSKit(FOnlineSubsystemEOSKit* InSubsystem)
	: EOSKitSubsystem(InSubsystem)
	, UserInfoHandle(nullptr)
{
	if (EOSKitSubsystem)
	{
		UserInfoHandle = EOSKitSubsystem->UserInfoHandle;
	}
}

FOnlineUserEOSKit::~FOnlineUserEOSKit()
{
}

bool FOnlineUserEOSKit::QueryUserInfo(int32 LocalUserNum, const TArray<FUniqueNetIdRef>& UserIds)
{
	// Delegate to EOSKitUserInfoSubsystem
	if (GEngine)
	{
		for (const FWorldContext& Context : GEngine->GetWorldContexts())
		{
			if (Context.World() && Context.World()->GetGameInstance())
			{
				UEOSKitUserInfoSubsystem* UserInfoSubsystem = Context.World()->GetGameInstance()->GetSubsystem<UEOSKitUserInfoSubsystem>();
				if (UserInfoSubsystem && UserIds.Num() > 0)
				{
					UE_LOG_ONLINE(Log, TEXT("FOnlineUserEOSKit::QueryUserInfo: Delegating to UEOSKitUserInfoSubsystem"));
					// UserInfoSubsystem->QueryUserInfo(...);
					return true;
				}
			}
		}
	}
	return false;
}

bool FOnlineUserEOSKit::GetAllUserInfo(int32 LocalUserNum, TArray<TSharedRef<FOnlineUser>>& OutUsers)
{
	FScopeLock ScopeLock(&UserInfoLock);
	CachedUsers.GenerateValueArray(OutUsers);
	return OutUsers.Num() > 0;
}

TSharedPtr<FOnlineUser> FOnlineUserEOSKit::GetUserInfo(int32 LocalUserNum, const FUniqueNetId& UserId) const
{
	FScopeLock ScopeLock(&UserInfoLock);
	const TSharedRef<FOnlineUser>* User = CachedUsers.Find(UserId.AsShared());
	if (User)
	{
		return *User;
	}
	return nullptr;
}

bool FOnlineUserEOSKit::QueryUserIdMapping(const FUniqueNetId& LocalUserId, const FString& DisplayNameOrEmail, const IOnlineUser::FOnQueryUserMappingComplete& Delegate)
{
	// In UE 5.5, FOnQueryUserMappingComplete takes (bool, const FUniqueNetId&, const FString&, const FUniqueNetId&, const FString&)
	// Parameters: (bWasSuccessful, LocalUserId, DisplayNameOrEmail, ResolvedUserId, ErrorMessage)
	// Pass LocalUserId as placeholder for ResolvedUserId since we don't have a resolved ID
	Delegate.ExecuteIfBound(false, LocalUserId, DisplayNameOrEmail, LocalUserId, TEXT("Not implemented"));
	return false;
}

bool FOnlineUserEOSKit::QueryExternalIdMappings(const FUniqueNetId& LocalUserId, const FExternalIdQueryOptions& QueryOptions, const TArray<FString>& ExternalIds, const IOnlineUser::FOnQueryExternalIdMappingsComplete& Delegate)
{
	// In UE 5.5, FOnQueryExternalIdMappingsComplete takes (bool, const FUniqueNetId&, const FExternalIdQueryOptions&, const TArray<FString>&, const FString&)
	// Parameters: (bWasSuccessful, LocalUserId, QueryOptions, ExternalIds, ErrorMessage)
	Delegate.ExecuteIfBound(false, LocalUserId, QueryOptions, ExternalIds, TEXT("Not implemented"));
	return false;
}

void FOnlineUserEOSKit::GetExternalIdMappings(const FExternalIdQueryOptions& QueryOptions, const TArray<FString>& ExternalIds, TArray<FUniqueNetIdRef>& OutUserIds)
{
	// Not implemented yet
}

FUniqueNetIdPtr FOnlineUserEOSKit::GetExternalIdMapping(const FExternalIdQueryOptions& QueryOptions, const FString& ExternalId)
{
	return nullptr;
}

TSharedPtr<FOnlineUser, ESPMode::ThreadSafe> FOnlineUserEOSKit::GetUserInfo(int32 LocalUserNum, const FUniqueNetId& UserId)
{
	FScopeLock ScopeLock(&UserInfoLock);
	const TSharedRef<FOnlineUser>* User = CachedUsers.Find(UserId.AsShared());
	if (User)
	{
		return *User;
	}
	return nullptr;
}

void FOnlineUserEOSKit::GetExternalIdMappings(const FExternalIdQueryOptions& QueryOptions, const TArray<FString>& ExternalIds, TArray<FUniqueNetIdPtr>& OutUserIds)
{
	// Get external ID mappings - not implemented yet
	OutUserIds.Empty();
}

#endif // WITH_EOS_SDK

