// Copyright (c) 2023 Betide Studio. All Rights Reserved.


#include "EOK_Sanctions_QueryActivePlayerSanctions.h"

#include "Async/Async.h"
#include "OnlineSubsystemEOK/SdkFunctions/ConnectInterface/EOK_ConnectSubsystem.h"

UEOK_Sanctions_QueryActivePlayerSanctions* UEOK_Sanctions_QueryActivePlayerSanctions::
EOK_Sanctions_QueryActivePlayerSanctions(FEOK_ProductUserId LocalUserId, FEOK_ProductUserId TargetUserId)
{
	UEOK_Sanctions_QueryActivePlayerSanctions* Node = NewObject<UEOK_Sanctions_QueryActivePlayerSanctions>();
	Node->Var_LocalUserId = LocalUserId;
	Node->Var_TargetUserId = TargetUserId;
	return Node;
}

void UEOK_Sanctions_QueryActivePlayerSanctions::Activate()
{
	Super::Activate();
	if (IOnlineSubsystem* OnlineSub = IOnlineSubsystem::Get("EOK"))
	{
		if (FOnlineSubsystemEOS* EOSRef = static_cast<FOnlineSubsystemEOS*>(OnlineSub))
		{
			EOS_Sanctions_QueryActivePlayerSanctionsOptions Options = {};
			Options.ApiVersion = EOS_SANCTIONS_QUERYACTIVEPLAYERSANCTIONS_API_LATEST;
			Options.LocalUserId = Var_LocalUserId.GetValueAsEosType();
			Options.TargetUserId = Var_TargetUserId.GetValueAsEosType();
			EOS_Sanctions_QueryActivePlayerSanctions(EOSRef->SanctionsHandle, &Options, this, &UEOK_Sanctions_QueryActivePlayerSanctions::Internal_OnQueryActivePlayerSanctionsComplete);
			return;
		}
	}
	UE_LOG(LogEOK, Error, TEXT("UEOK_Sanctions_QueryActivePlayerSanctions::Activate: Unable to get EOS SDK"));
	OnCallback.Broadcast(Var_LocalUserId, Var_TargetUserId, EEOK_Result::EOS_NotFound);
	SetReadyToDestroy();
	#if ENGINE_MAJOR_VERSION == 5
	MarkAsGarbage();
#else
	MarkPendingKill();
#endif
}

void UEOK_Sanctions_QueryActivePlayerSanctions::Internal_OnQueryActivePlayerSanctionsComplete(
	const EOS_Sanctions_QueryActivePlayerSanctionsCallbackInfo* Data)
{
	if (UEOK_Sanctions_QueryActivePlayerSanctions* Node = static_cast<UEOK_Sanctions_QueryActivePlayerSanctions*>(Data->ClientData))
	{
		AsyncTask(ENamedThreads::GameThread, [Node, Data]()
		{
			Node->OnCallback.Broadcast(Node->Var_LocalUserId, Node->Var_TargetUserId, static_cast<EEOK_Result>(Data->ResultCode));
			Node->SetReadyToDestroy();
#if ENGINE_MAJOR_VERSION == 5
Node->MarkAsGarbage();
#else
Node->MarkPendingKill();
#endif
		});
	}
}