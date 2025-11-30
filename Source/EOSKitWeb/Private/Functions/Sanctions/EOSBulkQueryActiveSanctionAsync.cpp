// Copyright (C) 2024, All Rights Reserved.

#include "Functions/Sanctions/EOSBulkQueryActiveSanctionAsync.h"
#include "HttpModule.h"
#include "Interfaces/IHttpRequest.h"

UEOSBulkQueryActiveSanctionAsync* UEOSBulkQueryActiveSanctionAsync::BulkQueryActiveSanction(
	const FString& Authorization,
	const FString& DeploymentId,
	const TArray<FString>& ProductUserIds,
	const TArray<FString>& Actions)
{
	UEOSBulkQueryActiveSanctionAsync* Node = NewObject<UEOSBulkQueryActiveSanctionAsync>();
	Node->Var_Authorization = Authorization;
	Node->Var_DeploymentId = DeploymentId;
	Node->Var_ProductUserIds = ProductUserIds;
	Node->Var_Actions = Actions;
	return Node;
}

void UEOSBulkQueryActiveSanctionAsync::Activate()
{
	Super::Activate();

	FString URL = FString::Printf(TEXT("%s/sanctions/v1/%s/active-sanctions"), *APIEndpoint, *Var_DeploymentId);
	
	bool bFirstParam = true;
	if (Var_ProductUserIds.Num() > 0)
	{
		URL += TEXT("?");
		for (int32 i = 0; i < Var_ProductUserIds.Num(); i++)
		{
			if (i > 0)
			{
				URL += TEXT("&");
			}
			URL += FString::Printf(TEXT("productUserId=%s"), *Var_ProductUserIds[i]);
		}
		bFirstParam = false;
	}

	if (Var_Actions.Num() > 0)
	{
		FString Prefix = bFirstParam ? TEXT("?") : TEXT("&");
		for (int32 i = 0; i < Var_Actions.Num(); i++)
		{
			if (i > 0)
			{
				URL += TEXT("&");
			}
			URL += FString::Printf(TEXT("%saction=%s"), *Prefix, *Var_Actions[i]);
			Prefix = TEXT("");
		}
	}

	TSharedRef<IHttpRequest, ESPMode::ThreadSafe> HttpRequest = FHttpModule::Get().CreateRequest();
	HttpRequest->SetVerb(TEXT("GET"));
	HttpRequest->SetURL(URL);
	
	FString AuthHeader = Var_Authorization.Contains(TEXT("Bearer")) 
		? Var_Authorization 
		: FString::Printf(TEXT("Bearer %s"), *Var_Authorization);
	HttpRequest->SetHeader(TEXT("Authorization"), AuthHeader);

	HttpRequest->OnProcessRequestComplete().BindUObject(this, &UEOSBulkQueryActiveSanctionAsync::OnResponseReceived);
	HttpRequest->ProcessRequest();
}

