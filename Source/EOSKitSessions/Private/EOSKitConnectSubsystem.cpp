// Copyright (C) 2024, All Rights Reserved.

#include "EOSKitConnectSubsystem.h"
#include "EOSKitSubsystem.h"
#include "eos_connect.h"
#include "eos_connect_types.h"
#include "Async/Async.h"

void UEOSKitConnectSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	
	UE_LOG(LogTemp, Log, TEXT("EOSKit: Connect Subsystem Initialized"));
}

void UEOSKitConnectSubsystem::Deinitialize()
{
	// Cleanup all notifications
	UnregisterAllNotifications();
	
	UE_LOG(LogTemp, Log, TEXT("EOSKit: Connect Subsystem Deinitialized"));
	
	Super::Deinitialize();
}

// ========================================
// Notification Management
// ========================================

bool UEOSKitConnectSubsystem::RegisterAuthExpirationNotification(const FEOSKitOnAuthExpiration& Callback)
{
	UEOSKitSubsystem* EOSKitSubsystem = GetEOSKitSubsystem();
	if (!EOSKitSubsystem || !EOSKitSubsystem->GetPlatformHandle())
	{
		UE_LOG(LogTemp, Error, TEXT("EOSKit Connect: Cannot register notification - Platform Handle is null"));
		return false;
	}

	EOS_HPlatform PlatformHandle = EOSKitSubsystem->GetPlatformHandle();
	EOS_HConnect ConnectHandle = EOS_Platform_GetConnectInterface(PlatformHandle);
	
	if (!ConnectHandle)
	{
		UE_LOG(LogTemp, Error, TEXT("EOSKit Connect: Failed to get Connect Handle"));
		return false;
	}

	if (AuthExpirationNotificationId != 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("EOSKit Connect: Auth expiration notification already registered"));
		return true;
	}

	OnAuthExpirationDelegate = Callback;

	EOS_Connect_AddNotifyAuthExpirationOptions Options = {};
	Options.ApiVersion = EOS_CONNECT_ADDNOTIFYAUTHEXPIRATION_API_LATEST;
	
	AuthExpirationNotificationId = EOS_Connect_AddNotifyAuthExpiration(
		ConnectHandle,
		&Options,
		this,
		[](const EOS_Connect_AuthExpirationCallbackInfo* Data)
		{
			UEOSKitConnectSubsystem::OnAuthExpirationCallback(Data);
		}
	);
	
	UE_LOG(LogTemp, Log, TEXT("EOSKit Connect: Registered for auth expiration notifications"));
	return AuthExpirationNotificationId != 0;
}

bool UEOSKitConnectSubsystem::RegisterLoginStatusChangedNotification(const FEOSKitOnLoginStatusChanged& Callback)
{
	UEOSKitSubsystem* EOSKitSubsystem = GetEOSKitSubsystem();
	if (!EOSKitSubsystem || !EOSKitSubsystem->GetPlatformHandle())
	{
		return false;
	}

	EOS_HPlatform PlatformHandle = EOSKitSubsystem->GetPlatformHandle();
	EOS_HConnect ConnectHandle = EOS_Platform_GetConnectInterface(PlatformHandle);
	
	if (!ConnectHandle)
	{
		return false;
	}

	if (LoginStatusChangedNotificationId != 0)
	{
		return true;
	}

	OnLoginStatusChangedDelegate = Callback;

	EOS_Connect_AddNotifyLoginStatusChangedOptions Options = {};
	Options.ApiVersion = EOS_CONNECT_ADDNOTIFYLOGINSTATUSCHANGED_API_LATEST;
	
	LoginStatusChangedNotificationId = EOS_Connect_AddNotifyLoginStatusChanged(
		ConnectHandle,
		&Options,
		this,
		[](const EOS_Connect_LoginStatusChangedCallbackInfo* Data)
		{
			UEOSKitConnectSubsystem::OnLoginStatusChangedCallback(Data);
		}
	);
	
	UE_LOG(LogTemp, Log, TEXT("EOSKit Connect: Registered for login status change notifications"));
	return LoginStatusChangedNotificationId != 0;
}

