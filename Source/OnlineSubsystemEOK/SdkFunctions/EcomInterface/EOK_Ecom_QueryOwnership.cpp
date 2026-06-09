// Copyright (c) 2023 Betide Studio. All Rights Reserved.


#include "EOK_Ecom_QueryOwnership.h"

#include "OnlineSubsystemEOS.h"
#include "Async/Async.h"
#include "OnlineSubsystemEOK/SdkFunctions/ConnectInterface/EOK_ConnectSubsystem.h"

UEOK_Ecom_QueryOwnership* UEOK_Ecom_QueryOwnership::EOK_Ecom_QueryOwnership(
	FEOK_Ecom_QueryOwnershipOptions QueryOwnershipOptions)
{
	UEOK_Ecom_QueryOwnership* Node = NewObject<UEOK_Ecom_QueryOwnership>();
	Node->Var_QueryOwnershipOptions = QueryOwnershipOptions;
	return Node;
}

void UEOK_Ecom_QueryOwnership::OnQueryOwnershipCallback(const EOS_Ecom_QueryOwnershipCallbackInfo* Data)
{
	if (UEOK_Ecom_QueryOwnership* Node = static_cast<UEOK_Ecom_QueryOwnership*>(Data->ClientData))
	{
		AsyncTask(ENamedThreads::GameThread, [Node, Data]()
		{
			Node->OnCallback.Broadcast(Data);
		});
		Node->SetReadyToDestroy();
#if ENGINE_MAJOR_VERSION == 5
		Node->MarkAsGarbage();
#else
		Node->MarkPendingKill();
#endif
	}
}

void UEOK_Ecom_QueryOwnership::Activate()
{
	Super::Activate();
	if (IOnlineSubsystem* OnlineSub = IOnlineSubsystem::Get("EOK"))
	{
		if (FOnlineSubsystemEOS* EOSRef = static_cast<FOnlineSubsystemEOS*>(OnlineSub))
		{
			EOS_Ecom_QueryOwnershipOptions QueryOwnershipOptions = Var_QueryOwnershipOptions.ToEOS_Ecom_QueryOwnershipOptions();
			EOS_Ecom_QueryOwnership(EOSRef->EcomHandle, &QueryOwnershipOptions, this, &UEOK_Ecom_QueryOwnership::OnQueryOwnershipCallback);
			return;
		}
	}
	UE_LOG(LogEOK, Error, TEXT("Failed to query ownership either OnlineSubsystem is not valid or EOSRef is not valid."));
	OnCallback.Broadcast(FEOK_Ecom_QueryOwnershipCallbackInfo());
	SetReadyToDestroy();
	#if ENGINE_MAJOR_VERSION == 5
	MarkAsGarbage();
#else
	MarkPendingKill();
#endif
}
