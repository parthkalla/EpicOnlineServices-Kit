// Copyright (C) 2024, All Rights Reserved.

#include "Functions/Connect/EOSQueryExternalAccountsAsync.h"
#include "HttpModule.h"
#include "Interfaces/IHttpRequest.h"

UEOSQueryExternalAccountsAsync* UEOSQueryExternalAccountsAsync::QueryExternalAccounts(
	const FString& Authorization,
	const TArray<FString>& AccountIds,
	const FString& IdentityProviderId,
	const FString& Environment)
{
	UEOSQueryExternalAccountsAsync* Node = NewObject<UEOSQueryExternalAccountsAsync>();
	Node->Var_Authorization = Authorization;
	Node->Var_AccountIds = AccountIds;
	Node->Var_IdentityProviderId = IdentityProviderId;
	Node->Var_Environment = Environment;
	return Node;
}

void UEOSQueryExternalAccountsAsync::Activate()
{
	Super::Activate();

	FString URL = FString::Printf(TEXT("%s/user/v1/accounts"), *APIEndpoint);
	
	// Build query string
	FString QueryString = TEXT("?");
	bool bFirstParam = true;

	if (Var_AccountIds.Num() > 0)
	{
		for (const FString& AccountId : Var_AccountIds)
		{
			if (!bFirstParam)
			{
				QueryString += TEXT("&");
			}
			QueryString += FString::Printf(TEXT("accountId=%s"), *AccountId);
			bFirstParam = false;
		}
	}

	if (!Var_IdentityProviderId.IsEmpty())
	{
		if (!bFirstParam)
		{
			QueryString += TEXT("&");
		}
		QueryString += FString::Printf(TEXT("identityProviderId=%s"), *Var_IdentityProviderId);
		bFirstParam = false;
	}

	if (!Var_Environment.IsEmpty())
	{
		if (!bFirstParam)
		{
			QueryString += TEXT("&");
		}
		QueryString += FString::Printf(TEXT("environment=%s"), *Var_Environment);
	}

	URL += QueryString;

	TSharedRef<IHttpRequest, ESPMode::ThreadSafe> HttpRequest = FHttpModule::Get().CreateRequest();
	HttpRequest->SetVerb(TEXT("GET"));
	HttpRequest->SetURL(URL);
	HttpRequest->SetHeader(TEXT("Content-Type"), TEXT("application/json"));
	
	FString AuthHeader = Var_Authorization.Contains(TEXT("Bearer")) 
		? Var_Authorization 
		: FString::Printf(TEXT("Bearer %s"), *Var_Authorization);
	HttpRequest->SetHeader(TEXT("Authorization"), AuthHeader);

	HttpRequest->OnProcessRequestComplete().BindUObject(this, &UEOSQueryExternalAccountsAsync::OnResponseReceived);
	HttpRequest->ProcessRequest();
}

