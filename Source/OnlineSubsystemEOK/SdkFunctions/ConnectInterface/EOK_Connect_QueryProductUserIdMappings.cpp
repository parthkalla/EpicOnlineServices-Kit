// Copyright (c) 2023 Betide Studio. All Rights Reserved.


#include "EOK_Connect_QueryProductUserIdMappings.h"

#include "EOK_ConnectSubsystem.h"
#include "OnlineSubsystemEOS.h"
#include "Async/Async.h"

UEOK_Connect_QueryProductUserIdMappings* UEOK_Connect_QueryProductUserIdMappings::
EOK_Connect_QueryProductUserIdMappings(FEOK_ProductUserId LocalUserId,
	const TArray<FEOK_ProductUserId>& TargetProductUserIds)
{
	UEOK_Connect_QueryProductUserIdMappings* Node = NewObject<UEOK_Connect_QueryProductUserIdMappings>();
	Node->Var_LocalUserId = LocalUserId;
	Node->Var_TargetProductUserIds = TargetProductUserIds;
	return Node;
}

void UEOK_Connect_QueryProductUserIdMappings::OnQueryProductUserIdMappingsCallback(
	const EOS_Connect_QueryProductUserIdMappingsCallbackInfo* Data)
{
	if (UEOK_Connect_QueryProductUserIdMappings* Proxy = static_cast<UEOK_Connect_QueryProductUserIdMappings*>(Data->ClientData))
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

void UEOK_Connect_QueryProductUserIdMappings::Activate()
{
	Super::Activate();
	if (IOnlineSubsystem* OnlineSub = IOnlineSubsystem::Get("EOK"))
	{
		if (FOnlineSubsystemEOS* EOSRef = static_cast<FOnlineSubsystemEOS*>(OnlineSub))
		{
			EOS_Connect_QueryProductUserIdMappingsOptions QueryProductUserIdMappingsOptions = {};
			QueryProductUserIdMappingsOptions.ApiVersion = EOS_CONNECT_QUERYPRODUCTUSERIDMAPPINGS_API_LATEST;
			QueryProductUserIdMappingsOptions.LocalUserId = Var_LocalUserId.GetValueAsEosType();
			QueryProductUserIdMappingsOptions.ProductUserIdCount = Var_TargetProductUserIds.Num();
			EOS_ProductUserId* TargetProductUserIds = new EOS_ProductUserId[Var_TargetProductUserIds.Num()];
			for (int i = 0; i < Var_TargetProductUserIds.Num(); i++)
			{
				TargetProductUserIds[i] = Var_TargetProductUserIds[i].GetValueAsEosType();
			}
			QueryProductUserIdMappingsOptions.ProductUserIds = TargetProductUserIds;
			EOS_Connect_QueryProductUserIdMappings(EOSRef->ConnectHandle, &QueryProductUserIdMappingsOptions, this, &UEOK_Connect_QueryProductUserIdMappings::OnQueryProductUserIdMappingsCallback);
			return;
		}
	}
	UE_LOG(LogEOK, Error, TEXT("Failed to query product user id mappings either OnlineSubsystem is not valid or EOSRef is not valid."));
	OnCallback.Broadcast(FEOK_ProductUserId(), EEOK_Result::EOS_ServiceFailure);
	SetReadyToDestroy();
#if ENGINE_MAJOR_VERSION == 5
	MarkAsGarbage();
#else
	MarkPendingKill();
#endif
}
