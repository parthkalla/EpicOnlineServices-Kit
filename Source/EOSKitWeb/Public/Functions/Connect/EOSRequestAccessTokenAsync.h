// Copyright (C) 2024, All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "EOSKitBaseWebApi.h"
#include "EOSRequestAccessTokenAsync.generated.h"

/**
 * Async node to request access token via Web API
 * EOS Connect backend provides an OAuth 2.0 token endpoint to request access tokens for the EOS Game Services Web APIs
 */
UCLASS()
class EOSKITWEB_API UEOSRequestAccessTokenAsync : public UEOSKitBaseWebApi
{
	GENERATED_BODY()

public:
	/**
	 * Request access token
	 * @param Authorization - Basic auth (ClientId:ClientSecret base64 encoded)
	 * @param ClientId - Client ID
	 * @param ClientSecret - Client Secret
	 * @param GrantType - Grant type (default: "client_credentials")
	 * @param Nonce - Optional nonce
	 * @param DeploymentId - Deployment ID
	 * @param ExternalAuthToken - Optional external auth token
	 * @param ExternalAuthType - Optional external auth type
	 * @return Async node instance
	 */
	UFUNCTION(BlueprintCallable, Category = "EOSKit|Web|Connect",
		meta = (BlueprintInternalUseOnly = "true"),
		DisplayName = "Request Access Token")
	static UEOSRequestAccessTokenAsync* RequestAccessToken(
		const FString& Authorization,
		const FString& ClientId,
		const FString& ClientSecret,
		const FString& GrantType = TEXT("client_credentials"),
		const FString& Nonce = TEXT(""),
		const FString& DeploymentId = TEXT(""),
		const FString& ExternalAuthToken = TEXT(""),
		const FString& ExternalAuthType = TEXT("")
	);

private:
	virtual void Activate() override;

	FString Var_Authorization;
	FString Var_ClientId;
	FString Var_ClientSecret;
	FString Var_GrantType;
	FString Var_Nonce;
	FString Var_DeploymentId;
	FString Var_ExternalAuthToken;
	FString Var_ExternalAuthType;
};

