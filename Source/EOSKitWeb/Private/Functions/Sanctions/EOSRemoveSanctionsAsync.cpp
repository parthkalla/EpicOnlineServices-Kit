// Copyright (C) 2024, All Rights Reserved.

#include "Functions/Sanctions/EOSRemoveSanctionsAsync.h"
#include "HttpModule.h"
#include "Interfaces/IHttpRequest.h"
#include "Dom/JsonObject.h"
#include "Serialization/JsonSerializer.h"
#include "Serialization/JsonWriter.h"

UEOSRemoveSanctionsAsync* UEOSRemoveSanctionsAsync::RemoveSanctions(
	const FString& Authorization,
	const FString& DeploymentId,
	const TArray<FString>& ReferenceIds,
	const FString& Justification)
{
	UEOSRemoveSanctionsAsync* Node = NewObject<UEOSRemoveSanctionsAsync>();
	Node->Var_Authorization = Authorization;
	Node->Var_DeploymentId = DeploymentId;
	Node->Var_ReferenceIds = ReferenceIds;
	Node->Var_Justification = Justification;
	return Node;
}

void UEOSRemoveSanctionsAsync::Activate()
{
	Super::Activate();

	FString URL = FString::Printf(TEXT("%s/sanctions/v1/%s/sanctions"), *APIEndpoint, *Var_DeploymentId);

	TSharedRef<IHttpRequest, ESPMode::ThreadSafe> HttpRequest = FHttpModule::Get().CreateRequest();
	HttpRequest->SetVerb(TEXT("DELETE"));
	HttpRequest->SetURL(URL);
	HttpRequest->SetHeader(TEXT("Content-Type"), TEXT("application/json"));
	
	FString AuthHeader = Var_Authorization.Contains(TEXT("Bearer")) 
		? Var_Authorization 
		: FString::Printf(TEXT("Bearer %s"), *Var_Authorization);
	HttpRequest->SetHeader(TEXT("Authorization"), AuthHeader);

	// Build JSON body
	TSharedPtr<FJsonObject> RequestBodyJson = MakeShareable(new FJsonObject);
	
	TArray<TSharedPtr<FJsonValue>> ReferenceIdsJson;
	for (const FString& ReferenceId : Var_ReferenceIds)
	{
		ReferenceIdsJson.Add(MakeShareable(new FJsonValueString(ReferenceId)));
	}
	RequestBodyJson->SetArrayField(TEXT("referenceIds"), ReferenceIdsJson);
	
	if (!Var_Justification.IsEmpty())
	{
		RequestBodyJson->SetStringField(TEXT("justification"), Var_Justification);
	}

	FString RequestBodyString;
	TSharedRef<TJsonWriter<TCHAR>> JsonWriter = TJsonWriterFactory<>::Create(&RequestBodyString);
	FJsonSerializer::Serialize(RequestBodyJson.ToSharedRef(), JsonWriter);
	HttpRequest->SetContentAsString(RequestBodyString);

	HttpRequest->OnProcessRequestComplete().BindUObject(this, &UEOSRemoveSanctionsAsync::OnResponseReceived);
	HttpRequest->ProcessRequest();
}

