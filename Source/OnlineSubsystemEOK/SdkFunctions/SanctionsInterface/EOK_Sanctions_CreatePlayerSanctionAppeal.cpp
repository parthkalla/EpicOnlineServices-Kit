// Copyright (c) 2023 Betide Studio. All Rights Reserved.


#include "EOK_Sanctions_CreatePlayerSanctionAppeal.h"

#include "Async/Async.h"
#include "OnlineSubsystemEOK/SdkFunctions/ConnectInterface/EOK_ConnectSubsystem.h"

UEOK_Sanctions_CreatePlayerSanctionAppeal* UEOK_Sanctions_CreatePlayerSanctionAppeal::EOK_Sanctions_CreatePlayerSanctionAppeal(FEOK_ProductUserId LocalUserId, const TEnumAsByte<EEOK_ESanctionAppealReason>& AppealReason, const FString& ReferenceId)
{
	UEOK_Sanctions_CreatePlayerSanctionAppeal* Node = NewObject<UEOK_Sanctions_CreatePlayerSanctionAppeal>();
	Node->Var_LocalUserId = LocalUserId;
	Node->Var_AppealReason = AppealReason;
	Node->Var_ReferenceId = ReferenceId;
	return Node;
}

void UEOK_Sanctions_CreatePlayerSanctionAppeal::Activate()
{
	Super::Activate();
	if (IOnlineSubsystem* OnlineSub = IOnlineSubsystem::Get("EOK"))
	{
		if (FOnlineSubsystemEOS* EOSRef = static_cast<FOnlineSubsystemEOS*>(OnlineSub))
		{
			EOS_Sanctions_CreatePlayerSanctionAppealOptions Options = {};
			Options.ApiVersion = EOS_SANCTIONS_CREATEPLAYERSANCTIONAPPEAL_API_LATEST;
			Options.LocalUserId = Var_LocalUserId.GetValueAsEosType();
			Options.Reason = static_cast<EOS_ESanctionAppealReason>(Var_AppealReason.GetValue());
			Options.ReferenceId = TCHAR_TO_ANSI(*Var_ReferenceId);
			EOS_Sanctions_CreatePlayerSanctionAppeal(EOSRef->SanctionsHandle, &Options, this, &UEOK_Sanctions_CreatePlayerSanctionAppeal::Internal_OnCreatePlayerSanctionAppealComplete);
			return;
		}
	}
	UE_LOG(LogEOK, Error, TEXT("UEOK_Sanctions_CreatePlayerSanctionAppeal::Activate: Unable to get EOS SDK"));
	OnCallback.Broadcast(Var_LocalUserId, EEOK_Result::EOS_NotFound);
	SetReadyToDestroy();
	#if ENGINE_MAJOR_VERSION == 5
	MarkAsGarbage();
#else
	MarkPendingKill();
#endif
}

void UEOK_Sanctions_CreatePlayerSanctionAppeal::Internal_OnCreatePlayerSanctionAppealComplete(
	const EOS_Sanctions_CreatePlayerSanctionAppealCallbackInfo* Data)
{
	if (UEOK_Sanctions_CreatePlayerSanctionAppeal* Node = static_cast<UEOK_Sanctions_CreatePlayerSanctionAppeal*>(Data->ClientData))
	{
		AsyncTask(ENamedThreads::GameThread, [Node, Data]()
		{
			Node->OnCallback.Broadcast(Node->Var_LocalUserId, static_cast<EEOK_Result>(Data->ResultCode));
			Node->SetReadyToDestroy();
#if ENGINE_MAJOR_VERSION == 5
Node->MarkAsGarbage();
#else
Node->MarkPendingKill();
#endif
		});
	}
}
