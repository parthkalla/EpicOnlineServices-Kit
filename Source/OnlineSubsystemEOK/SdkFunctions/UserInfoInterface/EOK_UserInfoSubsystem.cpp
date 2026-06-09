// Copyright (c) 2023 Betide Studio. All Rights Reserved.


#include "EOK_UserInfoSubsystem.h"

#include "OnlineSubsystemEOS.h"
#include "OnlineSubsystemEOK/SdkFunctions/ConnectInterface/EOK_ConnectSubsystem.h"

TEnumAsByte<EEOK_Result> UEOK_UserInfoSubsystem::EOK_UserInfo_CopyBestDisplayName(FEOK_EpicAccountId LocalUserId,
                                                              FEOK_EpicAccountId TargetUserId, FEOK_UserInfo_BestDisplayName& OutBestDisplayName)
{
	if (IOnlineSubsystem* OnlineSub = IOnlineSubsystem::Get("EOK"))
	{
		if (FOnlineSubsystemEOS* EOSRef = static_cast<FOnlineSubsystemEOS*>(OnlineSub))
		{
			EOS_UserInfo_CopyBestDisplayNameOptions Options;
			Options.ApiVersion = EOS_USERINFO_COPYBESTDISPLAYNAME_API_LATEST;
			Options.LocalUserId = LocalUserId.GetValueAsEosType();
			Options.TargetUserId = TargetUserId.GetValueAsEosType();
			EOS_UserInfo_BestDisplayName* TempRef;
			auto CopyBestDisplayNameResult = EOS_UserInfo_CopyBestDisplayName(EOSRef->UserInfoHandle, &Options, &TempRef);
			if (CopyBestDisplayNameResult == EOS_EResult::EOS_Success)
			{
				OutBestDisplayName = *TempRef;
				EOS_UserInfo_BestDisplayName_Release(TempRef);
			}
			return static_cast<EEOK_Result>(CopyBestDisplayNameResult);
		}
	}
	UE_LOG(LogEOK, Error, TEXT("UEOK_UserInfoSubsystem::EOK_UserInfo_CopyBestDisplayName: Failed to get EOS subsystem"));
	return EEOK_Result::EOS_NotFound;
}

TEnumAsByte<EEOK_Result> UEOK_UserInfoSubsystem::EOK_UserInfo_CopyBestDisplayNameWithPlatform(
	FEOK_EpicAccountId LocalUserId, FEOK_EpicAccountId TargetUserId, const int32& Platform,
	FEOK_UserInfo_BestDisplayName& OutBestDisplayName)
{
	if (IOnlineSubsystem* OnlineSub = IOnlineSubsystem::Get("EOK"))
	{
		if (FOnlineSubsystemEOS* EOSRef = static_cast<FOnlineSubsystemEOS*>(OnlineSub))
		{
			EOS_UserInfo_CopyBestDisplayNameWithPlatformOptions Options;
			Options.ApiVersion = EOS_USERINFO_COPYBESTDISPLAYNAMEWITHPLATFORM_API_LATEST;
			Options.LocalUserId = LocalUserId.GetValueAsEosType();
			Options.TargetUserId = TargetUserId.GetValueAsEosType();
			Options.TargetPlatformType = Platform;
			EOS_UserInfo_BestDisplayName* TempRef;
			auto CopyBestDisplayNameWithPlatformResult = EOS_UserInfo_CopyBestDisplayNameWithPlatform(EOSRef->UserInfoHandle, &Options, &TempRef);
			if (CopyBestDisplayNameWithPlatformResult == EOS_EResult::EOS_Success)
			{
				OutBestDisplayName = *TempRef;
				EOS_UserInfo_BestDisplayName_Release(TempRef);
			}
			return static_cast<EEOK_Result>(CopyBestDisplayNameWithPlatformResult);
		}
	}
	UE_LOG(LogEOK, Error, TEXT("UEOK_UserInfoSubsystem::EOK_UserInfo_CopyBestDisplayNameWithPlatform: Failed to get EOS subsystem"));
	return EEOK_Result::EOS_NotFound;
}

