// Copyright (C) 2024, All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Interfaces/OnlineIdentityInterface.h"
#include "OnlineSubsystemEOSKit.h"

#if WITH_EOS_SDK
	#include "eos_auth.h"
	#include "eos_connect.h"
#endif

class FOnlineSubsystemEOSKit;

#if WITH_EOS_SDK

/**
 * Interface for EOS identity/authentication
 */
class ONLINESUBSYSTEMEOSKIT_API FOnlineIdentityEOSKit :
	public IOnlineIdentity
	, public TSharedFromThis<FOnlineIdentityEOSKit, ESPMode::ThreadSafe>
{
public:
	FOnlineIdentityEOSKit() = delete;
	explicit FOnlineIdentityEOSKit(FOnlineSubsystemEOSKit* InSubsystem);
	virtual ~FOnlineIdentityEOSKit();

	// IOnlineIdentity interface
	virtual bool Login(int32 LocalUserNum, const FOnlineAccountCredentials& AccountCredentials) override;
	virtual bool Logout(int32 LocalUserNum) override;
	virtual bool AutoLogin(int32 LocalUserNum) override;
	virtual TSharedPtr<FUserOnlineAccount> GetUserAccount(const FUniqueNetId& UserId) const override;
	virtual TArray<TSharedPtr<FUserOnlineAccount>> GetAllUserAccounts() const override;
	virtual FUniqueNetIdPtr GetUniquePlayerId(int32 LocalUserNum) const override;
	virtual FUniqueNetIdPtr CreateUniquePlayerId(uint8* Bytes, int32 Size) override;
	virtual FUniqueNetIdPtr CreateUniquePlayerId(const FString& Str) override;
	virtual ELoginStatus::Type GetLoginStatus(int32 LocalUserNum) const override;
	virtual ELoginStatus::Type GetLoginStatus(const FUniqueNetId& UserId) const override;
	virtual FString GetPlayerNickname(int32 LocalUserNum) const override;
	virtual FString GetPlayerNickname(const FUniqueNetId& UserId) const override;
	virtual FString GetAuthToken(int32 LocalUserNum) const override;
	virtual void GetUserPrivilege(const FUniqueNetId& UserId, EUserPrivileges::Type Privilege, const FOnGetUserPrivilegeCompleteDelegate& Delegate, EShowPrivilegeResolveUI ShowUI = EShowPrivilegeResolveUI::Default) override;
	virtual FPlatformUserId GetPlatformUserIdFromUniqueNetId(const FUniqueNetId& UniqueNetId) const override;
	virtual FString GetAuthType() const override { return TEXT("EOS"); }
	virtual void RevokeAuthToken(const FUniqueNetId& UserId, const FOnRevokeAuthTokenCompleteDelegate& Delegate) override;

private:
	FOnlineSubsystemEOSKit* EOSKitSubsystem;
	EOS_HAuth AuthHandle;
	EOS_HConnect ConnectHandle;
	
	/** Lock for user account access */
	mutable FCriticalSection AccountLock;
	
	/** Cached user accounts */
	TMap<FUniqueNetIdRef, TSharedRef<FUserOnlineAccount>> UserAccounts;
	
	/** Map of LocalUserNum to DisplayName for Device ID logins */
	TMap<int32, FString> LocalUserNumToDisplayName;

	/** Internal handler for login complete to populate UserAccounts */
	void OnLoginCompleteInternal(int32 LocalUserNum, bool bWasSuccessful, const FUniqueNetId& UserId, const FString& Error);
	
	/** Helper to create Device ID and retry login */
	void CreateDeviceIdAndRetryLogin(int32 LocalUserNum, const FString& DisplayName);
};

#endif // WITH_EOS_SDK

