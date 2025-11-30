// Copyright (C) 2024, All Rights Reserved.

#include "Functions/AntiCheat/EOSQueryAntiCheatServiceStatusAsync.h"
#include "HttpModule.h"
#include "Interfaces/IHttpRequest.h"

UEOSQueryAntiCheatServiceStatusAsync* UEOSQueryAntiCheatServiceStatusAsync::QueryAntiCheatServiceStatus(
	const FString& Authorization,
	const FString& DeploymentId)
{
	UEOSQueryAntiCheatServiceStatusAsync* Node = NewObject<UEOSQueryAntiCheatServiceStatusAsync>();
	Node->Var_Authorization = Authorization;
	Node->Var_DeploymentId = DeploymentId;
	return Node;
}

void UEOSQueryAntiCheatServiceStatusAsync::Activate()
{
	Super::Activate();

	FString URL = FString::Printf(TEXT("%s/anticheat/v1/%s/status"), *APIEndpoint, *Var_DeploymentId);
	
	TSharedRef<IHttpRequest, ESPMode::ThreadSafe> HttpRequest = FHttpModule::Get().CreateRequest();
	HttpRequest->SetVerb(TEXT("GET"));
	HttpRequest->SetURL(URL);
	HttpRequest->SetHeader(TEXT("Content-Type"), TEXT("application/json"));
	
	FString AuthHeader = Var_Authorization.Contains(TEXT("Bearer")) 
		? Var_Authorization 
		: FString::Printf(TEXT("Bearer %s"), *Var_Authorization);
	HttpRequest->SetHeader(TEXT("Authorization"), AuthHeader);

	HttpRequest->OnProcessRequestComplete().BindUObject(this, &UEOSQueryAntiCheatServiceStatusAsync::OnResponseReceived);
	HttpRequest->ProcessRequest();
}

