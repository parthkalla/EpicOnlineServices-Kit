// Copyright (C) 2024, All Rights Reserved.

#include "Functions/Sanctions/EOSQuerySanctionAppealsAsync.h"
#include "HttpModule.h"
#include "Interfaces/IHttpRequest.h"

UEOSQuerySanctionAppealsAsync* UEOSQuerySanctionAppealsAsync::QuerySanctionAppeals(
	const FString& Authorization,
	const FString& DeploymentId,
	const FString& ReferenceId,
	const FString& Status,
	const FString& ProductUserId,
	int32 Limit,
	int32 Offset)
{
	UEOSQuerySanctionAppealsAsync* Node = NewObject<UEOSQuerySanctionAppealsAsync>();
	Node->Var_Authorization = Authorization;
	Node->Var_DeploymentId = DeploymentId;
	Node->Var_ReferenceId = ReferenceId;
	Node->Var_Status = Status;
	Node->Var_ProductUserId = ProductUserId;
	Node->Var_Limit = Limit;
	Node->Var_Offset = Offset;
	return Node;
}

void UEOSQuerySanctionAppealsAsync::Activate()
{
	Super::Activate();

	FString URL = FString::Printf(TEXT("%s/sanctions/v1/%s/appeals"), *APIEndpoint, *Var_DeploymentId);
	
	bool bFirstParam = true;
	if (!Var_Status.IsEmpty())
	{
		URL += TEXT("?status=") + Var_Status;
		bFirstParam = false;
	}
	if (!Var_ProductUserId.IsEmpty())
	{
		URL += (bFirstParam ? TEXT("?") : TEXT("&")) + FString::Printf(TEXT("productUserId=%s"), *Var_ProductUserId);
		bFirstParam = false;
	}
	if (!Var_ReferenceId.IsEmpty())
	{
		URL += (bFirstParam ? TEXT("?") : TEXT("&")) + FString::Printf(TEXT("referenceId=%s"), *Var_ReferenceId);
		bFirstParam = false;
	}
	if (Var_Limit > 0)
	{
		URL += (bFirstParam ? TEXT("?") : TEXT("&")) + FString::Printf(TEXT("limit=%d"), Var_Limit);
		bFirstParam = false;
	}
	if (Var_Offset > 0)
	{
		URL += (bFirstParam ? TEXT("?") : TEXT("&")) + FString::Printf(TEXT("offset=%d"), Var_Offset);
	}

	TSharedRef<IHttpRequest, ESPMode::ThreadSafe> HttpRequest = FHttpModule::Get().CreateRequest();
	HttpRequest->SetVerb(TEXT("GET"));
	HttpRequest->SetURL(URL);
	HttpRequest->SetHeader(TEXT("Content-Type"), TEXT("application/json"));
	
	FString AuthHeader = Var_Authorization.Contains(TEXT("Bearer")) 
		? Var_Authorization 
		: FString::Printf(TEXT("Bearer %s"), *Var_Authorization);
	HttpRequest->SetHeader(TEXT("Authorization"), AuthHeader);

	HttpRequest->OnProcessRequestComplete().BindUObject(this, &UEOSQuerySanctionAppealsAsync::OnResponseReceived);
	HttpRequest->ProcessRequest();
}

