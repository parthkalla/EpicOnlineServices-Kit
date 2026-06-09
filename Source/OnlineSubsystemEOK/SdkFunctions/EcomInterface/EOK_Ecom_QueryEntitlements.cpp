// Copyright (c) 2023 Betide Studio. All Rights Reserved.


#include "EOK_Ecom_QueryEntitlements.h"

#include "OnlineSubsystemEOS.h"
#include "Async/Async.h"
#include "OnlineSubsystemEOK/SdkFunctions/ConnectInterface/EOK_ConnectSubsystem.h"

UEOK_Ecom_QueryEntitlements* UEOK_Ecom_QueryEntitlements::EOK_Ecom_QueryEntitlements(
	FEOK_Ecom_QueryEntitlementsOptions QueryEntitlementsOptions)
{
	UEOK_Ecom_QueryEntitlements* Node = NewObject<UEOK_Ecom_QueryEntitlements>();
	Node->Local_QueryEntitlementsOptions = QueryEntitlementsOptions;
	return Node;
}

void UEOK_Ecom_QueryEntitlements::OnQueryEntitlementsCallback(const EOS_Ecom_QueryEntitlementsCallbackInfo* Data)
{
	if (UEOK_Ecom_QueryEntitlements* Node = static_cast<UEOK_Ecom_QueryEntitlements*>(Data->ClientData))
	{
		AsyncTask(ENamedThreads::GameThread, [Node, Data]()
		{
			Node->OnCallback.Broadcast(Data->LocalUserId, static_cast<EEOK_Result>(Data->ResultCode));
		});
		Node->SetReadyToDestroy();
#if ENGINE_MAJOR_VERSION == 5
		Node->MarkAsGarbage();
#else
		Node->MarkPendingKill();
#endif
	}
}

void UEOK_Ecom_QueryEntitlements::Activate()
{
	Super::Activate();
	if(	IOnlineSubsystem* OnlineSub = IOnlineSubsystem::Get("EOK"))
	{
		if (FOnlineSubsystemEOS* EOSRef = static_cast<FOnlineSubsystemEOS*>(OnlineSub))
		{
			EOS_Ecom_QueryEntitlementsOptions QueryEntitlementsOptions = Local_QueryEntitlementsOptions.ToEOS_Ecom_QueryEntitlementsOptions();
			EOS_Ecom_QueryEntitlements(EOSRef->EcomHandle, &QueryEntitlementsOptions, this, &UEOK_Ecom_QueryEntitlements::OnQueryEntitlementsCallback);
			return;
		}
	}
	UE_LOG(LogEOK, Error, TEXT("Failed to query entitlements either OnlineSubsystem is not valid or EOSRef is not valid."));
	OnCallback.Broadcast(FEOK_EpicAccountId(), EEOK_Result::EOS_ServiceFailure);
	SetReadyToDestroy();
	#if ENGINE_MAJOR_VERSION == 5
	MarkAsGarbage();
#else
	MarkPendingKill();
#endif
}