TEnumAsByte<EEOK_Result> UEOK_UserInfoSubsystem::EOK_UserInfo_CopyExternalUserInfoByAccountId(
	FEOK_EpicAccountId LocalUserId, FEOK_EpicAccountId TargetUserId, FString AccountId,
	FEOK_UserInfo_ExternalUserInfo& OutExternalUserInfo)
{
	if (IOnlineSubsystem* OnlineSub = IOnlineSubsystem::Get("EOK"))
	{
		if (FOnlineSubsystemEOS* EOSRef = static_cast<FOnlineSubsystemEOS*>(OnlineSub))
		{
			EOS_UserInfo_CopyExternalUserInfoByAccountIdOptions Options;
			Options.ApiVersion = EOS_USERINFO_COPYEXTERNALUSERINFOBYACCOUNTID_API_LATEST;
			Options.LocalUserId = LocalUserId.GetValueAsEosType();
			Options.TargetUserId = TargetUserId.GetValueAsEosType();
			Options.AccountId = TCHAR_TO_ANSI(*AccountId);
			EOS_UserInfo_ExternalUserInfo* TempRef;
			auto CopyExternalUserInfoByAccountIdResult = EOS_UserInfo_CopyExternalUserInfoByAccountId(EOSRef->UserInfoHandle, &Options, &TempRef);
			if (CopyExternalUserInfoByAccountIdResult == EOS_EResult::EOS_Success)
			{
				OutExternalUserInfo = *TempRef;
				EOS_UserInfo_ExternalUserInfo_Release(TempRef);
			}
			return static_cast<EEOK_Result>(CopyExternalUserInfoByAccountIdResult);
		}
	}
	UE_LOG(LogEOK, Error, TEXT("UEOK_UserInfoSubsystem::EOK_UserInfo_CopyExternalUserInfoByAccountId: Failed to get EOS subsystem"));
	return EEOK_Result::EOS_NotFound;
}

TEnumAsByte<EEOK_Result> UEOK_UserInfoSubsystem::EOK_UserInfo_CopyExternalUserInfoByAccountType(
	FEOK_EpicAccountId LocalUserId, FEOK_EpicAccountId TargetUserId,
	const TEnumAsByte<EEOK_EExternalAccountType>& AccountType, FEOK_UserInfo_ExternalUserInfo& OutExternalUserInfo)
{
	if (IOnlineSubsystem* OnlineSub = IOnlineSubsystem::Get("EOK"))
	{
		if (FOnlineSubsystemEOS* EOSRef = static_cast<FOnlineSubsystemEOS*>(OnlineSub))
		{
			EOS_UserInfo_CopyExternalUserInfoByAccountTypeOptions Options;
			Options.ApiVersion = EOS_USERINFO_COPYEXTERNALUSERINFOBYACCOUNTTYPE_API_LATEST;
			Options.LocalUserId = LocalUserId.GetValueAsEosType();
			Options.TargetUserId = TargetUserId.GetValueAsEosType();
			Options.AccountType = static_cast<EOS_EExternalAccountType>(AccountType.GetValue());
			EOS_UserInfo_ExternalUserInfo* TempRef;
			auto CopyExternalUserInfoByAccountTypeResult = EOS_UserInfo_CopyExternalUserInfoByAccountType(EOSRef->UserInfoHandle, &Options, &TempRef);
			if (CopyExternalUserInfoByAccountTypeResult == EOS_EResult::EOS_Success)
			{
				OutExternalUserInfo = *TempRef;
				EOS_UserInfo_ExternalUserInfo_Release(TempRef);
			}
			return static_cast<EEOK_Result>(CopyExternalUserInfoByAccountTypeResult);
		}
	}
	UE_LOG(LogEOK, Error, TEXT("UEOK_UserInfoSubsystem::EOK_UserInfo_CopyExternalUserInfoByAccountType: Failed to get EOS subsystem"));
	return EEOK_Result::EOS_NotFound;
}

TEnumAsByte<EEOK_Result> UEOK_UserInfoSubsystem::EOK_UserInfo_CopyExternalUserInfoByIndex(
	FEOK_EpicAccountId LocalUserId, FEOK_EpicAccountId TargetUserId, const int32& Index,
	FEOK_UserInfo_ExternalUserInfo& OutExternalUserInfo)
{
	if (IOnlineSubsystem* OnlineSub = IOnlineSubsystem::Get("EOK"))
	{
		if (FOnlineSubsystemEOS* EOSRef = static_cast<FOnlineSubsystemEOS*>(OnlineSub))
		{
			EOS_UserInfo_CopyExternalUserInfoByIndexOptions Options;
			Options.ApiVersion = EOS_USERINFO_COPYEXTERNALUSERINFOBYINDEX_API_LATEST;
			Options.LocalUserId = LocalUserId.GetValueAsEosType();
			Options.TargetUserId = TargetUserId.GetValueAsEosType();
			Options.Index = Index;
			EOS_UserInfo_ExternalUserInfo* TempRef;
			auto CopyExternalUserInfoByIndexResult = EOS_UserInfo_CopyExternalUserInfoByIndex(EOSRef->UserInfoHandle, &Options, &TempRef);
			if (CopyExternalUserInfoByIndexResult == EOS_EResult::EOS_Success)
			{
				OutExternalUserInfo = *TempRef;
				EOS_UserInfo_ExternalUserInfo_Release(TempRef);
			}
			return static_cast<EEOK_Result>(CopyExternalUserInfoByIndexResult);
		}
	}
	UE_LOG(LogEOK, Error, TEXT("UEOK_UserInfoSubsystem::EOK_UserInfo_CopyExternalUserInfoByIndex: Failed to get EOS subsystem"));
	return EEOK_Result::EOS_NotFound;
}

