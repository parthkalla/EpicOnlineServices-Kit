// Copyright (C) 2024, All Rights Reserved.

#include "Functions/Connect/EOSRequestAccessTokenAsync.h"
#include "HttpModule.h"
#include "Interfaces/IHttpRequest.h"
#include "Misc/Base64.h"

UEOSRequestAccessTokenAsync* UEOSRequestAccessTokenAsync::RequestAccessToken(
	const FString& Authorization,
	const FString& ClientId,
	const FString& ClientSecret,
	const FString& GrantType,
	const FString& Nonce,
	const FString& DeploymentId,
	const FString& ExternalAuthToken,
	const FString& ExternalAuthType)
{
	UEOSRequestAccessTokenAsync* Node = NewObject<UEOSRequestAccessTokenAsync>();
	Node->Var_Authorization = Authorization;
	Node->Var_ClientId = ClientId;
	Node->Var_ClientSecret = ClientSecret;
	Node->Var_GrantType = GrantType;
	Node->Var_Nonce = Nonce;
	Node->Var_DeploymentId = DeploymentId;
	Node->Var_ExternalAuthToken = ExternalAuthToken;
	Node->Var_ExternalAuthType = ExternalAuthType;
	return Node;
}

void UEOSRequestAccessTokenAsync::Activate()
{
	Super::Activate();

	FString URL = FString::Printf(TEXT("%s/auth/v1/oauth/token"), *APIEndpoint);
	
	TSharedRef<IHttpRequest, ESPMode::ThreadSafe> HttpRequest = FHttpModule::Get().CreateRequest();
	HttpRequest->SetVerb(TEXT("POST"));
	HttpRequest->SetURL(URL);
	HttpRequest->SetHeader(TEXT("Content-Type"), TEXT("application/x-www-form-urlencoded"));
	
	// Create Basic auth header
	FString Credentials = FString::Printf(TEXT("%s:%s"), *Var_ClientId, *Var_ClientSecret);
	FString Base64Credentials = FBase64::Encode(Credentials);
	HttpRequest->SetHeader(TEXT("Authorization"), FString::Printf(TEXT("Basic %s"), *Base64Credentials));

	// Build request body
	FString RequestBody = FString::Printf(TEXT("grant_type=%s"), *Var_GrantType);
	
	if (!Var_DeploymentId.IsEmpty())
	{
		RequestBody += FString::Printf(TEXT("&deployment_id=%s"), *Var_DeploymentId);
	}

	if (!Var_Nonce.IsEmpty())
	{
		RequestBody += FString::Printf(TEXT("&nonce=%s"), *Var_Nonce);
	}

	if (!Var_ExternalAuthToken.IsEmpty() && !Var_ExternalAuthType.IsEmpty())
	{
		RequestBody += FString::Printf(TEXT("&external_auth_token=%s&external_auth_type=%s"), 
			*Var_ExternalAuthToken, *Var_ExternalAuthType);
	}

	HttpRequest->SetContentAsString(RequestBody);
	HttpRequest->OnProcessRequestComplete().BindUObject(this, &UEOSRequestAccessTokenAsync::OnResponseReceived);
	HttpRequest->ProcessRequest();
}

