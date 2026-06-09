// Copyright (c) 2023 Betide Studio. All Rights Reserved.


#include "EOK_AuthSubsystem.h"

#include "OnlineSubsystemEOS.h"
#include "OnlineSubsystemEOK/SdkFunctions/ConnectInterface/EOK_ConnectSubsystem.h"

FEOK_NotificationId UEOK_AuthSubsystem::EOK_Auth_AddNotifyLoginStatusChanged(
	const FEOK_Auth_OnLoginStatusChangedCallback& Callback)
{
	if(	IOnlineSubsystem* OnlineSub = IOnlineSubsystem::Get("EOK"))
	{
		if (FOnlineSubsystemEOS* EOSRef = static_cast<FOnlineSubsystemEOS*>(OnlineSub))
		{
			EOS_Auth_AddNotifyLoginStatusChangedOptions Options = {};
			Options.ApiVersion = EOS_AUTH_ADDNOTIFYLOGINSTATUSCHANGED_API_LATEST;
			auto Return = EOS_Auth_AddNotifyLoginStatusChanged(EOSRef->AuthHandle, &Options, this, [](const EOS_Auth_LoginStatusChangedCallbackInfo* Data)
			{
				UEOK_AuthSubsystem* Subsystem = static_cast<UEOK_AuthSubsystem*>(Data->ClientData);
				if (Subsystem)
				{
					Subsystem->OnLoginStatusChanged.ExecuteIfBound(Data->LocalUserId, static_cast<EOK_ELoginStatus>(Data->PrevStatus), static_cast<EOK_ELoginStatus>(Data->CurrentStatus));
				}
			});
			return Return;
		}
	}
	UE_LOG(LogEOK, Error, TEXT("Failed to get EOS subsystem"));
	return FEOK_NotificationId();
}

TEnumAsByte<EEOK_Result> UEOK_AuthSubsystem::EOK_Auth_CopyIdToken(FEOK_EpicAccountId AccountId,
	FEOK_Auth_IdToken& OutToken)
{
	if(	IOnlineSubsystem* OnlineSub = IOnlineSubsystem::Get("EOK"))
	{
		if (FOnlineSubsystemEOS* EOSRef = static_cast<FOnlineSubsystemEOS*>(OnlineSub))
		{
			EOS_Auth_CopyIdTokenOptions Options = {};
			Options.ApiVersion = EOS_AUTH_COPYIDTOKEN_API_LATEST;
			Options.AccountId = AccountId.GetValueAsEosType();
			EOS_Auth_IdToken* Token = nullptr;
			auto Return = EOS_Auth_CopyIdToken(EOSRef->AuthHandle, &Options, &Token);
			if (Return == EOS_EResult::EOS_Success)
			{
				OutToken = *Token;
			}
			return static_cast<EEOK_Result>(Return);
		}
	}
	UE_LOG(LogEOK, Error, TEXT("Failed to get EOS subsystem"));
	return EEOK_Result::EOS_NotFound;
}

TEnumAsByte<EEOK_Result> UEOK_AuthSubsystem::EOK_Auth_CopyUserAuthToken(FEOK_EpicAccountId LocalUserId,
	FEOK_Auth_Token& OutToken)
{
	if(	IOnlineSubsystem* OnlineSub = IOnlineSubsystem::Get("EOK"))
	{
		if (FOnlineSubsystemEOS* EOSRef = static_cast<FOnlineSubsystemEOS*>(OnlineSub))
		{
			EOS_Auth_CopyUserAuthTokenOptions Options = {};
			Options.ApiVersion = EOS_AUTH_COPYUSERAUTHTOKEN_API_LATEST;
			EOS_Auth_Token* Token = nullptr;
			auto Return = EOS_Auth_CopyUserAuthToken(EOSRef->AuthHandle, &Options, LocalUserId.GetValueAsEosType(), &Token);
			if (Return == EOS_EResult::EOS_Success)
			{
				OutToken = *Token;
			}
			return static_cast<EEOK_Result>(Return);
		}
	}
	UE_LOG(LogEOK, Error, TEXT("Failed to get EOS subsystem"));
	return EEOK_Result::EOS_NotFound;
}

FEOK_EpicAccountId UEOK_AuthSubsystem::EOK_Auth_GetLoggedInAccountByIndex(int32 Index)
{
	if(	IOnlineSubsystem* OnlineSub = IOnlineSubsystem::Get("EOK"))
	{
		if (FOnlineSubsystemEOS* EOSRef = static_cast<FOnlineSubsystemEOS*>(OnlineSub))
		{
			return EOS_Auth_GetLoggedInAccountByIndex(EOSRef->AuthHandle, Index);
		}
	}
	UE_LOG(LogEOK, Error, TEXT("Failed to get EOS subsystem"));
	return FEOK_EpicAccountId();
}

