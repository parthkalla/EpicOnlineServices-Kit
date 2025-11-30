// Copyright (C) 2024, All Rights Reserved.

#include "Functions/Sanctions/EOSCreateSanctionAppealForLocalUserAsync.h"
#include "HttpModule.h"
#include "Interfaces/IHttpRequest.h"
#include "Dom/JsonObject.h"
#include "Serialization/JsonSerializer.h"
#include "Serialization/JsonWriter.h"

UEOSCreateSanctionAppealForLocalUserAsync* UEOSCreateSanctionAppealForLocalUserAsync::CreateSanctionAppealForLocalUser(
	const FString& Authorization,
	const FString& DeploymentId,
	const FString& ProductUserId,
	const FString& ReferenceId,
	const FString& Reason)
{
	UEOSCreateSanctionAppealForLocalUserAsync* Node = NewObject<UEOSCreateSanctionAppealForLocalUserAsync>();
	Node->Var_Authorization = Authorization;
	Node->Var_DeploymentId = DeploymentId;
	Node->Var_ProductUserId = ProductUserId;
	Node->Var_ReferenceId = ReferenceId;
	Node->Var_Reason = Reason;
	return Node;
}

void UEOSCreateSanctionAppealForLocalUserAsync::Activate()
{
	Super::Activate();

	FString URL = FString::Printf(TEXT("%s/sanctions/v1/%s/productUser/%s/appeals"), 
		*APIEndpoint, *Var_DeploymentId, *Var_ProductUserId);

	TSharedRef<IHttpRequest, ESPMode::ThreadSafe> HttpRequest = FHttpModule::Get().CreateRequest();
	HttpRequest->SetVerb(TEXT("POST"));
	HttpRequest->SetURL(URL);
	HttpRequest->SetHeader(TEXT("Content-Type"), TEXT("application/json"));
	
	FString AuthHeader = Var_Authorization.Contains(TEXT("Bearer")) 
		? Var_Authorization 
		: FString::Printf(TEXT("Bearer %s"), *Var_Authorization);
	HttpRequest->SetHeader(TEXT("Authorization"), AuthHeader);

	// Build JSON body
	TSharedPtr<FJsonObject> RequestBodyJson = MakeShareable(new FJsonObject);
	if (!Var_ReferenceId.IsEmpty())
	{
		RequestBodyJson->SetStringField(TEXT("referenceId"), Var_ReferenceId);
	}
	if (!Var_Reason.IsEmpty())
	{
		RequestBodyJson->SetStringField(TEXT("reason"), Var_Reason);
	}

	FString RequestBodyString;
	TSharedRef<TJsonWriter<TCHAR>> JsonWriter = TJsonWriterFactory<>::Create(&RequestBodyString);
	FJsonSerializer::Serialize(RequestBodyJson.ToSharedRef(), JsonWriter);
	HttpRequest->SetContentAsString(RequestBodyString);

	HttpRequest->OnProcessRequestComplete().BindUObject(this, &UEOSCreateSanctionAppealForLocalUserAsync::OnResponseReceived);
	HttpRequest->ProcessRequest();
}

