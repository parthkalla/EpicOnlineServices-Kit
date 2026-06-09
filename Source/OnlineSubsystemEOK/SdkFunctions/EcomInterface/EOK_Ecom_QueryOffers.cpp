// Copyright (c) 2023 Betide Studio. All Rights Reserved.


#include "EOK_Ecom_QueryOffers.h"

#include "OnlineSubsystemEOS.h"
#include "Async/Async.h"
#include "OnlineSubsystemEOK/SdkFunctions/ConnectInterface/EOK_ConnectSubsystem.h"

UEOK_Ecom_QueryOffers* UEOK_Ecom_QueryOffers::EOK_Ecom_QueryOffers(FEOK_EpicAccountId LocalUserId,
	FString OverrideCatalogNamespace)
{
	UEOK_Ecom_QueryOffers* Node = NewObject<UEOK_Ecom_QueryOffers>();
	Node->Var_LocalUserId = LocalUserId;
	Node->Var_OverrideCatalogNamespace = OverrideCatalogNamespace;
	return Node;
}

void UEOK_Ecom_QueryOffers::OnQueryOffersCallback(const EOS_Ecom_QueryOffersCallbackInfo* Data)
{
	if (UEOK_Ecom_QueryOffers* Node = static_cast<UEOK_Ecom_QueryOffers*>(Data->ClientData))
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
void UEOK_Ecom_QueryOffers::Activate()
{
	Super::Activate();
	if (IOnlineSubsystem* OnlineSub = IOnlineSubsystem::Get("EOK"))
	{
		if (FOnlineSubsystemEOS* EOSRef = static_cast<FOnlineSubsystemEOS*>(OnlineSub))
		{
			EOS_Ecom_QueryOffersOptions QueryOffersOptions;
			QueryOffersOptions.ApiVersion = EOS_ECOM_QUERYOFFERS_API_LATEST;
			QueryOffersOptions.LocalUserId = Var_LocalUserId.GetValueAsEosType();
			if(Var_OverrideCatalogNamespace.IsEmpty())
			{
				QueryOffersOptions.OverrideCatalogNamespace = nullptr;
			}
			else
			{
				QueryOffersOptions.OverrideCatalogNamespace = TCHAR_TO_ANSI(*Var_OverrideCatalogNamespace);
			}			
			EOS_Ecom_QueryOffers(EOSRef->EcomHandle, &QueryOffersOptions, this, &UEOK_Ecom_QueryOffers::OnQueryOffersCallback);
			return;
		}
	}
	UE_LOG(LogEOK, Error, TEXT("Failed to query offers either OnlineSubsystem is not valid or EOSRef is not valid."));
	OnCallback.Broadcast(FEOK_EpicAccountId(), EEOK_Result::EOS_ServiceFailure);
	SetReadyToDestroy();
	#if ENGINE_MAJOR_VERSION == 5
	MarkAsGarbage();
#else
	MarkPendingKill();
#endif
}