int32 UEOK_AuthSubsystem::EOK_Auth_GetLoggedInAccountsCount()
{
	if(	IOnlineSubsystem* OnlineSub = IOnlineSubsystem::Get("EOK"))
	{
		if (FOnlineSubsystemEOS* EOSRef = static_cast<FOnlineSubsystemEOS*>(OnlineSub))
		{
			return EOS_Auth_GetLoggedInAccountsCount(EOSRef->AuthHandle);
		}
	}
	UE_LOG(LogEOK, Error, TEXT("Failed to get EOS subsystem"));
	return 0;
}

TEnumAsByte<EOK_ELoginStatus> UEOK_AuthSubsystem::EOK_Auth_GetLoginStatus(FEOK_EpicAccountId LocalUserId)
{
	if(	IOnlineSubsystem* OnlineSub = IOnlineSubsystem::Get("EOK"))
	{
		if (FOnlineSubsystemEOS* EOSRef = static_cast<FOnlineSubsystemEOS*>(OnlineSub))
		{
			return static_cast<EOK_ELoginStatus>(EOS_Auth_GetLoginStatus(EOSRef->AuthHandle, LocalUserId.GetValueAsEosType()));
		}
	}
	UE_LOG(LogEOK, Error, TEXT("Failed to get EOS subsystem"));
	return EOK_ELoginStatus::EOK_LS_NotLoggedIn;
}

FEOK_EpicAccountId UEOK_AuthSubsystem::EOK_Auth_GetMergedAccountByIndex(FEOK_EpicAccountId LocalUserId,
	int32 Index)
{
	if(	IOnlineSubsystem* OnlineSub = IOnlineSubsystem::Get("EOK"))
	{
		if (FOnlineSubsystemEOS* EOSRef = static_cast<FOnlineSubsystemEOS*>(OnlineSub))
		{
			return EOS_Auth_GetMergedAccountByIndex(EOSRef->AuthHandle, LocalUserId.GetValueAsEosType(), Index);
		}
	}
	UE_LOG(LogEOK, Error, TEXT("Failed to get EOS subsystem"));
	return FEOK_EpicAccountId();
}

int32 UEOK_AuthSubsystem::EOK_Auth_GetMergedAccountsCount(FEOK_EpicAccountId LocalUserId)
{
	if(	IOnlineSubsystem* OnlineSub = IOnlineSubsystem::Get("EOK"))
	{
		if (FOnlineSubsystemEOS* EOSRef = static_cast<FOnlineSubsystemEOS*>(OnlineSub))
		{
			return EOS_Auth_GetMergedAccountsCount(EOSRef->AuthHandle, LocalUserId.GetValueAsEosType());
		}
	}
	UE_LOG(LogEOK, Error, TEXT("Failed to get EOS subsystem"));
	return 0;
}

TEnumAsByte<EEOK_Result> UEOK_AuthSubsystem::EOK_Auth_GetSelectedAccountId(FEOK_EpicAccountId LocalUserId,
	FEOK_EpicAccountId& OutSelectedAccountId)
{
	if(	IOnlineSubsystem* OnlineSub = IOnlineSubsystem::Get("EOK"))
	{
		if (FOnlineSubsystemEOS* EOSRef = static_cast<FOnlineSubsystemEOS*>(OnlineSub))
		{
			EOS_EpicAccountId OutSelectedAccountIdPtr = nullptr;
			auto Result = static_cast<EEOK_Result>(EOS_Auth_GetSelectedAccountId(EOSRef->AuthHandle, LocalUserId.GetValueAsEosType(), &OutSelectedAccountIdPtr));
			if (Result == EEOK_Result::EOS_Success)
			{
				OutSelectedAccountId = OutSelectedAccountIdPtr;
			}
			return Result;
		}
	}
	UE_LOG(LogEOK, Error, TEXT("Failed to get EOS subsystem"));
	return EEOK_Result::EOS_NotFound;
}

void UEOK_AuthSubsystem::EOK_Auth_IdToken_Release(FEOK_Auth_IdToken& Token)
{
	EOS_Auth_IdToken ReleaseToken = Token.GetValueAsEosType();
	EOS_Auth_IdToken_Release(&ReleaseToken);
}

void UEOK_AuthSubsystem::EOK_Auth_RemoveNotifyLoginStatusChanged(FEOK_NotificationId NotificationId)
{
	if(	IOnlineSubsystem* OnlineSub = IOnlineSubsystem::Get("EOK"))
	{
		if (FOnlineSubsystemEOS* EOSRef = static_cast<FOnlineSubsystemEOS*>(OnlineSub))
		{
			EOS_Auth_RemoveNotifyLoginStatusChanged(EOSRef->AuthHandle, NotificationId.GetValueAsEosType());
			return;
		}
	}
	UE_LOG(LogEOK, Error, TEXT("Failed to get EOS subsystem"));
}

void UEOK_AuthSubsystem::EOK_Auth_Token_Release(FEOK_Auth_Token& Token)
{
	EOS_Auth_Token ReleaseToken = Token.GetValueAsEosType();
	EOS_Auth_Token_Release(&ReleaseToken);
}