// Copyright (C) 2024, All Rights Reserved.

#include "EOSDeleteTicketDataAsync.h"
#include "HttpModule.h"
#include "Interfaces/IHttpRequest.h"
#include "Interfaces/IHttpResponse.h"

UEOSDeleteTicketDataAsync* UEOSDeleteTicketDataAsync::DeleteTicketData(
	UObject* WorldContextObject,
	const FString& Email,
	const FString& ApiKey)
{
	UEOSDeleteTicketDataAsync* BlueprintNode = NewObject<UEOSDeleteTicketDataAsync>();
	BlueprintNode->WorldContextObject = WorldContextObject;
	BlueprintNode->Email = Email;
	BlueprintNode->ApiKey = ApiKey;
	return BlueprintNode;
}

void UEOSDeleteTicketDataAsync::Activate()
{
	Super::Activate();
	DeleteTicketDataRequest();
}

void UEOSDeleteTicketDataAsync::DeleteTicketDataRequest()
{
	if (ApiKey.IsEmpty())
	{
		UE_LOG(LogTemp, Error, TEXT("EOSDeleteTicketDataAsync: API key is empty"));
		OnFailure.Broadcast(TEXT("API key is empty"), 0, false);
		SetReadyToDestroy();
		return;
	}

	if (Email.IsEmpty())
	{
		UE_LOG(LogTemp, Error, TEXT("EOSDeleteTicketDataAsync: Email is empty"));
		OnFailure.Broadcast(TEXT("Email is empty"), 0, false);
		SetReadyToDestroy();
		return;
	}

	FString URL = FString::Printf(TEXT("https://dev.epicgames.com/portal/api/v1/services/tickets/datarights/%s"), *Email);
	
	TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Request = FHttpModule::Get().CreateRequest();
	Request->SetURL(URL);
	Request->SetVerb(TEXT("DELETE"));
	Request->SetHeader(TEXT("Content-Type"), TEXT("application/json"));
	
	FString AuthHeader = ApiKey.Contains(TEXT("Token ")) ? ApiKey : FString::Printf(TEXT("Token %s"), *ApiKey);
	Request->SetHeader(TEXT("Authorization"), AuthHeader);

	Request->OnProcessRequestComplete().BindUObject(this, &UEOSDeleteTicketDataAsync::OnResponseReceived);
	Request->ProcessRequest();
}

void UEOSDeleteTicketDataAsync::OnResponseReceived(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful)
{
	if (bWasSuccessful && Response.IsValid())
	{
		int32 ResponseCode = Response->GetResponseCode();
		// 200 or 204 typically indicate success for DELETE
		if (ResponseCode == 200 || ResponseCode == 204)
		{
			UE_LOG(LogTemp, Log, TEXT("EOSDeleteTicketDataAsync: Ticket data deleted successfully"));
			OnSuccess.Broadcast(Response->GetContentAsString(), ResponseCode, true);
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("EOSDeleteTicketDataAsync: Delete failed with response code: %d"), ResponseCode);
			OnFailure.Broadcast(Response->GetContentAsString(), ResponseCode, false);
		}
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("EOSDeleteTicketDataAsync: Request failed"));
		FString ErrorMessage = Response.IsValid() ? Response->GetContentAsString() : TEXT("Request failed");
		int32 ResponseCode = Response.IsValid() ? Response->GetResponseCode() : 0;
		OnFailure.Broadcast(ErrorMessage, ResponseCode, false);
	}

	SetReadyToDestroy();
}

