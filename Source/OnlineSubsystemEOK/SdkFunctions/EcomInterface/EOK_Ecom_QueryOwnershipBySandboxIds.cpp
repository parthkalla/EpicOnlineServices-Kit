// Copyright (c) 2023 Betide Studio. All Rights Reserved.


#include "EOK_Ecom_QueryOwnershipBySandboxIds.h"

#include "OnlineSubsystemEOS.h"
#include "Async/Async.h"
#include "OnlineSubsystemEOK/SdkFunctions/ConnectInterface/EOK_ConnectSubsystem.h"

UEOK_Ecom_QueryOwnershipBySandboxIds* UEOK_Ecom_QueryOwnershipBySandboxIds::EOK_Ecom_QueryOwnershipBySandboxIds(
	FEOK_Ecom_QueryOwnershipBySandboxIdsOptions QueryOwnershipBySandboxIdsOptions)
{
	UEOK_Ecom_QueryOwnershipBySandboxIds* Node = NewObject<UEOK_Ecom_QueryOwnershipBySandboxIds>();
	Node->Var_QueryOwnershipBySandboxIdsOptions = QueryOwnershipBySandboxIdsOptions;
	return Node;
}

void UEOK_Ecom_QueryOwnershipBySandboxIds::OnQueryOwnershipBySandboxIdsCallback(
	const EOS_Ecom_QueryOwnershipBySandboxIdsCallbackInfo* Data)
{
	if (UEOK_Ecom_QueryOwnershipBySandboxIds* Node = static_cast<UEOK_Ecom_QueryOwnershipBySandboxIds*>(Data->ClientData))
	{
		AsyncTask(ENamedThreads::GameThread, [Node, Data]()
		{
			Node->OnCallback.Broadcast(*Data);
		});
		Node->SetReadyToDestroy();
#if ENGINE_MAJOR_VERSION == 5
		Node->MarkAsGarbage();
#else
		Node->MarkPendingKill();
#endif
	}
}

void UEOK_Ecom_QueryOwnershipBySandboxIds::Activate()
{
	Super::Activate();
	if (IOnlineSubsystem* OnlineSub = IOnlineSubsystem::Get("EOK"))
	{
		if (FOnlineSubsystemEOS* EOSRef = static_cast<FOnlineSubsystemEOS*>(OnlineSub))
		{
			EOS_Ecom_QueryOwnershipBySandboxIdsOptions QueryOwnershipBySandboxIdsOptions = Var_QueryOwnershipBySandboxIdsOptions.ToEOS_Ecom_QueryOwnershipBySandboxIdsOptions();
			EOS_Ecom_QueryOwnershipBySandboxIds(EOSRef->EcomHandle, &QueryOwnershipBySandboxIdsOptions, this, &UEOK_Ecom_QueryOwnershipBySandboxIds::OnQueryOwnershipBySandboxIdsCallback);
			return;
		}
	}
	UE_LOG(LogEOK, Error, TEXT("Failed to query ownership by sandbox ids either OnlineSubsystem is not valid or EOSRef is not valid."));
	OnCallback.Broadcast(FEOK_Ecom_QueryOwnershipBySandboxIdsCallbackInfo());
	SetReadyToDestroy();
	#if ENGINE_MAJOR_VERSION == 5
	MarkAsGarbage();
#else
	MarkPendingKill();
#endif
}