void UEOSKitConnectSubsystem::UnregisterAllNotifications()
{
	UEOSKitSubsystem* EOSKitSubsystem = GetEOSKitSubsystem();
	if (!EOSKitSubsystem || !EOSKitSubsystem->GetPlatformHandle())
	{
		return;
	}

	EOS_HPlatform PlatformHandle = EOSKitSubsystem->GetPlatformHandle();
	EOS_HConnect ConnectHandle = EOS_Platform_GetConnectInterface(PlatformHandle);
	
	if (!ConnectHandle)
	{
		return;
	}

	if (AuthExpirationNotificationId != 0)
	{
		EOS_Connect_RemoveNotifyAuthExpiration(ConnectHandle, AuthExpirationNotificationId);
		AuthExpirationNotificationId = 0;
	}

	if (LoginStatusChangedNotificationId != 0)
	{
		EOS_Connect_RemoveNotifyLoginStatusChanged(ConnectHandle, LoginStatusChangedNotificationId);
		LoginStatusChangedNotificationId = 0;
	}

	UE_LOG(LogTemp, Log, TEXT("EOSKit Connect: Unregistered all notifications"));
}

// ========================================
// ID Token Management
// ========================================

bool UEOSKitConnectSubsystem::CopyIdToken(const FString& ProductUserId, FEOSKitConnectIdToken& OutIdToken)
{
	UEOSKitSubsystem* EOSKitSubsystem = GetEOSKitSubsystem();
	if (!EOSKitSubsystem || !EOSKitSubsystem->GetPlatformHandle())
	{
		return false;
	}

	EOS_HPlatform PlatformHandle = EOSKitSubsystem->GetPlatformHandle();
	EOS_HConnect ConnectHandle = EOS_Platform_GetConnectInterface(PlatformHandle);
	
	if (!ConnectHandle)
	{
		return false;
	}

	EOS_ProductUserId UserId = EOS_ProductUserId_FromString(TCHAR_TO_UTF8(*ProductUserId));
	if (!UserId || !EOS_ProductUserId_IsValid(UserId))
	{
		return false;
	}

	EOS_Connect_CopyIdTokenOptions Options = {};
	Options.ApiVersion = EOS_CONNECT_COPYIDTOKEN_API_LATEST;
	Options.LocalUserId = UserId;
	
	EOS_Connect_IdToken* IdToken = nullptr;
	EOS_EResult Result = EOS_Connect_CopyIdToken(ConnectHandle, &Options, &IdToken);
	
	if (Result == EOS_EResult::EOS_Success && IdToken)
	{
		if (IdToken->JsonWebToken)
		{
			OutIdToken.JsonWebToken = UTF8_TO_TCHAR(IdToken->JsonWebToken);
		}
		EOS_Connect_IdToken_Release(IdToken);
		return true;
	}
	
	return false;
}

// ========================================
// External Account Management
// ========================================

bool UEOSKitConnectSubsystem::CopyProductUserExternalAccountByAccountId(
	const FString& ProductUserId,
	const FString& AccountId,
	FEOSKitExternalAccountInfo& OutAccountInfo)
{
	UEOSKitSubsystem* EOSKitSubsystem = GetEOSKitSubsystem();
	if (!EOSKitSubsystem || !EOSKitSubsystem->GetPlatformHandle())
	{
		return false;
	}

	EOS_HPlatform PlatformHandle = EOSKitSubsystem->GetPlatformHandle();
	EOS_HConnect ConnectHandle = EOS_Platform_GetConnectInterface(PlatformHandle);
	
	if (!ConnectHandle)
	{
		return false;
	}

	EOS_ProductUserId UserId = EOS_ProductUserId_FromString(TCHAR_TO_UTF8(*ProductUserId));
	if (!UserId || !EOS_ProductUserId_IsValid(UserId))
	{
		return false;
	}

	EOS_Connect_CopyProductUserExternalAccountByAccountIdOptions Options = {};
	Options.ApiVersion = EOS_CONNECT_COPYPRODUCTUSEREXTERNALACCOUNTBYACCOUNTID_API_LATEST;
	Options.TargetUserId = UserId;
	Options.AccountId = TCHAR_TO_UTF8(*AccountId);
	
	EOS_Connect_ExternalAccountInfo* AccountInfo = nullptr;
	EOS_EResult Result = EOS_Connect_CopyProductUserExternalAccountByAccountId(ConnectHandle, &Options, &AccountInfo);
	
	if (Result == EOS_EResult::EOS_Success && AccountInfo)
	{
		// Parse account info
		if (AccountInfo->ProductUserId)
		{
			char UserIdStr[EOS_PRODUCTUSERID_MAX_LENGTH + 1];
			int32_t BufferSize = sizeof(UserIdStr);
			EOS_ProductUserId_ToString(AccountInfo->ProductUserId, UserIdStr, &BufferSize);
			OutAccountInfo.ProductUserId = UTF8_TO_TCHAR(UserIdStr);
		}
		
		if (AccountInfo->DisplayName)
		{
			OutAccountInfo.DisplayName = UTF8_TO_TCHAR(AccountInfo->DisplayName);
		}
		
		if (AccountInfo->AccountId)
		{
			OutAccountInfo.AccountId = UTF8_TO_TCHAR(AccountInfo->AccountId);
		}
		
		OutAccountInfo.AccountIdType = static_cast<EEOSKitExternalAccountType>(AccountInfo->AccountIdType);
		
		EOS_Connect_ExternalAccountInfo_Release(AccountInfo);
		return true;
	}
	
	return false;
}

