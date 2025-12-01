// Copyright (C) 2024, All Rights Reserved.

#include "OnlineIdentityEOSKit.h"
#include "OnlineSubsystemEOSKit.h"
#include "EOSKitSubsystem.h"
#include "EOSLoginAsync.h"
#include "Engine/GameInstance.h"
#include "Engine/World.h"
#include "OnlineSubsystemTypes.h"

#if WITH_EOS_SDK

FOnlineIdentityEOSKit::FOnlineIdentityEOSKit(FOnlineSubsystemEOSKit* InSubsystem)
	: EOSKitSubsystem(InSubsystem)
	, AuthHandle(nullptr)
	, ConnectHandle(nullptr)
{
	if (EOSKitSubsystem)
	{
		AuthHandle = EOSKitSubsystem->AuthHandle;
		ConnectHandle = EOSKitSubsystem->ConnectHandle;
	}
}

FOnlineIdentityEOSKit::~FOnlineIdentityEOSKit()
{
	// Cleanup
}

bool FOnlineIdentityEOSKit::Login(int32 LocalUserNum, const FOnlineAccountCredentials& AccountCredentials)
{
	// Delegate to EOSKit auth functionality
	// This is a minimal implementation - full implementation would use UEOSLoginAsync
	UE_LOG_ONLINE(Log, TEXT("FOnlineIdentityEOSKit::Login: LocalUserNum=%d, Type=%s"), LocalUserNum, *AccountCredentials.Type);
	
	// Trigger login complete delegate
	if (OnLoginCompleteDelegates[LocalUserNum].IsBound())
	{
		// In a full implementation, this would be called after async login completes
		// For now, we'll just log
	}
	
	return true;
}

bool FOnlineIdentityEOSKit::Logout(int32 LocalUserNum)
{
	UE_LOG_ONLINE(Log, TEXT("FOnlineIdentityEOSKit::Logout: LocalUserNum=%d"), LocalUserNum);
	
	// Remove user account
	FScopeLock ScopeLock(&AccountLock);
	FUniqueNetIdPtr UserId = GetUniquePlayerId(LocalUserNum);
	if (UserId.IsValid())
	{
		UserAccounts.Remove(UserId.ToSharedRef());
	}
	
	// Trigger logout complete delegate
	if (OnLogoutCompleteDelegates[LocalUserNum].IsBound())
	{
		OnLogoutCompleteDelegates[LocalUserNum].Broadcast(LocalUserNum, true);
	}
	
	return true;
}

bool FOnlineIdentityEOSKit::AutoLogin(int32 LocalUserNum)
{
	// Attempt automatic login
	return Login(LocalUserNum, FOnlineAccountCredentials(TEXT("persistentauth"), TEXT(""), TEXT("")));
}

TSharedPtr<FUserOnlineAccount> FOnlineIdentityEOSKit::GetUserAccount(const FUniqueNetId& UserId) const
{
	FScopeLock ScopeLock(&AccountLock);
	const TSharedRef<FUserOnlineAccount>* Account = UserAccounts.Find(UserId.AsShared());
	if (Account)
	{
		return *Account;
	}
	return nullptr;
}

TArray<TSharedPtr<FUserOnlineAccount>> FOnlineIdentityEOSKit::GetAllUserAccounts() const
{
	FScopeLock ScopeLock(&AccountLock);
	TArray<TSharedRef<FUserOnlineAccount, ESPMode::ThreadSafe>> Accounts;
	UserAccounts.GenerateValueArray(Accounts);
	// Convert TSharedRef to TSharedPtr for return
	TArray<TSharedPtr<FUserOnlineAccount>> Result;
	Result.Reserve(Accounts.Num());
	for (const TSharedRef<FUserOnlineAccount, ESPMode::ThreadSafe>& Account : Accounts)
	{
		Result.Add(Account);
	}
	return Result;
}

