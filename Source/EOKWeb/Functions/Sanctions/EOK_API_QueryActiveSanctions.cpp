// Copyright (c) 2024 Betide Studio. All Rights Reserved.


#include "EOK_API_QueryActiveSanctions.h"

UEOK_API_QueryActiveSanctions* UEOK_API_QueryActiveSanctions::QueryActiveSanctions(FString Authorization,
	FString ProductUserId, TArray<FString> Action)
{
	UEOK_API_QueryActiveSanctions* Proxy = NewObject<UEOK_API_QueryActiveSanctions>();
	Proxy->Var_Authorization = Authorization;
	Proxy->Var_ProductUserId = ProductUserId;
	Proxy->Var_Action = Action;
	return Proxy;
}

void UEOK_API_QueryActiveSanctions::Activate()
{
	Super::Activate();
	FString URL = FString::Printf(TEXT("%s/sanctions/v1/productUser/%s/active"), *APIEndpoint, *Var_ProductUserId);
	if(Var_Action.Num() > 0)
	{
		FString ActionString = TEXT("action=");
		for (int i = 0; i < Var_Action.Num(); i++)
		{
			ActionString += Var_Action[i];
			if(i < Var_Action.Num() - 1)
			{
				ActionString += TEXT("&action=");
			}
		}
		URL += FString::Printf(TEXT("?%s"), *ActionString);
	}
	TSharedRef<IHttpRequest, ESPMode::ThreadSafe> HttpRequest = FHttpModule::Get().CreateRequest();
	HttpRequest->SetVerb(TEXT("GET"));
	HttpRequest->SetURL(URL);
	if (Var_Authorization.Contains("Bearer"))
	{
		HttpRequest->SetHeader(TEXT("Authorization"), Var_Authorization);
	}
	else
	{
		HttpRequest->SetHeader(TEXT("Authorization"), FString::Printf(TEXT("Bearer %s"), *Var_Authorization));
	}
	HttpRequest->OnProcessRequestComplete().BindUObject(this, &UEOK_API_QueryActiveSanctions::OnResponseReceived);
	HttpRequest->ProcessRequest();
}