bool UEOSKitConnectSubsystem::CopyProductUserExternalAccountByAccountType(
	const FString& ProductUserId,
	TEnumAsByte<EEOSKitExternalAccountType> AccountType,
	FEOSKitExternalAccountInfo& OutAccountInfo)
{
	UEOSKitSubsystem* EOSKitSubsystem = GetEOSKitSubsystem();
	if (!EOSKitSubsystem || !EOSKitSubsystem->GetPlatformHandle())
	{
		return false;
	}

	EOS_HPlatform PlatformHandle = EOSKitSubsystem->GetPlatformHandle();
	EOS_HConnect ConnectHandle = EOS_Platform_GetConnectInterface(PlatformHandle);
	
	if (!ConnectHandle)
	{
		return false;
	}

	EOS_ProductUserId UserId = EOS_ProductUserId_FromString(TCHAR_TO_UTF8(*ProductUserId));
	if (!UserId || !EOS_ProductUserId_IsValid(UserId))
	{
		return false;
	}

	EOS_Connect_CopyProductUserExternalAccountByAccountTypeOptions Options = {};
	Options.ApiVersion = EOS_CONNECT_COPYPRODUCTUSEREXTERNALACCOUNTBYACCOUNTTYPE_API_LATEST;
	Options.TargetUserId = UserId;
	Options.AccountIdType = static_cast<EOS_EExternalAccountType>(AccountType.GetValue());
	
	EOS_Connect_ExternalAccountInfo* AccountInfo = nullptr;
	EOS_EResult Result = EOS_Connect_CopyProductUserExternalAccountByAccountType(ConnectHandle, &Options, &AccountInfo);
	
	if (Result == EOS_EResult::EOS_Success && AccountInfo)
	{
		// Parse similar to above
		if (AccountInfo->ProductUserId)
		{
			char UserIdStr[EOS_PRODUCTUSERID_MAX_LENGTH + 1];
			int32_t BufferSize = sizeof(UserIdStr);
			EOS_ProductUserId_ToString(AccountInfo->ProductUserId, UserIdStr, &BufferSize);
			OutAccountInfo.ProductUserId = UTF8_TO_TCHAR(UserIdStr);
		}
		
		if (AccountInfo->DisplayName)
		{
			OutAccountInfo.DisplayName = UTF8_TO_TCHAR(AccountInfo->DisplayName);
		}
		
		if (AccountInfo->AccountId)
		{
			OutAccountInfo.AccountId = UTF8_TO_TCHAR(AccountInfo->AccountId);
		}
		
		OutAccountInfo.AccountIdType = static_cast<EEOSKitExternalAccountType>(AccountInfo->AccountIdType);
		
		EOS_Connect_ExternalAccountInfo_Release(AccountInfo);
		return true;
	}
	
	return false;
}

