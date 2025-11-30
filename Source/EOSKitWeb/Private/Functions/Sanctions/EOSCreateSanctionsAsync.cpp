// Copyright (C) 2024, All Rights Reserved.

#include "Functions/Sanctions/EOSCreateSanctionsAsync.h"
#include "HttpModule.h"
#include "Interfaces/IHttpRequest.h"
#include "Dom/JsonObject.h"
#include "Serialization/JsonSerializer.h"
#include "Serialization/JsonWriter.h"

UEOSCreateSanctionsAsync* UEOSCreateSanctionsAsync::CreateSanctions(
	const FString& Authorization,
	const FString& DeploymentId,
	const TArray<FEOSKitSanctionPostPayload>& Sanctions)
{
	UEOSCreateSanctionsAsync* Node = NewObject<UEOSCreateSanctionsAsync>();
	Node->Var_Authorization = Authorization;
	Node->Var_DeploymentId = DeploymentId;
	Node->Var_Sanctions = Sanctions;
	return Node;
}

void UEOSCreateSanctionsAsync::Activate()
{
	Super::Activate();

	FString URL = FString::Printf(TEXT("%s/sanctions/v1/%s/sanctions"), *APIEndpoint, *Var_DeploymentId);

	TSharedRef<IHttpRequest, ESPMode::ThreadSafe> HttpRequest = FHttpModule::Get().CreateRequest();
	HttpRequest->SetVerb(TEXT("POST"));
	HttpRequest->SetURL(URL);
	HttpRequest->SetHeader(TEXT("Content-Type"), TEXT("application/json"));
	
	FString AuthHeader = Var_Authorization.Contains(TEXT("Bearer")) 
		? Var_Authorization 
		: FString::Printf(TEXT("Bearer %s"), *Var_Authorization);
	HttpRequest->SetHeader(TEXT("Authorization"), AuthHeader);

	// Build JSON body - array of sanctions
	TArray<TSharedPtr<FJsonValue>> SanctionsJson;
	for (const FEOSKitSanctionPostPayload& Sanction : Var_Sanctions)
	{
		TSharedPtr<FJsonObject> SanctionJson = MakeShareable(new FJsonObject);
		
		if (!Sanction.ProductUserId.IsEmpty())
		{
			SanctionJson->SetStringField(TEXT("productUserId"), Sanction.ProductUserId);
		}
		if (!Sanction.Action.IsEmpty())
		{
			SanctionJson->SetStringField(TEXT("action"), Sanction.Action);
		}
		if (!Sanction.Justification.IsEmpty())
		{
			SanctionJson->SetStringField(TEXT("justification"), Sanction.Justification);
		}
		if (!Sanction.Source.IsEmpty())
		{
			SanctionJson->SetStringField(TEXT("source"), Sanction.Source);
		}
		if (Sanction.Tags.Num() > 0)
		{
			TArray<TSharedPtr<FJsonValue>> TagsJson;
			for (const FString& Tag : Sanction.Tags)
			{
				TagsJson.Add(MakeShareable(new FJsonValueString(Tag)));
			}
			SanctionJson->SetArrayField(TEXT("tags"), TagsJson);
		}
		SanctionJson->SetBoolField(TEXT("pending"), Sanction.bPending);
		
		if (Sanction.Metadata.Num() > 0)
		{
			TSharedPtr<FJsonObject> MetadataJson = MakeShareable(new FJsonObject);
			for (const auto& Meta : Sanction.Metadata)
			{
				MetadataJson->SetStringField(Meta.Key, Meta.Value);
			}
			SanctionJson->SetObjectField(TEXT("metadata"), MetadataJson);
		}
		if (!Sanction.DisplayName.IsEmpty())
		{
			SanctionJson->SetStringField(TEXT("displayName"), Sanction.DisplayName);
		}
		if (!Sanction.IdentityProvider.IsEmpty())
		{
			SanctionJson->SetStringField(TEXT("identityProvider"), Sanction.IdentityProvider);
		}
		if (!Sanction.AccountId.IsEmpty())
		{
			SanctionJson->SetStringField(TEXT("accountId"), Sanction.AccountId);
		}
		SanctionJson->SetNumberField(TEXT("duration"), Sanction.Duration);
		
		SanctionsJson.Add(MakeShareable(new FJsonValueObject(SanctionJson)));
	}

	FString RequestBody;
	TSharedRef<TJsonWriter<TCHAR>> JsonWriter = TJsonWriterFactory<>::Create(&RequestBody);
	FJsonSerializer::Serialize(SanctionsJson, JsonWriter);
	HttpRequest->SetContentAsString(RequestBody);

	HttpRequest->OnProcessRequestComplete().BindUObject(this, &UEOSCreateSanctionsAsync::OnResponseReceived);
	HttpRequest->ProcessRequest();
}

