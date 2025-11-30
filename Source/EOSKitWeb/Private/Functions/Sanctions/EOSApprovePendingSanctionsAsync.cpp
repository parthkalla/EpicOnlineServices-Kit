// Copyright (C) 2024, All Rights Reserved.

#include "Functions/Sanctions/EOSApprovePendingSanctionsAsync.h"
#include "HttpModule.h"
#include "Interfaces/IHttpRequest.h"
#include "Dom/JsonObject.h"
#include "Serialization/JsonSerializer.h"
#include "Serialization/JsonWriter.h"

UEOSApprovePendingSanctionsAsync* UEOSApprovePendingSanctionsAsync::ApprovePendingSanctions(
	const FString& Authorization,
	const FString& DeploymentId,
	const FString& EOS_OrganizationId,
	const FString& EOS_ClientId,
	const FString& EOS_DeploymentId,
	const TArray<FString>& ReferenceIds)
{
	UEOSApprovePendingSanctionsAsync* Node = NewObject<UEOSApprovePendingSanctionsAsync>();
	Node->Var_Authorization = Authorization;
	Node->Var_DeploymentId = DeploymentId;
	Node->Var_EOS_OrganizationId = EOS_OrganizationId;
	Node->Var_EOS_ClientId = EOS_ClientId;
	Node->Var_EOS_DeploymentId = EOS_DeploymentId;
	Node->Var_ReferenceIds = ReferenceIds;
	return Node;
}

void UEOSApprovePendingSanctionsAsync::Activate()
{
	Super::Activate();

	FString URL = FString::Printf(TEXT("%s/sanctions/v1/%s/sanctions/pending/approve"), *APIEndpoint, *Var_DeploymentId);

	TSharedRef<IHttpRequest, ESPMode::ThreadSafe> HttpRequest = FHttpModule::Get().CreateRequest();
	HttpRequest->SetVerb(TEXT("PATCH"));
	HttpRequest->SetURL(URL);
	HttpRequest->SetHeader(TEXT("Content-Type"), TEXT("application/json"));
	
	FString AuthHeader = Var_Authorization.Contains(TEXT("Bearer")) 
		? Var_Authorization 
		: FString::Printf(TEXT("Bearer %s"), *Var_Authorization);
	HttpRequest->SetHeader(TEXT("Authorization"), AuthHeader);
	HttpRequest->SetHeader(TEXT("Eos-Organization-Id"), Var_EOS_OrganizationId);
	HttpRequest->SetHeader(TEXT("Eos-Client-Id"), Var_EOS_ClientId);
	HttpRequest->SetHeader(TEXT("Eos-Deployment-Id"), Var_EOS_DeploymentId);

	// Build JSON body
	TSharedPtr<FJsonObject> RequestBodyJson = MakeShareable(new FJsonObject);
	TArray<TSharedPtr<FJsonValue>> ReferenceIdsJson;
	for (const FString& ReferenceId : Var_ReferenceIds)
	{
		ReferenceIdsJson.Add(MakeShareable(new FJsonValueString(ReferenceId)));
	}
	RequestBodyJson->SetArrayField(TEXT("referenceIds"), ReferenceIdsJson);

	FString RequestBodyString;
	TSharedRef<TJsonWriter<TCHAR>> JsonWriter = TJsonWriterFactory<>::Create(&RequestBodyString);
	FJsonSerializer::Serialize(RequestBodyJson.ToSharedRef(), JsonWriter);
	HttpRequest->SetContentAsString(RequestBodyString);

	HttpRequest->OnProcessRequestComplete().BindUObject(this, &UEOSApprovePendingSanctionsAsync::OnResponseReceived);
	HttpRequest->ProcessRequest();
}

