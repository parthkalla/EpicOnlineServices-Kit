// Copyright (c) 2023 Betide Studio. All Rights Reserved.


#include "EOK_API_GetReportReasonDefinition.h"

UEOK_API_GetReportReasonDefinition* UEOK_API_GetReportReasonDefinition::GetReportReasonDefinition(FString Authorization)
{
	UEOK_API_GetReportReasonDefinition* Proxy = NewObject<UEOK_API_GetReportReasonDefinition>();
	Proxy->Var_Authorization = Authorization;
	return Proxy;
}

void UEOK_API_GetReportReasonDefinition::Activate()
{
	Super::Activate();
	FString URL = FString::Printf(TEXT("%s/player-reports/v1/report/reason/definition"), *APIEndpoint);
	TSharedRef<IHttpRequest, ESPMode::ThreadSafe> HttpRequest = FHttpModule::Get().CreateRequest();
	HttpRequest->SetVerb(TEXT("GET"));
	HttpRequest->SetURL(URL);
	HttpRequest->SetHeader(TEXT("Content-Type"), TEXT("application/json"));
	if (Var_Authorization.Contains("Bearer"))
	{
		HttpRequest->SetHeader(TEXT("Authorization"), Var_Authorization);
	}
	else
	{
		HttpRequest->SetHeader(TEXT("Authorization"), FString::Printf(TEXT("Bearer %s"), *Var_Authorization));
	}
	HttpRequest->OnProcessRequestComplete().BindUObject(this, &UEOK_API_GetReportReasonDefinition::OnResponseReceived);
	HttpRequest->ProcessRequest();
}