TEnumAsByte<EEOK_Result> UEOK_UserInfoSubsystem::EOK_UserInfo_CopyUserInfo(FEOK_EpicAccountId LocalUserId,
	FEOK_EpicAccountId TargetUserId, FEOK_UserInfo& OutUserInfo)
{
	if (IOnlineSubsystem* OnlineSub = IOnlineSubsystem::Get("EOK"))
	{
		if (FOnlineSubsystemEOS* EOSRef = static_cast<FOnlineSubsystemEOS*>(OnlineSub))
		{
			EOS_UserInfo_CopyUserInfoOptions Options;
			Options.ApiVersion = EOS_USERINFO_COPYUSERINFO_API_LATEST;
			Options.LocalUserId = LocalUserId.GetValueAsEosType();
			Options.TargetUserId = TargetUserId.GetValueAsEosType();
			EOS_UserInfo* TempRef;
			auto CopyUserInfoResult = EOS_UserInfo_CopyUserInfo(EOSRef->UserInfoHandle, &Options, &TempRef);
			if (CopyUserInfoResult == EOS_EResult::EOS_Success)
			{
				OutUserInfo = *TempRef;
				EOS_UserInfo_Release(TempRef);
			}
			return static_cast<EEOK_Result>(CopyUserInfoResult);
		}
	}
	UE_LOG(LogEOK, Error, TEXT("UEOK_UserInfoSubsystem::EOK_UserInfo_CopyUserInfo: Failed to get EOS subsystem"));
	return EEOK_Result::EOS_NotFound;
}

int32 UEOK_UserInfoSubsystem::EOK_UserInfo_GetExternalUserInfoCount(FEOK_EpicAccountId LocalUserId,
	FEOK_EpicAccountId TargetUserId)
{
	if (IOnlineSubsystem* OnlineSub = IOnlineSubsystem::Get("EOK"))
	{
		if (FOnlineSubsystemEOS* EOSRef = static_cast<FOnlineSubsystemEOS*>(OnlineSub))
		{
			EOS_UserInfo_GetExternalUserInfoCountOptions Options;
			Options.ApiVersion = EOS_USERINFO_GETEXTERNALUSERINFOCOUNT_API_LATEST;
			Options.LocalUserId = LocalUserId.GetValueAsEosType();
			Options.TargetUserId = TargetUserId.GetValueAsEosType();
			return EOS_UserInfo_GetExternalUserInfoCount(EOSRef->UserInfoHandle, &Options);
		}
	}
	UE_LOG(LogEOK, Error, TEXT("UEOK_UserInfoSubsystem::EOK_UserInfo_GetExternalUserInfoCount: Failed to get EOS subsystem"));
	return 0;
}

int32 UEOK_UserInfoSubsystem::EOK_UserInfo_GetLocalPlatformType()
{
	if (IOnlineSubsystem* OnlineSub = IOnlineSubsystem::Get("EOK"))
	{
		if (FOnlineSubsystemEOS* EOSRef = static_cast<FOnlineSubsystemEOS*>(OnlineSub))
		{
			EOS_UserInfo_GetLocalPlatformTypeOptions Options;
			Options.ApiVersion = EOS_USERINFO_GETLOCALPLATFORMTYPE_API_LATEST;
			return EOS_UserInfo_GetLocalPlatformType(EOSRef->UserInfoHandle, &Options);
		}
	}
	UE_LOG(LogEOK, Error, TEXT("UEOK_UserInfoSubsystem::EOK_UserInfo_GetLocalPlatformType: Failed to get EOS subsystem"));
	return 0;
}
