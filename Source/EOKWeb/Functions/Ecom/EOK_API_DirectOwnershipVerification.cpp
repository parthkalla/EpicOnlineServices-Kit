// Copyright (c) 2023 Betide Studio. All Rights Reserved.


#include "EOK_API_DirectOwnershipVerification.h"

UEOK_API_DirectOwnershipVerification* UEOK_API_DirectOwnershipVerification::DirectOwnershipVerification(
	FString Authorization, FString CurrentAccountId, TArray<FString> NsCatalogItemIds, FString SandboxId)
{
	UEOK_API_DirectOwnershipVerification* Node = NewObject<UEOK_API_DirectOwnershipVerification>();
	Node->Var_Authorization = Authorization;
	Node->Var_CurrentAccountId = CurrentAccountId;
	Node->Var_NsCatalogItemIds = NsCatalogItemIds;
	Node->Var_SandboxId = SandboxId;
	return Node;
}

void UEOK_API_DirectOwnershipVerification::Activate()
{
	Super::Activate();
	FString URL = FString::Printf(TEXT("%s/epic/ecom/v3/platforms/EPIC/identities/%s/ownership"), *APIEndpoint, *Var_CurrentAccountId);
	//https://api.epicgames.dev/epic/ecom/v3/platforms/EPIC/identities/{{currentAccountId}}/ownership?nsCatalogItemId={{sandboxId:catalogItemId}}
	if (Var_NsCatalogItemIds.Num() > 0)
	{
		URL.Append("?nsCatalogItemId=");
		for (int i = 0; i < Var_NsCatalogItemIds.Num(); i++)
		{
			URL.Append(FString::Printf(TEXT("%s%s"), *Var_SandboxId, *Var_NsCatalogItemIds[i]));
			if (i < Var_NsCatalogItemIds.Num() - 1)
			{
				URL.Append(",");
			}
		}
	}
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
	HttpRequest->OnProcessRequestComplete().BindUObject(this, &UEOK_API_DirectOwnershipVerification::OnResponseReceived);
	HttpRequest->ProcessRequest();
	
	
	

}
