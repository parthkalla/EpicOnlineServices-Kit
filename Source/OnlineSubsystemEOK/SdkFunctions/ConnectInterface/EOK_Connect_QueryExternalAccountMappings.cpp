// Copyright (c) 2023 Betide Studio. All Rights Reserved.


#include "EOK_Connect_QueryExternalAccountMappings.h"

#include "EOK_ConnectSubsystem.h"
#include "OnlineSubsystemEOS.h"
#include "Async/Async.h"

UEOK_Connect_QueryExternalAccountMappings* UEOK_Connect_QueryExternalAccountMappings::EOK_Connect_QueryExternalAccountMappings(FEOK_ProductUserId ProductUserId,
                                                                                                                               TEnumAsByte<EEOK_EExternalAccountType> AccountType, const TArray<FString>& ExternalAccountIds)
{
	UEOK_Connect_QueryExternalAccountMappings* Node = NewObject<UEOK_Connect_QueryExternalAccountMappings>();
	Node->Var_ProductUserId = ProductUserId;
	Node->Var_AccountType = AccountType;
	Node->Var_ExternalAccountIds = ExternalAccountIds;
	return Node;
}

void UEOK_Connect_QueryExternalAccountMappings::OnQueryExternalAccountMappingsCallback(
	const EOS_Connect_QueryExternalAccountMappingsCallbackInfo* Data)
{
	if (UEOK_Connect_QueryExternalAccountMappings* Proxy = static_cast<UEOK_Connect_QueryExternalAccountMappings*>(Data->ClientData))
	{
		AsyncTask(ENamedThreads::GameThread, [Proxy, Data]()
		{
			Proxy->OnCallback.Broadcast(Proxy->Var_ProductUserId, static_cast<EEOK_Result>(Data->ResultCode));
		});
		Proxy->SetReadyToDestroy();
#if ENGINE_MAJOR_VERSION == 5
		Proxy->MarkAsGarbage();
#else
		Proxy->MarkPendingKill();
#endif
	}
}

void UEOK_Connect_QueryExternalAccountMappings::Activate()
{
	Super::Activate();
	if(	IOnlineSubsystem* OnlineSub = IOnlineSubsystem::Get("EOK"))
	{
		if (FOnlineSubsystemEOS* EOSRef = static_cast<FOnlineSubsystemEOS*>(OnlineSub))
		{
			EOS_Connect_QueryExternalAccountMappingsOptions QueryExternalAccountMappingsOptions = { };
			QueryExternalAccountMappingsOptions.ApiVersion = EOS_CONNECT_QUERYEXTERNALACCOUNTMAPPINGS_API_LATEST;
			QueryExternalAccountMappingsOptions.LocalUserId = Var_ProductUserId.GetValueAsEosType();
			QueryExternalAccountMappingsOptions.AccountIdType = static_cast<EOS_EExternalAccountType>(Var_AccountType.GetValue());
			QueryExternalAccountMappingsOptions.ExternalAccountIdCount = Var_ExternalAccountIds.Num();
			const char** ExternalAccountIds = new const char*[Var_ExternalAccountIds.Num()];
			for (int i = 0; i < Var_ExternalAccountIds.Num(); i++)
			{
				ExternalAccountIds[i] = TCHAR_TO_ANSI(*Var_ExternalAccountIds[i]);
			}
			QueryExternalAccountMappingsOptions.ExternalAccountIds = ExternalAccountIds;
			EOS_Connect_QueryExternalAccountMappings(EOSRef->ConnectHandle, &QueryExternalAccountMappingsOptions, this, &UEOK_Connect_QueryExternalAccountMappings::OnQueryExternalAccountMappingsCallback);
			return;
		}
	}
	UE_LOG(LogEOK, Error, TEXT("Failed to query external account mappings either OnlineSubsystem is not valid or EOSRef is not valid."));
	OnCallback.Broadcast(FEOK_ProductUserId(), EEOK_Result::EOS_ServiceFailure);
	SetReadyToDestroy();
	#if ENGINE_MAJOR_VERSION == 5
	MarkAsGarbage();
#else
	MarkPendingKill();
#endif	
}

