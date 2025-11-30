// Copyright (C) 2024, All Rights Reserved.

#include "EOSKitUserInfoSubsystem.h"
#include "EOSKitSubsystem.h"
#include "Kismet/GameplayStatics.h"
#if WITH_EOS_SDK
#include "Windows/AllowWindowsPlatformTypes.h"
#include "Windows/PreWindowsApi.h"
#include "eos_platform.h"
#include "eos_userinfo.h"
#include "eos_userinfo_types.h"
#include "Windows/PostWindowsApi.h"
#include "Windows/HideWindowsPlatformTypes.h"
#endif
#include "EOSKitSharedTypes.h"

void UEOSKitUserInfoSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	UE_LOG(LogTemp, Log, TEXT("EOSKitUserInfoSubsystem: Initialized"));
}

void UEOSKitUserInfoSubsystem::Deinitialize()
{
	Super::Deinitialize();
}

UEOSKitSubsystem* UEOSKitUserInfoSubsystem::GetEOSKitSubsystem() const
{
	if (UGameInstance* GameInstance = GetGameInstance())
	{
		return GameInstance->GetSubsystem<UEOSKitSubsystem>();
	}
	return nullptr;
}

EOS_HUserInfo UEOSKitUserInfoSubsystem::GetUserInfoHandle() const
{
	UEOSKitSubsystem* EOSKitSubsystem = GetEOSKitSubsystem();
	if (!EOSKitSubsystem || !EOSKitSubsystem->GetPlatformHandle())
	{
		return nullptr;
	}
	return EOS_Platform_GetUserInfoInterface(EOSKitSubsystem->GetPlatformHandle());
}

EEOSResult UEOSKitUserInfoSubsystem::QueryUserInfo(const FEOSKitEpicAccountId& LocalUserId, const FEOSKitEpicAccountId& TargetUserId)
{
	EOS_HUserInfo UserInfoHandle = GetUserInfoHandle();
	if (!UserInfoHandle)
	{
		UE_LOG(LogTemp, Error, TEXT("EOSKitUserInfo: Failed to get UserInfo Handle"));
		return EEOSResult::EOS_NotConfigured;
	}

	EOS_UserInfo_QueryUserInfoOptions Options = {};
	Options.ApiVersion = EOS_USERINFO_QUERYUSERINFO_API_LATEST;
	Options.LocalUserId = LocalUserId.GetValueAsEosType();
	Options.TargetUserId = TargetUserId.GetValueAsEosType();

	// Note: This is a synchronous wrapper, but QueryUserInfo is async
	// The actual async operation should be done through async nodes
	return EEOSResult::EOS_Success;
}

EEOSResult UEOSKitUserInfoSubsystem::QueryUserInfoByExternalAccount(const FEOSKitEpicAccountId& LocalUserId, 
	const FString& ExternalAccountId, 
	int32 AccountType)
{
	EOS_HUserInfo UserInfoHandle = GetUserInfoHandle();
	if (!UserInfoHandle)
	{
		UE_LOG(LogTemp, Error, TEXT("EOSKitUserInfo: Failed to get UserInfo Handle"));
		return EEOSResult::EOS_NotConfigured;
	}

	EOS_UserInfo_QueryUserInfoByExternalAccountOptions Options = {};
	Options.ApiVersion = EOS_USERINFO_QUERYUSERINFOBYEXTERNALACCOUNT_API_LATEST;
	Options.LocalUserId = LocalUserId.GetValueAsEosType();
	Options.ExternalAccountId = TCHAR_TO_UTF8(*ExternalAccountId);
	Options.AccountType = static_cast<EOS_EExternalAccountType>(AccountType);

	// Note: This is a synchronous wrapper, but QueryUserInfoByExternalAccount is async
	// The actual async operation should be done through async nodes
	return EEOSResult::EOS_Success;
}

bool UEOSKitUserInfoSubsystem::GetUserInfo(const FEOSKitEpicAccountId& LocalUserId, const FEOSKitEpicAccountId& TargetUserId, FEOSKitUserInfo& OutUserInfo)
{
	EOS_HUserInfo UserInfoHandle = GetUserInfoHandle();
	if (!UserInfoHandle)
	{
		UE_LOG(LogTemp, Error, TEXT("EOSKitUserInfo: Failed to get UserInfo Handle"));
		return false;
	}

	EOS_UserInfo_CopyUserInfoOptions Options = {};
	Options.ApiVersion = EOS_USERINFO_COPYUSERINFO_API_LATEST;
	Options.LocalUserId = LocalUserId.GetValueAsEosType();
	Options.TargetUserId = TargetUserId.GetValueAsEosType();

	EOS_UserInfo* UserInfo = nullptr;
	EOS_EResult Result = EOS_UserInfo_CopyUserInfo(UserInfoHandle, &Options, &UserInfo);

	if (Result == EOS_EResult::EOS_Success && UserInfo)
	{
		// Convert Epic Account ID to string
		char AccountIdBuffer[EOS_EPICACCOUNTID_MAX_LENGTH + 1];
		int32_t BufferSize = sizeof(AccountIdBuffer);
		if (EOS_EpicAccountId_ToString(UserInfo->UserId, AccountIdBuffer, &BufferSize) == EOS_EResult::EOS_Success)
		{
			OutUserInfo.EpicAccountId = UTF8_TO_TCHAR(AccountIdBuffer);
		}

		OutUserInfo.Country = UserInfo->Country ? UTF8_TO_TCHAR(UserInfo->Country) : TEXT("");
		OutUserInfo.DisplayName = UserInfo->DisplayName ? UTF8_TO_TCHAR(UserInfo->DisplayName) : TEXT("");
		OutUserInfo.PreferredLanguage = UserInfo->PreferredLanguage ? UTF8_TO_TCHAR(UserInfo->PreferredLanguage) : TEXT("");
		OutUserInfo.Nickname = UserInfo->Nickname ? UTF8_TO_TCHAR(UserInfo->Nickname) : TEXT("");
		OutUserInfo.DisplayNameSanitized = UserInfo->DisplayNameSanitized ? UTF8_TO_TCHAR(UserInfo->DisplayNameSanitized) : TEXT("");

		EOS_UserInfo_Release(UserInfo);
		return true;
	}

	return false;
}

