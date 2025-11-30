// Copyright (C) 2024, All Rights Reserved.

#include "Functions/Ecom/EOSQueryClawbacksAsync.h"
#include "HttpModule.h"
#include "Interfaces/IHttpRequest.h"

UEOSQueryClawbacksAsync* UEOSQueryClawbacksAsync::QueryClawbacks(
	const FString& Authorization,
	const FString& NameSpace,
	const FString& ClawbackDate,
	int32 Count)
{
	UEOSQueryClawbacksAsync* Node = NewObject<UEOSQueryClawbacksAsync>();
	Node->Var_Authorization = Authorization;
	Node->Var_NameSpace = NameSpace;
	Node->Var_ClawbackDate = ClawbackDate;
	Node->Var_Count = Count;
	return Node;
}

void UEOSQueryClawbacksAsync::Activate()
{
	Super::Activate();

	FString URL = FString::Printf(TEXT("%s/epic/ecom/v3/namespace/%s/entitlements/clawbacks?clawbackDate=%s&count=%d"), 
		*APIEndpoint, *Var_NameSpace, *Var_ClawbackDate, Var_Count);

	TSharedRef<IHttpRequest, ESPMode::ThreadSafe> HttpRequest = FHttpModule::Get().CreateRequest();
	HttpRequest->SetVerb(TEXT("GET"));
	HttpRequest->SetURL(URL);
	HttpRequest->SetHeader(TEXT("Content-Type"), TEXT("application/json"));
	
	FString AuthHeader = Var_Authorization.Contains(TEXT("Bearer")) 
		? Var_Authorization 
		: FString::Printf(TEXT("Bearer %s"), *Var_Authorization);
	HttpRequest->SetHeader(TEXT("Authorization"), AuthHeader);

	HttpRequest->OnProcessRequestComplete().BindUObject(this, &UEOSQueryClawbacksAsync::OnResponseReceived);
	HttpRequest->ProcessRequest();
}

