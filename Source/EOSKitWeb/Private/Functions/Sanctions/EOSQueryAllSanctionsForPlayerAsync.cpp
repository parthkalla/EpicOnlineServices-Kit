// Copyright (C) 2024, All Rights Reserved.

#include "Functions/Sanctions/EOSQueryAllSanctionsForPlayerAsync.h"
#include "HttpModule.h"
#include "Interfaces/IHttpRequest.h"

UEOSQueryAllSanctionsForPlayerAsync* UEOSQueryAllSanctionsForPlayerAsync::QueryAllSanctionsForPlayer(
	const FString& Authorization,
	const FString& DeploymentId,
	const FString& ProductUserId,
	int32 Limit,
	int32 Offset)
{
	UEOSQueryAllSanctionsForPlayerAsync* Node = NewObject<UEOSQueryAllSanctionsForPlayerAsync>();
	Node->Var_Authorization = Authorization;
	Node->Var_DeploymentId = DeploymentId;
	Node->Var_ProductUserId = ProductUserId;
	Node->Var_Limit = Limit;
	Node->Var_Offset = Offset;
	return Node;
}

void UEOSQueryAllSanctionsForPlayerAsync::Activate()
{
	Super::Activate();

	FString URL = FString::Printf(TEXT("%s/sanctions/v1/%s/users/%s?limit=%d&offset=%d"), 
		*APIEndpoint, *Var_DeploymentId, *Var_ProductUserId, Var_Limit, Var_Offset);

	TSharedRef<IHttpRequest, ESPMode::ThreadSafe> HttpRequest = FHttpModule::Get().CreateRequest();
	HttpRequest->SetVerb(TEXT("GET"));
	HttpRequest->SetURL(URL);
	
	FString AuthHeader = Var_Authorization.Contains(TEXT("Bearer")) 
		? Var_Authorization 
		: FString::Printf(TEXT("Bearer %s"), *Var_Authorization);
	HttpRequest->SetHeader(TEXT("Authorization"), AuthHeader);

	HttpRequest->OnProcessRequestComplete().BindUObject(this, &UEOSQueryAllSanctionsForPlayerAsync::OnResponseReceived);
	HttpRequest->ProcessRequest();
}

