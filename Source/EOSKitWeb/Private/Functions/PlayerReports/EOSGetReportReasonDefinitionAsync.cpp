// Copyright (C) 2024, All Rights Reserved.

#include "Functions/PlayerReports/EOSGetReportReasonDefinitionAsync.h"
#include "HttpModule.h"
#include "Interfaces/IHttpRequest.h"

UEOSGetReportReasonDefinitionAsync* UEOSGetReportReasonDefinitionAsync::GetReportReasonDefinition(
	const FString& Authorization)
{
	UEOSGetReportReasonDefinitionAsync* Node = NewObject<UEOSGetReportReasonDefinitionAsync>();
	Node->Var_Authorization = Authorization;
	return Node;
}

void UEOSGetReportReasonDefinitionAsync::Activate()
{
	Super::Activate();

	FString URL = FString::Printf(TEXT("%s/player-reports/v1/report/reason/definition"), *APIEndpoint);

	TSharedRef<IHttpRequest, ESPMode::ThreadSafe> HttpRequest = FHttpModule::Get().CreateRequest();
	HttpRequest->SetVerb(TEXT("GET"));
	HttpRequest->SetURL(URL);
	HttpRequest->SetHeader(TEXT("Content-Type"), TEXT("application/json"));
	
	FString AuthHeader = Var_Authorization.Contains(TEXT("Bearer")) 
		? Var_Authorization 
		: FString::Printf(TEXT("Bearer %s"), *Var_Authorization);
	HttpRequest->SetHeader(TEXT("Authorization"), AuthHeader);

	HttpRequest->OnProcessRequestComplete().BindUObject(this, &UEOSGetReportReasonDefinitionAsync::OnResponseReceived);
	HttpRequest->ProcessRequest();
}

