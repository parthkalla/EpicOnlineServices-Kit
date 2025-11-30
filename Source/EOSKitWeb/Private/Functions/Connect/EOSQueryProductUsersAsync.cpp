// Copyright (C) 2024, All Rights Reserved.

#include "Functions/Connect/EOSQueryProductUsersAsync.h"
#include "HttpModule.h"
#include "Interfaces/IHttpRequest.h"

UEOSQueryProductUsersAsync* UEOSQueryProductUsersAsync::QueryProductUsers(
	const FString& Authorization,
	const TArray<FString>& ProductUserIds)
{
	UEOSQueryProductUsersAsync* Node = NewObject<UEOSQueryProductUsersAsync>();
	Node->Var_Authorization = Authorization;
	Node->Var_ProductUserIds = ProductUserIds;
	return Node;
}

void UEOSQueryProductUsersAsync::Activate()
{
	Super::Activate();

	FString URL = FString::Printf(TEXT("%s/user/v1/product-users"), *APIEndpoint);
	
	// Build query string
	FString QueryString = TEXT("?");
	bool bFirstParam = true;

	if (Var_ProductUserIds.Num() > 0)
	{
		for (const FString& ProductUserId : Var_ProductUserIds)
		{
			if (!bFirstParam)
			{
				QueryString += TEXT("&");
			}
			QueryString += FString::Printf(TEXT("productUserId=%s"), *ProductUserId);
			bFirstParam = false;
		}
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

	HttpRequest->OnProcessRequestComplete().BindUObject(this, &UEOSQueryProductUsersAsync::OnResponseReceived);
	HttpRequest->ProcessRequest();
}

