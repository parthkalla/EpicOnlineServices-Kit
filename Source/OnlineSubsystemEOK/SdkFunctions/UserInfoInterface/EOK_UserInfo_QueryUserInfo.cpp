// Copyright (c) 2023 Betide Studio. All Rights Reserved.


#include "EOK_UserInfo_QueryUserInfo.h"

#include "Async/Async.h"
#include "OnlineSubsystemEOK/SdkFunctions/ConnectInterface/EOK_ConnectSubsystem.h"

UEOK_UserInfo_QueryUserInfo* UEOK_UserInfo_QueryUserInfo::EOK_UserInfo_QueryUserInfo(
	const FEOK_EpicAccountId& LocalUserId, const FEOK_EpicAccountId& TargetUserId)
{
	UEOK_UserInfo_QueryUserInfo* Node = NewObject<UEOK_UserInfo_QueryUserInfo>();
	Node->Var_LocalUserId = LocalUserId;
	Node->Var_TargetUserId = TargetUserId;
	return Node;
}

void UEOK_UserInfo_QueryUserInfo::Activate()
{
	Super::Activate();
	if(	IOnlineSubsystem* OnlineSub = IOnlineSubsystem::Get("EOK"))
	{
		if (FOnlineSubsystemEOS* EOSRef = static_cast<FOnlineSubsystemEOS*>(OnlineSub))
		{
			EOS_UserInfo_QueryUserInfoOptions Options;
			Options.ApiVersion = EOS_USERINFO_QUERYUSERINFO_API_LATEST;
			Options.LocalUserId = Var_LocalUserId.GetValueAsEosType();
			Options.TargetUserId = Var_TargetUserId.GetValueAsEosType();
			EOS_UserInfo_QueryUserInfo(EOSRef->UserInfoHandle, &Options, this, &UEOK_UserInfo_QueryUserInfo::OnQueryUserInfoCallback);
			return;
		}
	}
	UE_LOG(LogEOK, Error, TEXT("UEOK_UserInfo_QueryUserInfo::Activate: Unable to get EOS subsystem."));
	OnCallback.Broadcast(EEOK_Result::EOS_NotFound, Var_LocalUserId, Var_TargetUserId);
	SetReadyToDestroy();
	#if ENGINE_MAJOR_VERSION == 5
	MarkAsGarbage();
#else
	MarkPendingKill();
#endif
}

void UEOK_UserInfo_QueryUserInfo::OnQueryUserInfoCallback(const EOS_UserInfo_QueryUserInfoCallbackInfo* Data)
{
	if(UEOK_UserInfo_QueryUserInfo* Node = static_cast<UEOK_UserInfo_QueryUserInfo*>(Data->ClientData))
	{
		AsyncTask(ENamedThreads::GameThread, [Node, Data]()
		{
			Node->OnCallback.Broadcast(static_cast<EEOK_Result>(Data->ResultCode), Node->Var_LocalUserId, Node->Var_TargetUserId);
		});
		Node->SetReadyToDestroy();
#if ENGINE_MAJOR_VERSION == 5
		Node->MarkAsGarbage();
#else
		Node->MarkPendingKill();
#endif
	}
}