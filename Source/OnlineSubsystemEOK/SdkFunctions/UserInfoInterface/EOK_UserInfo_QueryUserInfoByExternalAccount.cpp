// Copyright (c) 2023 Betide Studio. All Rights Reserved.


#include "EOK_UserInfo_QueryUserInfoByExternalAccount.h"

#include "OnlineSubsystemEOS.h"
#include "Async/Async.h"
#include "OnlineSubsystemEOK/SdkFunctions/ConnectInterface/EOK_ConnectSubsystem.h"

UEOK_UserInfo_QueryUserInfoByExternalAccount* UEOK_UserInfo_QueryUserInfoByExternalAccount::
EOK_UserInfo_QueryUserInfoByExternalAccount(const FEOK_EpicAccountId& LocalUserId,
	const TEnumAsByte<EEOK_EExternalAccountType>& AccountType, const FString& ExternalAccountId)
{
	UEOK_UserInfo_QueryUserInfoByExternalAccount* Node = NewObject<UEOK_UserInfo_QueryUserInfoByExternalAccount>();
	Node->Var_LocalUserId = LocalUserId;
	Node->Var_AccountType = AccountType;
	Node->Var_ExternalAccountId = ExternalAccountId;
	return Node;
}

void UEOK_UserInfo_QueryUserInfoByExternalAccount::Activate()
{
	Super::Activate();
	if (IOnlineSubsystem* OnlineSub = IOnlineSubsystem::Get("EOK"))
	{
		if (FOnlineSubsystemEOS* EOSRef = static_cast<FOnlineSubsystemEOS*>(OnlineSub))
		{
			EOS_UserInfo_QueryUserInfoByExternalAccountOptions Options;
			Options.ApiVersion = EOS_USERINFO_QUERYUSERINFOBYEXTERNALACCOUNT_API_LATEST;
			Options.LocalUserId = Var_LocalUserId.GetValueAsEosType();
			Options.AccountType = static_cast<EOS_EExternalAccountType>(Var_AccountType.GetValue());
			Options.ExternalAccountId = TCHAR_TO_ANSI(*Var_ExternalAccountId);
			EOS_UserInfo_QueryUserInfoByExternalAccount(EOSRef->UserInfoHandle, &Options, this,&UEOK_UserInfo_QueryUserInfoByExternalAccount::OnQueryUserInfoByExternalAccountCallback);
			return;
		}
	}
	UE_LOG(LogEOK, Error, TEXT("UEOK_UserInfo_QueryUserInfoByExternalAccount::Activate: Unable to get EOS subsystem."));
	OnCallback.Broadcast(EEOK_Result::EOS_NotFound, Var_LocalUserId, FEOK_EpicAccountId(), Var_AccountType, Var_ExternalAccountId);
	SetReadyToDestroy();
	#if ENGINE_MAJOR_VERSION == 5
	MarkAsGarbage();
#else
	MarkPendingKill();
#endif
}

void UEOK_UserInfo_QueryUserInfoByExternalAccount::OnQueryUserInfoByExternalAccountCallback(
	const EOS_UserInfo_QueryUserInfoByExternalAccountCallbackInfo* Data)
{
	if (UEOK_UserInfo_QueryUserInfoByExternalAccount* Node = static_cast<UEOK_UserInfo_QueryUserInfoByExternalAccount*>(Data->ClientData))
	{
		AsyncTask(ENamedThreads::GameThread, [Node, Data]()
		{
			Node->OnCallback.Broadcast(static_cast<EEOK_Result>(Data->ResultCode), Data->LocalUserId, Data->TargetUserId, static_cast<EEOK_EExternalAccountType>(Data->AccountType), Data->ExternalAccountId);
		});
		Node->SetReadyToDestroy();
#if ENGINE_MAJOR_VERSION == 5
		Node->MarkAsGarbage();
#else
		Node->MarkPendingKill();
#endif
	}
}
