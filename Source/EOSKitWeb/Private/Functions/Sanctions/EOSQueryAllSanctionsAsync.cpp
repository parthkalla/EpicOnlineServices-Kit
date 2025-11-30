// Copyright (C) 2024, All Rights Reserved.

#include "Functions/Sanctions/EOSQueryAllSanctionsAsync.h"
#include "HttpModule.h"
#include "Interfaces/IHttpRequest.h"

UEOSQueryAllSanctionsAsync* UEOSQueryAllSanctionsAsync::QueryAllSanctions(
	const FString& Authorization,
	const FString& DeploymentId,
	int32 Limit,
	int32 Offset)
{
	UEOSQueryAllSanctionsAsync* Node = NewObject<UEOSQueryAllSanctionsAsync>();
	Node->Var_Authorization = Authorization;
	Node->Var_DeploymentId = DeploymentId;
	Node->Var_Limit = Limit;
	Node->Var_Offset = Offset;
	return Node;
}

void UEOSQueryAllSanctionsAsync::Activate()
{
	Super::Activate();

	FString URL = FString::Printf(TEXT("%s/sanctions/v1/%s/sanctions?limit=%d&offset=%d"), 
		*APIEndpoint, *Var_DeploymentId, Var_Limit, Var_Offset);

	TSharedRef<IHttpRequest, ESPMode::ThreadSafe> HttpRequest = FHttpModule::Get().CreateRequest();
	HttpRequest->SetVerb(TEXT("GET"));
	HttpRequest->SetURL(URL);
	
	FString AuthHeader = Var_Authorization.Contains(TEXT("Bearer")) 
		? Var_Authorization 
		: FString::Printf(TEXT("Bearer %s"), *Var_Authorization);
	HttpRequest->SetHeader(TEXT("Authorization"), AuthHeader);

	HttpRequest->OnProcessRequestComplete().BindUObject(this, &UEOSQueryAllSanctionsAsync::OnResponseReceived);
	HttpRequest->ProcessRequest();
}