int32 UEOSKitUserInfoSubsystem::GetExternalAccountInfoCount(const FEOSKitEpicAccountId& LocalUserId, const FEOSKitEpicAccountId& TargetUserId)
{
	EOS_HUserInfo UserInfoHandle = GetUserInfoHandle();
	if (!UserInfoHandle)
	{
		UE_LOG(LogTemp, Error, TEXT("EOSKitUserInfo: Failed to get UserInfo Handle"));
		return 0;
	}

	EOS_UserInfo_GetExternalUserInfoCountOptions Options = {};
	Options.ApiVersion = EOS_USERINFO_GETEXTERNALUSERINFOCOUNT_API_LATEST;
	Options.LocalUserId = LocalUserId.GetValueAsEosType();
	Options.TargetUserId = TargetUserId.GetValueAsEosType();

	return EOS_UserInfo_GetExternalUserInfoCount(UserInfoHandle, &Options);
}

bool UEOSKitUserInfoSubsystem::GetExternalAccountInfoAtIndex(const FEOSKitEpicAccountId& LocalUserId, 
	const FEOSKitEpicAccountId& TargetUserId, 
	int32 Index, 
	FEOSKitExternalUserInfo& OutExternalInfo)
{
	EOS_HUserInfo UserInfoHandle = GetUserInfoHandle();
	if (!UserInfoHandle)
	{
		UE_LOG(LogTemp, Error, TEXT("EOSKitUserInfo: Failed to get UserInfo Handle"));
		return false;
	}

	EOS_UserInfo_CopyExternalUserInfoByIndexOptions Options = {};
	Options.ApiVersion = EOS_USERINFO_COPYEXTERNALUSERINFOBYINDEX_API_LATEST;
	Options.LocalUserId = LocalUserId.GetValueAsEosType();
	Options.TargetUserId = TargetUserId.GetValueAsEosType();
	Options.Index = Index;

	EOS_UserInfo_ExternalUserInfo* ExternalInfo = nullptr;
	EOS_EResult Result = EOS_UserInfo_CopyExternalUserInfoByIndex(UserInfoHandle, &Options, &ExternalInfo);

	if (Result == EOS_EResult::EOS_Success && ExternalInfo)
	{
		OutExternalInfo.AccountType = static_cast<int32>(ExternalInfo->AccountType);
		OutExternalInfo.AccountId = ExternalInfo->AccountId ? UTF8_TO_TCHAR(ExternalInfo->AccountId) : TEXT("");
		OutExternalInfo.DisplayName = ExternalInfo->DisplayName ? UTF8_TO_TCHAR(ExternalInfo->DisplayName) : TEXT("");
		OutExternalInfo.DisplayNameSanitized = ExternalInfo->DisplayNameSanitized ? UTF8_TO_TCHAR(ExternalInfo->DisplayNameSanitized) : TEXT("");

		EOS_UserInfo_ExternalUserInfo_Release(ExternalInfo);
		return true;
	}

	return false;
}

bool UEOSKitUserInfoSubsystem::GetExternalAccountInfoByAccountType(const FEOSKitEpicAccountId& LocalUserId, 
	const FEOSKitEpicAccountId& TargetUserId, 
	int32 AccountType, 
	FEOSKitExternalUserInfo& OutExternalInfo)
{
	EOS_HUserInfo UserInfoHandle = GetUserInfoHandle();
	if (!UserInfoHandle)
	{
		UE_LOG(LogTemp, Error, TEXT("EOSKitUserInfo: Failed to get UserInfo Handle"));
		return false;
	}

	EOS_UserInfo_CopyExternalUserInfoByAccountTypeOptions Options = {};
	Options.ApiVersion = EOS_USERINFO_COPYEXTERNALUSERINFOBYACCOUNTTYPE_API_LATEST;
	Options.LocalUserId = LocalUserId.GetValueAsEosType();
	Options.TargetUserId = TargetUserId.GetValueAsEosType();
	Options.AccountType = static_cast<EOS_EExternalAccountType>(AccountType);

	EOS_UserInfo_ExternalUserInfo* ExternalInfo = nullptr;
	EOS_EResult Result = EOS_UserInfo_CopyExternalUserInfoByAccountType(UserInfoHandle, &Options, &ExternalInfo);

	if (Result == EOS_EResult::EOS_Success && ExternalInfo)
	{
		OutExternalInfo.AccountType = static_cast<int32>(ExternalInfo->AccountType);
		OutExternalInfo.AccountId = ExternalInfo->AccountId ? UTF8_TO_TCHAR(ExternalInfo->AccountId) : TEXT("");
		OutExternalInfo.DisplayName = ExternalInfo->DisplayName ? UTF8_TO_TCHAR(ExternalInfo->DisplayName) : TEXT("");
		OutExternalInfo.DisplayNameSanitized = ExternalInfo->DisplayNameSanitized ? UTF8_TO_TCHAR(ExternalInfo->DisplayNameSanitized) : TEXT("");

		EOS_UserInfo_ExternalUserInfo_Release(ExternalInfo);
		return true;
	}

	return false;
}