bool UEOSKitConnectSubsystem::CopyProductUserExternalAccountByIndex(
	const FString& ProductUserId,
	int32 Index,
	FEOSKitExternalAccountInfo& OutAccountInfo)
{
	UEOSKitSubsystem* EOSKitSubsystem = GetEOSKitSubsystem();
	if (!EOSKitSubsystem || !EOSKitSubsystem->GetPlatformHandle())
	{
		return false;
	}

	EOS_HPlatform PlatformHandle = EOSKitSubsystem->GetPlatformHandle();
	EOS_HConnect ConnectHandle = EOS_Platform_GetConnectInterface(PlatformHandle);
	
	if (!ConnectHandle)
	{
		return false;
	}

	EOS_ProductUserId UserId = EOS_ProductUserId_FromString(TCHAR_TO_UTF8(*ProductUserId));
	if (!UserId || !EOS_ProductUserId_IsValid(UserId))
	{
		return false;
	}

	EOS_Connect_CopyProductUserExternalAccountByIndexOptions Options = {};
	Options.ApiVersion = EOS_CONNECT_COPYPRODUCTUSEREXTERNALACCOUNTBYINDEX_API_LATEST;
	Options.TargetUserId = UserId;
	Options.ExternalAccountInfoIndex = Index;
	
	EOS_Connect_ExternalAccountInfo* AccountInfo = nullptr;
	EOS_EResult Result = EOS_Connect_CopyProductUserExternalAccountByIndex(ConnectHandle, &Options, &AccountInfo);
	
	if (Result == EOS_EResult::EOS_Success && AccountInfo)
	{
		// Parse similar to above
		if (AccountInfo->ProductUserId)
		{
			char UserIdStr[EOS_PRODUCTUSERID_MAX_LENGTH + 1];
			int32_t BufferSize = sizeof(UserIdStr);
			EOS_ProductUserId_ToString(AccountInfo->ProductUserId, UserIdStr, &BufferSize);
			OutAccountInfo.ProductUserId = UTF8_TO_TCHAR(UserIdStr);
		}
		
		if (AccountInfo->DisplayName)
		{
			OutAccountInfo.DisplayName = UTF8_TO_TCHAR(AccountInfo->DisplayName);
		}
		
		if (AccountInfo->AccountId)
		{
			OutAccountInfo.AccountId = UTF8_TO_TCHAR(AccountInfo->AccountId);
		}
		
		OutAccountInfo.AccountIdType = static_cast<EEOSKitExternalAccountType>(AccountInfo->AccountIdType);
		
		EOS_Connect_ExternalAccountInfo_Release(AccountInfo);
		return true;
	}
	
	return false;
}

bool UEOSKitConnectSubsystem::CopyProductUserInfo(
	const FString& ProductUserId,
	FEOSKitExternalAccountInfo& OutAccountInfo)
{
	UEOSKitSubsystem* EOSKitSubsystem = GetEOSKitSubsystem();
	if (!EOSKitSubsystem || !EOSKitSubsystem->GetPlatformHandle())
	{
		return false;
	}

	EOS_HPlatform PlatformHandle = EOSKitSubsystem->GetPlatformHandle();
	EOS_HConnect ConnectHandle = EOS_Platform_GetConnectInterface(PlatformHandle);
	
	if (!ConnectHandle)
	{
		return false;
	}

	EOS_ProductUserId UserId = EOS_ProductUserId_FromString(TCHAR_TO_UTF8(*ProductUserId));
	if (!UserId || !EOS_ProductUserId_IsValid(UserId))
	{
		return false;
	}

	EOS_Connect_CopyProductUserInfoOptions Options = {};
	Options.ApiVersion = EOS_CONNECT_COPYPRODUCTUSERINFO_API_LATEST;
	Options.TargetUserId = UserId;
	
	EOS_Connect_ExternalAccountInfo* AccountInfo = nullptr;
	EOS_EResult Result = EOS_Connect_CopyProductUserInfo(ConnectHandle, &Options, &AccountInfo);
	
	if (Result == EOS_EResult::EOS_Success && AccountInfo)
	{
		// Parse similar to above
		if (AccountInfo->ProductUserId)
		{
			char UserIdStr[EOS_PRODUCTUSERID_MAX_LENGTH + 1];
			int32_t BufferSize = sizeof(UserIdStr);
			EOS_ProductUserId_ToString(AccountInfo->ProductUserId, UserIdStr, &BufferSize);
			OutAccountInfo.ProductUserId = UTF8_TO_TCHAR(UserIdStr);
		}
		
		if (AccountInfo->DisplayName)
		{
			OutAccountInfo.DisplayName = UTF8_TO_TCHAR(AccountInfo->DisplayName);
		}
		
		if (AccountInfo->AccountId)
		{
			OutAccountInfo.AccountId = UTF8_TO_TCHAR(AccountInfo->AccountId);
		}
		
		OutAccountInfo.AccountIdType = static_cast<EEOSKitExternalAccountType>(AccountInfo->AccountIdType);
		
		EOS_Connect_ExternalAccountInfo_Release(AccountInfo);
		return true;
	}
	
	return false;
}

