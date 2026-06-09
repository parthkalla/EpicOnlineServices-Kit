// Copyright (c) 2023 Betide Studio. All Rights Reserved.


#include "EOK_Ecom_Checkout.h"

#include "Async/Async.h"
#include "OnlineSubsystemEOK/SdkFunctions/ConnectInterface/EOK_ConnectSubsystem.h"


UEOK_Ecom_Checkout* UEOK_Ecom_Checkout::EOK_Ecom_Checkout(FEOK_Ecom_CheckoutOptions CheckoutOptions)
{
	UEOK_Ecom_Checkout* Node = NewObject<UEOK_Ecom_Checkout>();
	Node->Local_CheckoutOptions = CheckoutOptions;
	return Node;
}

void UEOK_Ecom_Checkout::OnCheckoutCallback(const EOS_Ecom_CheckoutCallbackInfo* Data)
{
	if(UEOK_Ecom_Checkout* Node = static_cast<UEOK_Ecom_Checkout*>(Data->ClientData))
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

void UEOK_Ecom_Checkout::Activate()
{
	Super::Activate();
	if(	IOnlineSubsystem* OnlineSub = IOnlineSubsystem::Get("EOK"))
	{
		if (FOnlineSubsystemEOS* EOSRef = static_cast<FOnlineSubsystemEOS*>(OnlineSub))
		{
			EOS_Ecom_CheckoutOptions CheckoutOptions = Local_CheckoutOptions.ToEOS_Ecom_CheckoutOptions();
			EOS_Ecom_Checkout(EOSRef->EcomHandle, &CheckoutOptions, this, &UEOK_Ecom_Checkout::OnCheckoutCallback);
			return;
		}
	}
	UE_LOG(LogEOK, Error, TEXT("Failed to checkout item either OnlineSubsystem is not valid or EOSRef is not valid."));
	OnCallback.Broadcast(FEOK_Ecom_CheckoutCallbackInfo());
	SetReadyToDestroy();
	#if ENGINE_MAJOR_VERSION == 5
	MarkAsGarbage();
#else
	MarkPendingKill();
#endif
}
