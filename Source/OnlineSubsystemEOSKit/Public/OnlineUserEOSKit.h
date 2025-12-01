// Copyright (C) 2024, All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Interfaces/OnlineUserInterface.h"
#include "OnlineSubsystemEOSKit.h"

#if WITH_EOS_SDK
	#include "eos_userinfo.h"
#endif

class FOnlineSubsystemEOSKit;

#if WITH_EOS_SDK

/**
 * Interface for EOS user info
 */
class ONLINESUBSYSTEMEOSKIT_API FOnlineUserEOSKit :
	public IOnlineUser
	, public TSharedFromThis<FOnlineUserEOSKit, ESPMode::ThreadSafe>
{
public:
	FOnlineUserEOSKit() = delete;
	explicit FOnlineUserEOSKit(FOnlineSubsystemEOSKit* InSubsystem);
	virtual ~FOnlineUserEOSKit();

	// IOnlineUser interface
	virtual bool QueryUserInfo(int32 LocalUserNum, const TArray<FUniqueNetIdRef>& UserIds) override;
	virtual bool GetAllUserInfo(int32 LocalUserNum, TArray<TSharedRef<FOnlineUser>>& OutUsers) override;
	virtual TSharedPtr<FOnlineUser, ESPMode::ThreadSafe> GetUserInfo(int32 LocalUserNum, const FUniqueNetId& UserId) override;
	virtual void GetExternalIdMappings(const FExternalIdQueryOptions& QueryOptions, const TArray<FString>& ExternalIds, TArray<FUniqueNetIdPtr>& OutUserIds) override;
	// These methods were removed from IOnlineUser in UE 5.5, but kept for backward compatibility
	TSharedPtr<FOnlineUser> GetUserInfo(int32 LocalUserNum, const FUniqueNetId& UserId) const;
	virtual bool QueryUserIdMapping(const FUniqueNetId& LocalUserId, const FString& DisplayNameOrEmail, const IOnlineUser::FOnQueryUserMappingComplete& Delegate = IOnlineUser::FOnQueryUserMappingComplete());
	virtual bool QueryExternalIdMappings(const FUniqueNetId& LocalUserId, const FExternalIdQueryOptions& QueryOptions, const TArray<FString>& ExternalIds, const IOnlineUser::FOnQueryExternalIdMappingsComplete& Delegate = IOnlineUser::FOnQueryExternalIdMappingsComplete());
	virtual void GetExternalIdMappings(const FExternalIdQueryOptions& QueryOptions, const TArray<FString>& ExternalIds, TArray<FUniqueNetIdRef>& OutUserIds);
	virtual FUniqueNetIdPtr GetExternalIdMapping(const FExternalIdQueryOptions& QueryOptions, const FString& ExternalId) override;

private:
	FOnlineSubsystemEOSKit* EOSKitSubsystem;
	EOS_HUserInfo UserInfoHandle;
	
	mutable FCriticalSection UserInfoLock;
	TMap<FUniqueNetIdRef, TSharedRef<FOnlineUser>> CachedUsers;
};

#endif // WITH_EOS_SDK

