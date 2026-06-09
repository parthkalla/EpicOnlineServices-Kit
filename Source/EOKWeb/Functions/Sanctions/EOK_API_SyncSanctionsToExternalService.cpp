// Copyright (c) 2023 Betide Studio. All Rights Reserved.


#include "EOK_API_SyncSanctionsToExternalService.h"

UEOK_API_SyncSanctionsToExternalService* UEOK_API_SyncSanctionsToExternalService::SyncSanctionsToExternalService(
	FString Authorization, FString LastLogId)
{
	UEOK_API_SyncSanctionsToExternalService* Proxy = NewObject<UEOK_API_SyncSanctionsToExternalService>();
	Proxy->Var_Authorization = Authorization;
	Proxy->Var_LastLogId = LastLogId;
	return Proxy;
}

void UEOK_API_SyncSanctionsToExternalService::Activate()
{
	Super::Activate();
	FString URL = FString::Printf(TEXT("%s/sanctions/v1/sync"), *APIEndpoint);
	if (!Var_LastLogId.IsEmpty())
	{
		URL.Append(FString::Printf(TEXT("?lastLogId=%s"), *Var_LastLogId));
	}
	TSharedRef<IHttpRequest, ESPMode::ThreadSafe> HttpRequest = FHttpModule::Get().CreateRequest();
	HttpRequest->SetVerb(TEXT("GET"));
	HttpRequest->SetURL(URL);
	if (Var_Authorization.Contains("Bearer"))
	{
		HttpRequest->SetHeader(TEXT("Authorization"), Var_Authorization);
	}
	else
	{
		HttpRequest->SetHeader(TEXT("Authorization"), FString::Printf(TEXT("Bearer %s"), *Var_Authorization));
	}
	HttpRequest->OnProcessRequestComplete().BindUObject(this, &UEOK_API_SyncSanctionsToExternalService::OnResponseReceived);
	HttpRequest->ProcessRequest();
}
