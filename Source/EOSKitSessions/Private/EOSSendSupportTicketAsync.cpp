// Copyright (C) 2024, All Rights Reserved.

#include "EOSSendSupportTicketAsync.h"
#include "HttpModule.h"
#include "Interfaces/IHttpRequest.h"
#include "Interfaces/IHttpResponse.h"
#include "Dom/JsonObject.h"
#include "Serialization/JsonSerializer.h"
#include "Serialization/JsonWriter.h"

UEOSSendSupportTicketAsync* UEOSSendSupportTicketAsync::SendSupportTicket(
	UObject* WorldContextObject,
	const FEOSKitSupportTicketData& TicketData,
	const FString& ApiKey)
{
	UEOSSendSupportTicketAsync* BlueprintNode = NewObject<UEOSSendSupportTicketAsync>();
	BlueprintNode->WorldContextObject = WorldContextObject;
	BlueprintNode->TicketData = TicketData;
	BlueprintNode->ApiKey = ApiKey;
	return BlueprintNode;
}

void UEOSSendSupportTicketAsync::Activate()
{
	Super::Activate();
	SendTicketRequest();
}

void UEOSSendSupportTicketAsync::SendTicketRequest()
{
	if (ApiKey.IsEmpty())
	{
		UE_LOG(LogTemp, Error, TEXT("EOSSendSupportTicketAsync: API key is empty"));
		FEOSKitSupportTicketResponseData EmptyResponse;
		OnFailure.Broadcast(TEXT("API key is empty"), EmptyResponse, 0);
		SetReadyToDestroy();
		return;
	}

	TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Request = FHttpModule::Get().CreateRequest();
	Request->SetURL(TEXT("https://dev.epicgames.com/portal/api/v1/services/tickets/submit/"));
	Request->SetVerb(TEXT("POST"));
	Request->SetHeader(TEXT("Content-Type"), TEXT("application/json"));
	
	FString AuthHeader = ApiKey.Contains(TEXT("Token ")) ? ApiKey : FString::Printf(TEXT("Token %s"), *ApiKey);
	Request->SetHeader(TEXT("Authorization"), AuthHeader);

	// Construct the request body
	TSharedPtr<FJsonObject> RequestObj = MakeShareable(new FJsonObject);

	FString SubjectString;
	switch (TicketData.Subject)
	{
	case EEOSKitSupportTicketSubject::OpenQuestion:
		SubjectString = TEXT("open-question");
		break;
	case EEOSKitSupportTicketSubject::TechnicalSupport:
		SubjectString = TEXT("technical-support");
		break;
	case EEOSKitSupportTicketSubject::BanAppeal:
		SubjectString = TEXT("ban-appeal");
		break;
	default:
		UE_LOG(LogTemp, Warning, TEXT("EOSSendSupportTicketAsync: Invalid subject enum value"));
		SubjectString = TEXT("open-question");
		break;
	}

	RequestObj->SetStringField(TEXT("subject"), SubjectString);
	RequestObj->SetStringField(TEXT("message"), TicketData.Message);
	RequestObj->SetStringField(TEXT("sender_email"), TicketData.SenderEmail);
	RequestObj->SetStringField(TEXT("sender_name"), TicketData.SenderName);
	RequestObj->SetStringField(TEXT("guid"), TicketData.Guid);
	RequestObj->SetStringField(TEXT("error_code"), TicketData.ErrorCode);
	RequestObj->SetStringField(TEXT("system_os"), TicketData.SystemOS);
	RequestObj->SetStringField(TEXT("system_antimalware"), TicketData.SystemAntiMalware);
	RequestObj->SetStringField(TEXT("system_other"), TicketData.SystemOther);

	FString RequestBody;
	TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&RequestBody);
	FJsonSerializer::Serialize(RequestObj.ToSharedRef(), Writer);
	Request->SetContentAsString(RequestBody);

	Request->OnProcessRequestComplete().BindUObject(this, &UEOSSendSupportTicketAsync::OnResponseReceived);
	Request->ProcessRequest();
}

void UEOSSendSupportTicketAsync::OnResponseReceived(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful)
{
	if (bWasSuccessful && Response.IsValid())
	{
		// Parse the JSON response
		TSharedPtr<FJsonObject> JsonObject;
		TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(Response->GetContentAsString());

		if (FJsonSerializer::Deserialize(Reader, JsonObject) && JsonObject.IsValid())
		{
			// Get the 'data' object
			const TSharedPtr<FJsonObject>* DataObjectPtr = nullptr;
			if (JsonObject->TryGetObjectField(TEXT("data"), DataObjectPtr) && DataObjectPtr && DataObjectPtr->IsValid())
			{
				const TSharedPtr<FJsonObject>& DataObject = *DataObjectPtr;

				// Populate the struct
				FEOSKitSupportTicketResponseData ResponseData;
				DataObject->TryGetStringField(TEXT("prod_name"), ResponseData.ProdName);
				DataObject->TryGetStringField(TEXT("prod_slug"), ResponseData.ProdSlug);
				DataObject->TryGetStringField(TEXT("guid"), ResponseData.Guid);
				DataObject->TryGetStringField(TEXT("sender_name"), ResponseData.SenderName);
				DataObject->TryGetStringField(TEXT("sender_email"), ResponseData.SenderEmail);
				DataObject->TryGetStringField(TEXT("subject"), ResponseData.Subject);
				DataObject->TryGetStringField(TEXT("message"), ResponseData.Message);
				DataObject->TryGetStringField(TEXT("error_code"), ResponseData.ErrorCode);
				DataObject->TryGetStringField(TEXT("system_os"), ResponseData.SystemOS);
				DataObject->TryGetStringField(TEXT("system_antimalware"), ResponseData.SystemAntiMalware);
				DataObject->TryGetStringField(TEXT("system_other"), ResponseData.SystemOther);
				DataObject->TryGetStringField(TEXT("timestamp"), ResponseData.Timestamp);

				// Broadcast the success delegate
				OnSuccess.Broadcast(Response->GetContentAsString(), ResponseData, Response->GetResponseCode());
				SetReadyToDestroy();
				return;
			}
		}
	}

	// Failure case
	UE_LOG(LogTemp, Warning, TEXT("EOSSendSupportTicketAsync: Request failed or invalid response"));
	FEOSKitSupportTicketResponseData EmptyResponse;
	FString ErrorMessage = Response.IsValid() ? Response->GetContentAsString() : TEXT("Request failed");
	int32 ResponseCode = Response.IsValid() ? Response->GetResponseCode() : 0;
	OnFailure.Broadcast(ErrorMessage, EmptyResponse, ResponseCode);
	SetReadyToDestroy();
}

