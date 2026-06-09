// Copyright (c) 2023 Betide Studio. All Rights Reserved.


#include "EOK_Connect_TransferDeviceIdAccount.h"

#include "EOK_ConnectSubsystem.h"
#include "OnlineSubsystemEOS.h"
#include "Async/Async.h"

UEOK_Connect_TransferDeviceIdAccount* UEOK_Connect_TransferDeviceIdAccount::EOK_Connect_TransferDeviceIdAccount(
	FEOK_ProductUserId PrimaryLocalUserId, FEOK_ProductUserId LocalUserId, FEOK_ProductUserId ProductUserIdToPreserve)
{
	UEOK_Connect_TransferDeviceIdAccount* Node = NewObject<UEOK_Connect_TransferDeviceIdAccount>();
	Node->Var_PrimaryLocalUserId = PrimaryLocalUserId;
	Node->Var_LocalUserId = LocalUserId;
	Node->Var_ProductUserIdToPreserve = ProductUserIdToPreserve;
	return Node;
}

void UEOK_Connect_TransferDeviceIdAccount::OnTransferDeviceIdAccountCallback(
	const EOS_Connect_TransferDeviceIdAccountCallbackInfo* Data)
{
	if (UEOK_Connect_TransferDeviceIdAccount* Proxy = static_cast<UEOK_Connect_TransferDeviceIdAccount*>(Data->ClientData))
	{
		AsyncTask(ENamedThreads::GameThread, [Proxy, Data]()
		{
			Proxy->OnCallback.Broadcast(FEOK_ProductUserId(Data->LocalUserId), static_cast<EEOK_Result>(Data->ResultCode));
		});
		Proxy->SetReadyToDestroy();
#if ENGINE_MAJOR_VERSION == 5
		Proxy->MarkAsGarbage();
#else
		Proxy->MarkPendingKill();
#endif
	}
}

void UEOK_Connect_TransferDeviceIdAccount::Activate()
{
	Super::Activate();
	if (IOnlineSubsystem* OnlineSub = IOnlineSubsystem::Get("EOK"))
	{
		if (FOnlineSubsystemEOS* EOSRef = static_cast<FOnlineSubsystemEOS*>(OnlineSub))
		{
			EOS_Connect_TransferDeviceIdAccountOptions TransferDeviceIdAccountOptions = {};
			TransferDeviceIdAccountOptions.ApiVersion = EOS_CONNECT_TRANSFERDEVICEIDACCOUNT_API_LATEST;
			TransferDeviceIdAccountOptions.LocalDeviceUserId = Var_LocalUserId.GetValueAsEosType();
			TransferDeviceIdAccountOptions.ProductUserIdToPreserve = Var_ProductUserIdToPreserve.GetValueAsEosType();
			TransferDeviceIdAccountOptions.PrimaryLocalUserId = Var_PrimaryLocalUserId.GetValueAsEosType();
			EOS_Connect_TransferDeviceIdAccount(EOSRef->ConnectHandle, &TransferDeviceIdAccountOptions, this, &UEOK_Connect_TransferDeviceIdAccount::OnTransferDeviceIdAccountCallback);
		}
	}
	UE_LOG(LogEOK, Error, TEXT("Failed to transfer device id account either OnlineSubsystem is not valid or EOSRef is not valid."));
	OnCallback.Broadcast(FEOK_ProductUserId(), EEOK_Result::EOS_ServiceFailure);
}
