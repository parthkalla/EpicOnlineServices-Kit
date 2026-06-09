// Copyright (c) 2023 Betide Studio. All Rights Reserved.


#include "EOK_API_DirectEntitlementEnumeration.h"

UEOK_API_DirectEntitlementEnumeration* UEOK_API_DirectEntitlementEnumeration::DirectEntitlementEnumeration(
	FString Authorization, FString IdentityId, FString SandboxId, FString EntitlementName, bool bIncludeRedeemed)
{
	UEOK_API_DirectEntitlementEnumeration* Node = NewObject<UEOK_API_DirectEntitlementEnumeration>();
	Node->Var_Authorization = Authorization;
	Node->Var_IdentityId = IdentityId;
	Node->Var_SandboxId = SandboxId;
	Node->Var_EntitlementName = EntitlementName;
	Node->Var_bIncludeRedeemed = bIncludeRedeemed;
	return Node;
}

void UEOK_API_DirectEntitlementEnumeration::Activate()
{
	Super::Activate();
	FString URL = FString::Printf(TEXT("%s/epic/ecom/v3/identities/%s/entitlements?"), *APIEndpoint, *Var_IdentityId);
	if (!Var_SandboxId.IsEmpty())
	{
		URL.Append(FString::Printf(TEXT("sandboxId=%s"), *Var_SandboxId));
	}
	if (!Var_EntitlementName.IsEmpty())
	{
		URL.Append(FString::Printf(TEXT("&entitlementName=%s"), *Var_EntitlementName));
	}
	if (Var_bIncludeRedeemed)
	{
		URL.Append(TEXT("&includeRedeemed=true"));
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
	HttpRequest->OnProcessRequestComplete().BindUObject(this, &UEOK_API_DirectEntitlementEnumeration::OnResponseReceived);
	HttpRequest->ProcessRequest();
}