FUniqueNetIdPtr FOnlineIdentityEOSKit::GetUniquePlayerId(int32 LocalUserNum) const
{
	// Get ProductUserId from EOSKitSubsystem
	if (GEngine)
	{
		for (const FWorldContext& Context : GEngine->GetWorldContexts())
		{
			if (Context.World() && Context.World()->GetGameInstance())
			{
				UEOSKitSubsystem* EOSKitSubsystemPtr = Context.World()->GetGameInstance()->GetSubsystem<UEOSKitSubsystem>();
				if (EOSKitSubsystemPtr)
				{
					EOS_ProductUserId ProductUserId = EOSKitSubsystemPtr->GetProductUserId(LocalUserNum);
					if (ProductUserId)
					{
						// Create FUniqueNetId from ProductUserId
						// This is a simplified version - full implementation would create proper FUniqueNetIdEOS
						char ProductUserIdStr[EOS_PRODUCTUSERID_MAX_LENGTH + 1];
						int32_t ProductUserIdStrSize = sizeof(ProductUserIdStr);
						if (EOS_ProductUserId_ToString(ProductUserId, ProductUserIdStr, &ProductUserIdStrSize) == EOS_EResult::EOS_Success)
						{
							FString UserIdString = UTF8_TO_TCHAR(ProductUserIdStr);
							// Use CreateUniquePlayerId which is available in the base class
							// Cast away const to call non-const method
							return const_cast<FOnlineIdentityEOSKit*>(this)->CreateUniquePlayerId(UserIdString);
						}
					}
				}
			}
		}
	}
	return nullptr;
}

FUniqueNetIdPtr FOnlineIdentityEOSKit::CreateUniquePlayerId(uint8* Bytes, int32 Size)
{
	// Create from bytes - not implemented yet
	return nullptr;
}

FUniqueNetIdPtr FOnlineIdentityEOSKit::CreateUniquePlayerId(const FString& Str)
{
	// Create from string - in UE 5.5, FUniqueNetIdString constructor is protected
	// Use FUniqueNetIdString::Create with subsystem name parameter
	// The Create function signature in UE 5.5 is: Create(const FString& InUniqueNetIdStr, const FName& InType)
	if (!Str.IsEmpty())
	{
		return FUniqueNetIdString::Create(Str, FName(TEXT("EOS")));
	}
	return nullptr;
}

ELoginStatus::Type FOnlineIdentityEOSKit::GetLoginStatus(int32 LocalUserNum) const
{
	FUniqueNetIdPtr UserId = GetUniquePlayerId(LocalUserNum);
	if (UserId.IsValid())
	{
		return GetLoginStatus(*UserId);
	}
	return ELoginStatus::NotLoggedIn;
}

ELoginStatus::Type FOnlineIdentityEOSKit::GetLoginStatus(const FUniqueNetId& UserId) const
{
	FScopeLock ScopeLock(&AccountLock);
	const TSharedRef<FUserOnlineAccount>* Account = UserAccounts.Find(UserId.AsShared());
	if (Account)
	{
		return ELoginStatus::LoggedIn;
	}
	return ELoginStatus::NotLoggedIn;
}

FString FOnlineIdentityEOSKit::GetPlayerNickname(int32 LocalUserNum) const
{
	FUniqueNetIdPtr UserId = GetUniquePlayerId(LocalUserNum);
	if (UserId.IsValid())
	{
		return GetPlayerNickname(*UserId);
	}
	return FString();
}

FString FOnlineIdentityEOSKit::GetPlayerNickname(const FUniqueNetId& UserId) const
{
	TSharedPtr<FUserOnlineAccount> Account = GetUserAccount(UserId);
	if (Account.IsValid())
	{
		return Account->GetDisplayName();
	}
	return FString();
}

FString FOnlineIdentityEOSKit::GetAuthToken(int32 LocalUserNum) const
{
	// Get auth token - not fully implemented yet
	return FString();
}

void FOnlineIdentityEOSKit::GetUserPrivilege(const FUniqueNetId& UserId, EUserPrivileges::Type Privilege, const FOnGetUserPrivilegeCompleteDelegate& Delegate, EShowPrivilegeResolveUI ShowUI)
{
	// Privilege checking - not implemented yet
	Delegate.ExecuteIfBound(UserId, Privilege, (uint32)EUserPrivileges::CanPlay);
}

FPlatformUserId FOnlineIdentityEOSKit::GetPlatformUserIdFromUniqueNetId(const FUniqueNetId& UniqueNetId) const
{
	// Platform user ID mapping - not implemented yet
	return FPlatformUserId();
}

void FOnlineIdentityEOSKit::RevokeAuthToken(const FUniqueNetId& UserId, const FOnRevokeAuthTokenCompleteDelegate& Delegate)
{
	// Revoke auth token - not implemented yet
	// In UE 5.5, FOnRevokeAuthTokenCompleteDelegate takes (const FUniqueNetId&, const FOnlineError&)
	Delegate.ExecuteIfBound(UserId, FOnlineError(TEXT("Not implemented")));
}

#endif // WITH_EOS_SDK

