// Copyright (c) 2023 Betide Studio. All Rights Reserved.


#include "EOK_API_QueryAntiCheatServiceStatus.h"

UEOK_API_QueryAntiCheatServiceStatus* UEOK_API_QueryAntiCheatServiceStatus::QueryAntiCheatServiceStatus(
	FString Authorization, FString DeploymentId)
{
	UEOK_API_QueryAntiCheatServiceStatus* Node = NewObject<UEOK_API_QueryAntiCheatServiceStatus>();
	Node->Var_Authorization = Authorization;
	Node->Var_DeploymentId = DeploymentId;
	return Node;
}

void UEOK_API_QueryAntiCheatServiceStatus::Activate()
{
	Super::Activate();
	FString URL = FString::Printf(TEXT("%s/anticheat/v1/%s/status"), *APIEndpoint, *Var_DeploymentId);
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
	HttpRequest->OnProcessRequestComplete().BindUObject(this, &UEOK_API_QueryAntiCheatServiceStatus::OnResponseReceived);
	HttpRequest->ProcessRequest();
}
