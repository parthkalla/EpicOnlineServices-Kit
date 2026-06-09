// Copyright (c) 2023 Betide Studio. All Rights Reserved.


#include "EOK_UserInfo_QueryUserInfoByDisplayName.h"

#include "Async/Async.h"
#include "OnlineSubsystemEOK/SdkFunctions/ConnectInterface/EOK_ConnectSubsystem.h"

UEOK_UserInfo_QueryUserInfoByDisplayName* UEOK_UserInfo_QueryUserInfoByDisplayName::
EOK_UserInfo_QueryUserInfoByDisplayName(const FEOK_EpicAccountId& LocalUserId, const FString& DisplayName)
{
	UEOK_UserInfo_QueryUserInfoByDisplayName* Node = NewObject<UEOK_UserInfo_QueryUserInfoByDisplayName>();
	Node->Var_LocalUserId = LocalUserId;
	Node->Var_DisplayName = DisplayName;
	return Node;
}

void UEOK_UserInfo_QueryUserInfoByDisplayName::Activate()
{
	Super::Activate();
	if (IOnlineSubsystem* OnlineSub = IOnlineSubsystem::Get("EOK"))
	{
		if (FOnlineSubsystemEOS* EOSRef = static_cast<FOnlineSubsystemEOS*>(OnlineSub))
		{
			EOS_UserInfo_QueryUserInfoByDisplayNameOptions Options;
			Options.ApiVersion = EOS_USERINFO_QUERYUSERINFOBYDISPLAYNAME_API_LATEST;
			Options.LocalUserId = Var_LocalUserId.GetValueAsEosType();
			Options.DisplayName = TCHAR_TO_ANSI(*Var_DisplayName);
			EOS_UserInfo_QueryUserInfoByDisplayName(EOSRef->UserInfoHandle, &Options, this, &UEOK_UserInfo_QueryUserInfoByDisplayName::OnQueryUserInfoByDisplayNameCallback);
			return;
		}
	}
	UE_LOG(LogEOK, Error, TEXT("UEOK_UserInfo_QueryUserInfoByDisplayName::Activate: Unable to get EOS subsystem."));
	OnCallback.Broadcast(EEOK_Result::EOS_NotFound, Var_LocalUserId, FEOK_EpicAccountId(), Var_DisplayName);
	SetReadyToDestroy();
	#if ENGINE_MAJOR_VERSION == 5
	MarkAsGarbage();
#else
	MarkPendingKill();
#endif
}

void UEOK_UserInfo_QueryUserInfoByDisplayName::OnQueryUserInfoByDisplayNameCallback(
	const EOS_UserInfo_QueryUserInfoByDisplayNameCallbackInfo* Data)
{
	if (UEOK_UserInfo_QueryUserInfoByDisplayName* Node = static_cast<UEOK_UserInfo_QueryUserInfoByDisplayName*>(Data->ClientData))
	{
		AsyncTask(ENamedThreads::GameThread, [Node, Data]()
		{
			Node->OnCallback.Broadcast(static_cast<EEOK_Result>(Data->ResultCode), Data->LocalUserId, Data->TargetUserId, Data->DisplayName);
		});
		Node->SetReadyToDestroy();
#if ENGINE_MAJOR_VERSION == 5
		Node->MarkAsGarbage();
#else
		Node->MarkPendingKill();
#endif
	}
}
