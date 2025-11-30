// Copyright (C) 2024, All Rights Reserved.

#include "Functions/Ecom/EOSRedeemEntitlementsAsync.h"
#include "HttpModule.h"
#include "Interfaces/IHttpRequest.h"
#include "Dom/JsonObject.h"
#include "Serialization/JsonSerializer.h"
#include "Serialization/JsonWriter.h"

UEOSRedeemEntitlementsAsync* UEOSRedeemEntitlementsAsync::RedeemEntitlements(
	const FString& Authorization,
	const FString& IdentityId,
	const TArray<FString>& EntitlementIds,
	const FString& SandboxId)
{
	UEOSRedeemEntitlementsAsync* Node = NewObject<UEOSRedeemEntitlementsAsync>();
	Node->Var_Authorization = Authorization;
	Node->Var_IdentityId = IdentityId;
	Node->Var_EntitlementIds = EntitlementIds;
	Node->Var_SandboxId = SandboxId;
	return Node;
}

void UEOSRedeemEntitlementsAsync::Activate()
{
	Super::Activate();

	FString URL = FString::Printf(TEXT("%s/epic/ecom/v3/identities/%s/entitlements/redeem"), 
		*APIEndpoint, *Var_IdentityId);

	TSharedRef<IHttpRequest, ESPMode::ThreadSafe> HttpRequest = FHttpModule::Get().CreateRequest();
	HttpRequest->SetVerb(TEXT("PUT"));
	HttpRequest->SetURL(URL);
	HttpRequest->SetHeader(TEXT("Content-Type"), TEXT("application/json"));
	
	FString AuthHeader = Var_Authorization.Contains(TEXT("Bearer")) 
		? Var_Authorization 
		: FString::Printf(TEXT("Bearer %s"), *Var_Authorization);
	HttpRequest->SetHeader(TEXT("Authorization"), AuthHeader);

	// Build JSON body
	TSharedPtr<FJsonObject> RequestBodyJson = MakeShareable(new FJsonObject);
	if (!Var_SandboxId.IsEmpty())
	{
		RequestBodyJson->SetStringField(TEXT("sandboxId"), Var_SandboxId);
	}

	TArray<TSharedPtr<FJsonValue>> EntitlementIdsJson;
	for (const FString& EntitlementId : Var_EntitlementIds)
	{
		EntitlementIdsJson.Add(MakeShareable(new FJsonValueString(EntitlementId)));
	}
	RequestBodyJson->SetArrayField(TEXT("entitlementIds"), EntitlementIdsJson);

	FString RequestBodyString;
	TSharedRef<TJsonWriter<TCHAR>> JsonWriter = TJsonWriterFactory<>::Create(&RequestBodyString);
	FJsonSerializer::Serialize(RequestBodyJson.ToSharedRef(), JsonWriter);
	HttpRequest->SetContentAsString(RequestBodyString);

	HttpRequest->OnProcessRequestComplete().BindUObject(this, &UEOSRedeemEntitlementsAsync::OnResponseReceived);
	HttpRequest->ProcessRequest();
}

