// Copyright (C) 2024, All Rights Reserved.

#include "Functions/Voice/EOSCreateRoomTokensAsync.h"
#include "HttpModule.h"
#include "Interfaces/IHttpRequest.h"
#include "Dom/JsonObject.h"
#include "Serialization/JsonSerializer.h"
#include "Serialization/JsonWriter.h"

UEOSCreateRoomTokensAsync* UEOSCreateRoomTokensAsync::CreateRoomTokens(
	const FString& Authorization,
	const FString& DeploymentId,
	const FString& RoomId,
	const TArray<FEOSKitRoomParticipant>& Participants)
{
	UEOSCreateRoomTokensAsync* Node = NewObject<UEOSCreateRoomTokensAsync>();
	Node->Var_Authorization = Authorization;
	Node->Var_DeploymentId = DeploymentId;
	Node->Var_RoomId = RoomId;
	Node->Var_Participants = Participants;
	return Node;
}

void UEOSCreateRoomTokensAsync::Activate()
{
	Super::Activate();

	FString URL = FString::Printf(TEXT("%s/rtc/v1/%s/room/%s"), *APIEndpoint, *Var_DeploymentId, *Var_RoomId);

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
	
	if (Var_Participants.Num() > 0)
	{
		TArray<TSharedPtr<FJsonValue>> ParticipantsJson;
		for (const FEOSKitRoomParticipant& Participant : Var_Participants)
		{
			TSharedPtr<FJsonObject> ParticipantJson = MakeShareable(new FJsonObject);
			if (!Participant.ProductUserId.IsEmpty())
			{
				ParticipantJson->SetStringField(TEXT("puid"), Participant.ProductUserId);
			}
			if (!Participant.ClientIp.IsEmpty())
			{
				ParticipantJson->SetStringField(TEXT("clientIP"), Participant.ClientIp);
			}
			ParticipantJson->SetBoolField(TEXT("hardMuted"), Participant.bHardMuted);
			ParticipantsJson.Add(MakeShareable(new FJsonValueObject(ParticipantJson)));
		}
		RequestBodyJson->SetArrayField(TEXT("participants"), ParticipantsJson);
	}

	FString RequestBody;
	TSharedRef<TJsonWriter<TCHAR>> JsonWriter = TJsonWriterFactory<>::Create(&RequestBody);
	FJsonSerializer::Serialize(RequestBodyJson.ToSharedRef(), JsonWriter);
	HttpRequest->SetContentAsString(RequestBody);

	HttpRequest->OnProcessRequestComplete().BindUObject(this, &UEOSCreateRoomTokensAsync::OnResponseReceived);
	HttpRequest->ProcessRequest();
}