// ========================================
// Account Mapping
// ========================================

FString UEOSKitConnectSubsystem::GetExternalAccountMapping(
	const FString& LocalUserId,
	TEnumAsByte<EEOSKitExternalAccountType> AccountType,
	const FString& TargetExternalUserId)
{
	UEOSKitSubsystem* EOSKitSubsystem = GetEOSKitSubsystem();
	if (!EOSKitSubsystem || !EOSKitSubsystem->GetPlatformHandle())
	{
		return FString();
	}

	EOS_HPlatform PlatformHandle = EOSKitSubsystem->GetPlatformHandle();
	EOS_HConnect ConnectHandle = EOS_Platform_GetConnectInterface(PlatformHandle);
	
	if (!ConnectHandle)
	{
		return FString();
	}

	EOS_ProductUserId UserId = EOS_ProductUserId_FromString(TCHAR_TO_UTF8(*LocalUserId));
	if (!UserId || !EOS_ProductUserId_IsValid(UserId))
	{
		return FString();
	}

	EOS_Connect_GetExternalAccountMappingsOptions Options = {};
	Options.ApiVersion = EOS_CONNECT_GETEXTERNALACCOUNTMAPPING_API_LATEST;
	Options.LocalUserId = UserId;
	Options.AccountIdType = static_cast<EOS_EExternalAccountType>(AccountType.GetValue());
	
	if (!TargetExternalUserId.IsEmpty())
	{
		Options.TargetExternalUserId = TCHAR_TO_UTF8(*TargetExternalUserId);
	}
	
	EOS_ProductUserId MappedUserId = EOS_Connect_GetExternalAccountMapping(ConnectHandle, &Options);
	
	if (MappedUserId && EOS_ProductUserId_IsValid(MappedUserId))
	{
		char UserIdStr[EOS_PRODUCTUSERID_MAX_LENGTH + 1];
		int32_t BufferSize = sizeof(UserIdStr);
		EOS_ProductUserId_ToString(MappedUserId, UserIdStr, &BufferSize);
		return UTF8_TO_TCHAR(UserIdStr);
	}
	
	return FString();
}

bool UEOSKitConnectSubsystem::GetProductUserIdMapping(
	const FString& LocalUserId,
	TEnumAsByte<EEOSKitExternalAccountType> AccountType,
	const FString& TargetUserId,
	FString& OutExternalAccountId)
{
	UEOSKitSubsystem* EOSKitSubsystem = GetEOSKitSubsystem();
	if (!EOSKitSubsystem || !EOSKitSubsystem->GetPlatformHandle())
	{
		return false;
	}

	EOS_HPlatform PlatformHandle = EOSKitSubsystem->GetPlatformHandle();
	EOS_HConnect ConnectHandle = EOS_Platform_GetConnectInterface(PlatformHandle);
	
	if (!ConnectHandle)
	{
		return false;
	}

	EOS_ProductUserId LocalProductUserId = EOS_ProductUserId_FromString(TCHAR_TO_UTF8(*LocalUserId));
	EOS_ProductUserId TargetProductUserId = EOS_ProductUserId_FromString(TCHAR_TO_UTF8(*TargetUserId));
	
	if (!LocalProductUserId || !TargetProductUserId)
	{
		return false;
	}

	EOS_Connect_GetProductUserIdMappingOptions Options = {};
	Options.ApiVersion = EOS_CONNECT_GETPRODUCTUSERIDMAPPING_API_LATEST;
	Options.LocalUserId = LocalProductUserId;
	Options.AccountIdType = static_cast<EOS_EExternalAccountType>(AccountType.GetValue());
	Options.TargetProductUserId = TargetProductUserId;
	
	char Buffer[EOS_CONNECT_EXTERNAL_ACCOUNT_ID_MAX_LENGTH + 1];
	int32_t BufferSize = sizeof(Buffer);
	
	EOS_EResult Result = EOS_Connect_GetProductUserIdMapping(ConnectHandle, &Options, Buffer, &BufferSize);
	
	if (Result == EOS_EResult::EOS_Success)
	{
		OutExternalAccountId = UTF8_TO_TCHAR(Buffer);
		return true;
	}
	
	return false;
}

