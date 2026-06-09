// Copyright (c) 2023 Betide Studio. All Rights Reserved.


#include "EOK_Ecom_QueryOwnershipToken.h"

#include "OnlineSubsystemEOS.h"
#include "Async/Async.h"
#include "OnlineSubsystemEOK/SdkFunctions/ConnectInterface/EOK_ConnectSubsystem.h"

UEOK_Ecom_QueryOwnershipToken* UEOK_Ecom_QueryOwnershipToken::EOK_Ecom_QueryOwnershipToken(
	FEOK_Ecom_QueryOwnershipTokenOptions QueryOwnershipTokenOptions)
{
	UEOK_Ecom_QueryOwnershipToken* Node = NewObject<UEOK_Ecom_QueryOwnershipToken>();
	Node->Var_QueryOwnershipTokenOptions = QueryOwnershipTokenOptions;
	return Node;
}

void UEOK_Ecom_QueryOwnershipToken::OnQueryOwnershipTokenCallback(const EOS_Ecom_QueryOwnershipTokenCallbackInfo* Data)
{
	if (UEOK_Ecom_QueryOwnershipToken* Node = static_cast<UEOK_Ecom_QueryOwnershipToken*>(Data->ClientData))
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

void UEOK_Ecom_QueryOwnershipToken::Activate()
{
	Super::Activate();
	if (IOnlineSubsystem* OnlineSub = IOnlineSubsystem::Get("EOK"))
	{
		if (FOnlineSubsystemEOS* EOSRef = static_cast<FOnlineSubsystemEOS*>(OnlineSub))
		{
			EOS_Ecom_QueryOwnershipTokenOptions QueryOwnershipTokenOptions = Var_QueryOwnershipTokenOptions.ToEOS_Ecom_QueryOwnershipOptions();
			EOS_Ecom_QueryOwnershipToken(EOSRef->EcomHandle, &QueryOwnershipTokenOptions, this, &UEOK_Ecom_QueryOwnershipToken::OnQueryOwnershipTokenCallback);
			return;
		}
	}
	UE_LOG(LogEOK, Error, TEXT("Failed to query ownership token either OnlineSubsystem is not valid or EOSRef is not valid."));
	OnCallback.Broadcast(FEOK_Ecom_QueryOwnershipTokenCallbackInfo());
	SetReadyToDestroy();
	#if ENGINE_MAJOR_VERSION == 5
	MarkAsGarbage();
#else
	MarkPendingKill();
#endif
}
