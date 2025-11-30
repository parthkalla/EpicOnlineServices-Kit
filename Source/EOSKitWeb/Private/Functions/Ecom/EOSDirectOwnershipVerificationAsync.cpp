// Copyright (C) 2024, All Rights Reserved.

#include "Functions/Ecom/EOSDirectOwnershipVerificationAsync.h"
#include "HttpModule.h"
#include "Interfaces/IHttpRequest.h"

UEOSDirectOwnershipVerificationAsync* UEOSDirectOwnershipVerificationAsync::DirectOwnershipVerification(
	const FString& Authorization,
	const FString& CurrentAccountId,
	const TArray<FString>& NsCatalogItemIds,
	const FString& SandboxId)
{
	UEOSDirectOwnershipVerificationAsync* Node = NewObject<UEOSDirectOwnershipVerificationAsync>();
	Node->Var_Authorization = Authorization;
	Node->Var_CurrentAccountId = CurrentAccountId;
	Node->Var_NsCatalogItemIds = NsCatalogItemIds;
	Node->Var_SandboxId = SandboxId;
	return Node;
}

void UEOSDirectOwnershipVerificationAsync::Activate()
{
	Super::Activate();

	FString URL = FString::Printf(TEXT("%s/epic/ecom/v3/platforms/EPIC/identities/%s/ownership"), 
		*APIEndpoint, *Var_CurrentAccountId);
	
	if (Var_NsCatalogItemIds.Num() > 0)
	{
		URL += TEXT("?nsCatalogItemId=");
		for (int32 i = 0; i < Var_NsCatalogItemIds.Num(); i++)
		{
			URL += FString::Printf(TEXT("%s%s"), *Var_SandboxId, *Var_NsCatalogItemIds[i]);
			if (i < Var_NsCatalogItemIds.Num() - 1)
			{
				URL += TEXT(",");
			}
		}
	}

	TSharedRef<IHttpRequest, ESPMode::ThreadSafe> HttpRequest = FHttpModule::Get().CreateRequest();
	HttpRequest->SetVerb(TEXT("GET"));
	HttpRequest->SetURL(URL);
	HttpRequest->SetHeader(TEXT("Content-Type"), TEXT("application/json"));
	
	FString AuthHeader = Var_Authorization.Contains(TEXT("Bearer")) 
		? Var_Authorization 
		: FString::Printf(TEXT("Bearer %s"), *Var_Authorization);
	HttpRequest->SetHeader(TEXT("Authorization"), AuthHeader);

	HttpRequest->OnProcessRequestComplete().BindUObject(this, &UEOSDirectOwnershipVerificationAsync::OnResponseReceived);
	HttpRequest->ProcessRequest();
}