// ========================================
// User Status Queries
// ========================================

int32 UEOSKitConnectSubsystem::GetLoggedInUsersCount() const
{
	UEOSKitSubsystem* EOSKitSubsystem = GetEOSKitSubsystem();
	if (!EOSKitSubsystem || !EOSKitSubsystem->GetPlatformHandle())
	{
		return 0;
	}

	EOS_HPlatform PlatformHandle = EOSKitSubsystem->GetPlatformHandle();
	EOS_HConnect ConnectHandle = EOS_Platform_GetConnectInterface(PlatformHandle);
	
	if (!ConnectHandle)
	{
		return 0;
	}

	return EOS_Connect_GetLoggedInUsersCount(ConnectHandle);
}

FString UEOSKitConnectSubsystem::GetLoggedInUserByIndex(int32 Index) const
{
	UEOSKitSubsystem* EOSKitSubsystem = GetEOSKitSubsystem();
	if (!EOSKitSubsystem || !EOSKitSubsystem->GetPlatformHandle())
	{
		return FString();
	}

	EOS_HPlatform PlatformHandle = EOSKitSubsystem->GetPlatformHandle();
	EOS_HConnect ConnectHandle = EOS_Platform_GetConnectInterface(PlatformHandle);
	
	if (!ConnectHandle)
	{
		return FString();
	}

	EOS_ProductUserId UserId = EOS_Connect_GetLoggedInUserByIndex(ConnectHandle, Index);
	
	if (UserId && EOS_ProductUserId_IsValid(UserId))
	{
		char UserIdStr[EOS_PRODUCTUSERID_MAX_LENGTH + 1];
		int32_t BufferSize = sizeof(UserIdStr);
		EOS_ProductUserId_ToString(UserId, UserIdStr, &BufferSize);
		return UTF8_TO_TCHAR(UserIdStr);
	}
	
	return FString();
}

TEnumAsByte<EEOSKitLoginStatus> UEOSKitConnectSubsystem::GetLoginStatus(const FString& ProductUserId) const
{
	UEOSKitSubsystem* EOSKitSubsystem = GetEOSKitSubsystem();
	if (!EOSKitSubsystem || !EOSKitSubsystem->GetPlatformHandle())
	{
		return EKLS_NotLoggedIn;
	}

	EOS_HPlatform PlatformHandle = EOSKitSubsystem->GetPlatformHandle();
	EOS_HConnect ConnectHandle = EOS_Platform_GetConnectInterface(PlatformHandle);
	
	if (!ConnectHandle)
	{
		return EKLS_NotLoggedIn;
	}

	EOS_ProductUserId UserId = EOS_ProductUserId_FromString(TCHAR_TO_UTF8(*ProductUserId));
	if (!UserId || !EOS_ProductUserId_IsValid(UserId))
	{
		return EKLS_NotLoggedIn;
	}

	EOS_ELoginStatus Status = EOS_Connect_GetLoginStatus(ConnectHandle, UserId);
	return static_cast<EEOSKitLoginStatus>(Status);
}

