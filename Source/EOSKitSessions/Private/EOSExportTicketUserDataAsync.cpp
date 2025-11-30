// Copyright (C) 2024, All Rights Reserved.

#include "EOSExportTicketUserDataAsync.h"
#include "HttpModule.h"
#include "Interfaces/IHttpRequest.h"
#include "Interfaces/IHttpResponse.h"
#include "Dom/JsonObject.h"
#include "Serialization/JsonSerializer.h"

UEOSExportTicketUserDataAsync* UEOSExportTicketUserDataAsync::ExportTicketUserData(
	UObject* WorldContextObject,
	const FString& Email,
	const FString& ApiKey)
{
	UEOSExportTicketUserDataAsync* BlueprintNode = NewObject<UEOSExportTicketUserDataAsync>();
	BlueprintNode->WorldContextObject = WorldContextObject;
	BlueprintNode->Email = Email;
	BlueprintNode->ApiKey = ApiKey;
	return BlueprintNode;
}

void UEOSExportTicketUserDataAsync::Activate()
{
	Super::Activate();
	ExportTicketData();
}

void UEOSExportTicketUserDataAsync::ExportTicketData()
{
	if (ApiKey.IsEmpty())
	{
		UE_LOG(LogTemp, Error, TEXT("EOSExportTicketUserDataAsync: API key is empty"));
		TArray<FEOSKitTicketConversationData> EmptyConversations;
		OnFailure.Broadcast(TEXT("API key is empty"), EmptyConversations, 0);
		SetReadyToDestroy();
		return;
	}

	if (Email.IsEmpty())
	{
		UE_LOG(LogTemp, Error, TEXT("EOSExportTicketUserDataAsync: Email is empty"));
		TArray<FEOSKitTicketConversationData> EmptyConversations;
		OnFailure.Broadcast(TEXT("Email is empty"), EmptyConversations, 0);
		SetReadyToDestroy();
		return;
	}

	FString URL = FString::Printf(TEXT("https://dev.epicgames.com/portal/api/v1/services/tickets/datarights/%s"), *Email);
	
	TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Request = FHttpModule::Get().CreateRequest();
	Request->SetURL(URL);
	Request->SetVerb(TEXT("GET"));
	Request->SetHeader(TEXT("Content-Type"), TEXT("application/json"));
	
	FString AuthHeader = ApiKey.Contains(TEXT("Token ")) ? ApiKey : FString::Printf(TEXT("Token %s"), *ApiKey);
	Request->SetHeader(TEXT("Authorization"), AuthHeader);

	Request->OnProcessRequestComplete().BindUObject(this, &UEOSExportTicketUserDataAsync::OnResponseReceived);
	Request->ProcessRequest();
}

void UEOSExportTicketUserDataAsync::OnResponseReceived(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful)
{
	if (bWasSuccessful && Response.IsValid())
	{
		// Parse the JSON response
		TSharedPtr<FJsonObject> JsonObject;
		TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(Response->GetContentAsString());

		if (FJsonSerializer::Deserialize(Reader, JsonObject) && JsonObject.IsValid())
		{
			TArray<FEOSKitTicketConversationData> Conversations;

			// Get the 'data' array
			const TArray<TSharedPtr<FJsonValue>>* DataArray = nullptr;
			if (JsonObject->TryGetArrayField(TEXT("data"), DataArray) && DataArray)
			{
				for (const TSharedPtr<FJsonValue>& Item : *DataArray)
				{
					if (Item->Type == EJson::Object)
					{
						const TSharedPtr<FJsonObject>& ConversationObj = Item->AsObject();
						FEOSKitTicketConversationData Conversation;

						ConversationObj->TryGetStringField(TEXT("guid"), Conversation.Guid);
						ConversationObj->TryGetStringField(TEXT("subject"), Conversation.Subject);
						ConversationObj->TryGetStringField(TEXT("message"), Conversation.Message);
						ConversationObj->TryGetStringField(TEXT("sender_name"), Conversation.SenderName);
						ConversationObj->TryGetStringField(TEXT("sender_email"), Conversation.SenderEmail);
						ConversationObj->TryGetStringField(TEXT("timestamp"), Conversation.Timestamp);

						// Parse messages array
						const TArray<TSharedPtr<FJsonValue>>* MessagesArray = nullptr;
						if (ConversationObj->TryGetArrayField(TEXT("messages"), MessagesArray) && MessagesArray)
						{
							for (const TSharedPtr<FJsonValue>& MessageItem : *MessagesArray)
							{
								if (MessageItem->Type == EJson::Object)
								{
									const TSharedPtr<FJsonObject>& MessageObj = MessageItem->AsObject();
									FEOSKitTicketMessageData Message;

									MessageObj->TryGetNumberField(TEXT("ticket_id"), Message.TicketId);
									MessageObj->TryGetStringField(TEXT("message"), Message.Message);
									MessageObj->TryGetStringField(TEXT("sender_name"), Message.SenderName);
									MessageObj->TryGetStringField(TEXT("sender_email"), Message.SenderEmail);
									MessageObj->TryGetStringField(TEXT("timestamp"), Message.Timestamp);

									Conversation.Messages.Add(Message);
								}
							}
						}

						Conversations.Add(Conversation);
					}
				}
			}

			// Broadcast the success delegate
			OnSuccess.Broadcast(Response->GetContentAsString(), Conversations, Response->GetResponseCode());
			SetReadyToDestroy();
			return;
		}
	}

	// Failure case
	UE_LOG(LogTemp, Warning, TEXT("EOSExportTicketUserDataAsync: Request failed or invalid response"));
	TArray<FEOSKitTicketConversationData> EmptyConversations;
	FString ErrorMessage = Response.IsValid() ? Response->GetContentAsString() : TEXT("Request failed");
	int32 ResponseCode = Response.IsValid() ? Response->GetResponseCode() : 0;
	OnFailure.Broadcast(ErrorMessage, EmptyConversations, ResponseCode);
	SetReadyToDestroy();
}

