// Copyright (C) 2024, All Rights Reserved.

#include "Functions/Voice/EOSRemoveVoiceParticipantAsync.h"
#include "HttpModule.h"
#include "Interfaces/IHttpRequest.h"

UEOSRemoveVoiceParticipantAsync* UEOSRemoveVoiceParticipantAsync::RemoveVoiceParticipant(
	const FString& Authorization,
	const FString& DeploymentId,
	const FString& RoomId,
	const FString& ProductUserId)
{
	UEOSRemoveVoiceParticipantAsync* Node = NewObject<UEOSRemoveVoiceParticipantAsync>();
	Node->Var_Authorization = Authorization;
	Node->Var_DeploymentId = DeploymentId;
	Node->Var_RoomId = RoomId;
	Node->Var_ProductUserId = ProductUserId;
	return Node;
}

void UEOSRemoveVoiceParticipantAsync::Activate()
{
	Super::Activate();

	FString URL = FString::Printf(TEXT("%s/rtc/v1/%s/room/%s/participants/%s"), 
		*APIEndpoint, *Var_DeploymentId, *Var_RoomId, *Var_ProductUserId);

	TSharedRef<IHttpRequest, ESPMode::ThreadSafe> HttpRequest = FHttpModule::Get().CreateRequest();
	HttpRequest->SetVerb(TEXT("DELETE"));
	HttpRequest->SetURL(URL);
	
	FString AuthHeader = Var_Authorization.Contains(TEXT("Bearer")) 
		? Var_Authorization 
		: FString::Printf(TEXT("Bearer %s"), *Var_Authorization);
	HttpRequest->SetHeader(TEXT("Authorization"), AuthHeader);

	HttpRequest->OnProcessRequestComplete().BindUObject(this, &UEOSRemoveVoiceParticipantAsync::OnResponseReceived);
	HttpRequest->ProcessRequest();
}