int32 UEOSKitConnectSubsystem::GetProductUserExternalAccountCount(const FString& ProductUserId) const
{
	UEOSKitSubsystem* EOSKitSubsystem = GetEOSKitSubsystem();
	if (!EOSKitSubsystem || !EOSKitSubsystem->GetPlatformHandle())
	{
		return 0;
	}

	EOS_HPlatform PlatformHandle = EOSKitSubsystem->GetPlatformHandle();
	EOS_HConnect ConnectHandle = EOS_Platform_GetConnectInterface(PlatformHandle);
	
	if (!ConnectHandle)
	{
		return 0;
	}

	EOS_ProductUserId UserId = EOS_ProductUserId_FromString(TCHAR_TO_UTF8(*ProductUserId));
	if (!UserId || !EOS_ProductUserId_IsValid(UserId))
	{
		return 0;
	}

	EOS_Connect_GetProductUserExternalAccountCountOptions Options = {};
	Options.ApiVersion = EOS_CONNECT_GETPRODUCTUSEREXTERNALACCOUNTCOUNT_API_LATEST;
	Options.TargetUserId = UserId;
	
	return EOS_Connect_GetProductUserExternalAccountCount(ConnectHandle, &Options);
}

// ========================================
// Private Helper Functions
// ========================================

UEOSKitSubsystem* UEOSKitConnectSubsystem::GetEOSKitSubsystem() const
{
	UGameInstance* GameInstance = GetGameInstance();
	if (!GameInstance)
	{
		return nullptr;
	}

	return GameInstance->GetSubsystem<UEOSKitSubsystem>();
}

// ========================================
// Static Callback Functions
// ========================================

void UEOSKitConnectSubsystem::OnAuthExpirationCallback(const void* Data)
{
	const EOS_Connect_AuthExpirationCallbackInfo* CallbackInfo = static_cast<const EOS_Connect_AuthExpirationCallbackInfo*>(Data);
	
	if (!CallbackInfo || !CallbackInfo->ClientData)
	{
		return;
	}

	UEOSKitConnectSubsystem* Subsystem = static_cast<UEOSKitConnectSubsystem*>(CallbackInfo->ClientData);
	
	char UserIdStr[EOS_PRODUCTUSERID_MAX_LENGTH + 1];
	int32_t BufferSize = sizeof(UserIdStr);
	EOS_ProductUserId_ToString(CallbackInfo->LocalUserId, UserIdStr, &BufferSize);
	FString ProductUserId = UTF8_TO_TCHAR(UserIdStr);
	
	AsyncTask(ENamedThreads::GameThread, [Subsystem, ProductUserId]()
	{
		if (Subsystem)
		{
			UE_LOG(LogTemp, Warning, TEXT("EOSKit Connect: Auth expiring soon for user: %s"), *ProductUserId);
			UE_LOG(LogTemp, Warning, TEXT("EOSKit Connect: Re-login required within ~10 minutes"));
			Subsystem->OnAuthExpirationDelegate.ExecuteIfBound(ProductUserId);
		}
	});
}

void UEOSKitConnectSubsystem::OnLoginStatusChangedCallback(const void* Data)
{
	const EOS_Connect_LoginStatusChangedCallbackInfo* CallbackInfo = static_cast<const EOS_Connect_LoginStatusChangedCallbackInfo*>(Data);
	
	if (!CallbackInfo || !CallbackInfo->ClientData)
	{
		return;
	}

	UEOSKitConnectSubsystem* Subsystem = static_cast<UEOSKitConnectSubsystem*>(CallbackInfo->ClientData);
	
	char UserIdStr[EOS_PRODUCTUSERID_MAX_LENGTH + 1];
	int32_t BufferSize = sizeof(UserIdStr);
	EOS_ProductUserId_ToString(CallbackInfo->LocalUserId, UserIdStr, &BufferSize);
	FString ProductUserId = UTF8_TO_TCHAR(UserIdStr);
	int32 LoginStatus = static_cast<int32>(CallbackInfo->CurrentStatus);
	
	AsyncTask(ENamedThreads::GameThread, [Subsystem, ProductUserId, LoginStatus]()
	{
		if (Subsystem)
		{
			UE_LOG(LogTemp, Log, TEXT("EOSKit Connect: Login status changed for user: %s, Status: %d"), *ProductUserId, LoginStatus);
			Subsystem->OnLoginStatusChangedDelegate.ExecuteIfBound(ProductUserId, LoginStatus);
		}
	});
}
