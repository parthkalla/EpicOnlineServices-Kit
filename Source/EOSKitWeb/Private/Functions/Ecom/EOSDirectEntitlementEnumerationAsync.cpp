// Copyright (C) 2024, All Rights Reserved.

#include "Functions/Ecom/EOSDirectEntitlementEnumerationAsync.h"
#include "HttpModule.h"
#include "Interfaces/IHttpRequest.h"

UEOSDirectEntitlementEnumerationAsync* UEOSDirectEntitlementEnumerationAsync::DirectEntitlementEnumeration(
	const FString& Authorization,
	const FString& IdentityId,
	const FString& SandboxId,
	const FString& EntitlementName,
	bool bIncludeRedeemed)
{
	UEOSDirectEntitlementEnumerationAsync* Node = NewObject<UEOSDirectEntitlementEnumerationAsync>();
	Node->Var_Authorization = Authorization;
	Node->Var_IdentityId = IdentityId;
	Node->Var_SandboxId = SandboxId;
	Node->Var_EntitlementName = EntitlementName;
	Node->Var_bIncludeRedeemed = bIncludeRedeemed;
	return Node;
}

void UEOSDirectEntitlementEnumerationAsync::Activate()
{
	Super::Activate();

	FString URL = FString::Printf(TEXT("%s/epic/ecom/v3/identities/%s/entitlements?"), *APIEndpoint, *Var_IdentityId);
	
	if (!Var_SandboxId.IsEmpty())
	{
		URL += FString::Printf(TEXT("sandboxId=%s"), *Var_SandboxId);
	}
	
	if (!Var_EntitlementName.IsEmpty())
	{
		URL += FString::Printf(TEXT("&entitlementName=%s"), *Var_EntitlementName);
	}
	
	if (Var_bIncludeRedeemed)
	{
		URL += TEXT("&includeRedeemed=true");
	}

	TSharedRef<IHttpRequest, ESPMode::ThreadSafe> HttpRequest = FHttpModule::Get().CreateRequest();
	HttpRequest->SetVerb(TEXT("GET"));
	HttpRequest->SetURL(URL);
	HttpRequest->SetHeader(TEXT("Content-Type"), TEXT("application/json"));
	
	FString AuthHeader = Var_Authorization.Contains(TEXT("Bearer")) 
		? Var_Authorization 
		: FString::Printf(TEXT("Bearer %s"), *Var_Authorization);
	HttpRequest->SetHeader(TEXT("Authorization"), AuthHeader);

	HttpRequest->OnProcessRequestComplete().BindUObject(this, &UEOSDirectEntitlementEnumerationAsync::OnResponseReceived);
	HttpRequest->ProcessRequest();
}

