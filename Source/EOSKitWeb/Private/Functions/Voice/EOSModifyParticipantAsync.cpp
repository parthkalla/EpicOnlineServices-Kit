// Copyright (C) 2024, All Rights Reserved.

#include "Functions/Voice/EOSModifyParticipantAsync.h"
#include "HttpModule.h"
#include "Interfaces/IHttpRequest.h"
#include "Dom/JsonObject.h"
#include "Serialization/JsonSerializer.h"
#include "Serialization/JsonWriter.h"

UEOSModifyParticipantAsync* UEOSModifyParticipantAsync::ModifyParticipant(
	const FString& Authorization,
	const FString& DeploymentId,
	const FString& RoomId,
	const FString& ProductUserId,
	bool bHardMuted)
{
	UEOSModifyParticipantAsync* Node = NewObject<UEOSModifyParticipantAsync>();
	Node->Var_Authorization = Authorization;
	Node->Var_DeploymentId = DeploymentId;
	Node->Var_RoomId = RoomId;
	Node->Var_ProductUserId = ProductUserId;
	Node->Var_bHardMuted = bHardMuted;
	return Node;
}

void UEOSModifyParticipantAsync::Activate()
{
	Super::Activate();

	FString URL = FString::Printf(TEXT("%s/rtc/v1/%s/room/%s/participants/%s"), 
		*APIEndpoint, *Var_DeploymentId, *Var_RoomId, *Var_ProductUserId);

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
	RequestBodyJson->SetBoolField(TEXT("hardMuted"), Var_bHardMuted);

	FString RequestBody;
	TSharedRef<TJsonWriter<TCHAR>> JsonWriter = TJsonWriterFactory<>::Create(&RequestBody);
	FJsonSerializer::Serialize(RequestBodyJson.ToSharedRef(), JsonWriter);
	HttpRequest->SetContentAsString(RequestBody);

	HttpRequest->OnProcessRequestComplete().BindUObject(this, &UEOSModifyParticipantAsync::OnResponseReceived);
	HttpRequest->ProcessRequest();
}

