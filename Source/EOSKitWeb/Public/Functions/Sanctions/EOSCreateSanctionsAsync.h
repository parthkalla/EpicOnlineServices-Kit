// Copyright (C) 2024, All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "EOSKitBaseWebApi.h"
#include "EOSCreateSanctionsAsync.generated.h"

/**
 * Sanction payload structure
 */
USTRUCT(BlueprintType)
struct FEOSKitSanctionPostPayload
{
	GENERATED_BODY()

	/** Sanctioned user's EOS Product User ID */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "EOSKit|Web|Sanctions")
	FString ProductUserId = TEXT("");

	/** Action string associated with this sanction. Format: [a-zA-Z0-9_-]+. Min length: 1. Max length: 64. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "EOSKit|Web|Sanctions")
	FString Action = TEXT("");

	/** Justification string associated with this sanction. Min length: 1 Max length: 2048 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "EOSKit|Web|Sanctions")
	FString Justification = TEXT("");

	/** Source which created this sanction, e.g. developer-portal. Format: [a-zA-Z0-9_-]+. Min length: 2. Max length: 64. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "EOSKit|Web|Sanctions")
	FString Source = TEXT("");

	/** List of tags associated with this sanction. Items are case insensitive and unique. Item format: [a-zA-Z0-9_-]+. Item max length: 16 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "EOSKit|Web|Sanctions")
	TArray<FString> Tags;

	/** True if this sanction is currently pending */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "EOSKit|Web|Sanctions")
	bool bPending = false;

	/** Arbitrary metadata key/value pairs associated with this sanction */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "EOSKit|Web|Sanctions")
	TMap<FString, FString> Metadata;

	/** Display name of sanctioned user. Max length: 64 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "EOSKit|Web|Sanctions")
	FString DisplayName = TEXT("");

	/** Identity provider that the sanctioned user authenticated with. Max length: 64 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "EOSKit|Web|Sanctions")
	FString IdentityProvider = TEXT("");

	/** Sanctioned user's account ID with the specified identityProvider. Max length: 64 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "EOSKit|Web|Sanctions")
	FString AccountId = TEXT("");

	/** The length of the sanction in seconds */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "EOSKit|Web|Sanctions")
	int32 Duration = 0;
};

/**
 * Async node to create sanctions via Web API
 */
UCLASS()
class EOSKITWEB_API UEOSCreateSanctionsAsync : public UEOSKitBaseWebApi
{
	GENERATED_BODY()

public:
	/**
	 * Create sanctions
	 * @param Authorization - Bearer token for authentication
	 * @param DeploymentId - Deployment ID
	 * @param Sanctions - Array of sanction payloads
	 * @return Async node instance
	 */
	UFUNCTION(BlueprintCallable, Category = "EOSKit|Web|Sanctions",
		meta = (BlueprintInternalUseOnly = "true"),
		DisplayName = "Create Sanctions")
	static UEOSCreateSanctionsAsync* CreateSanctions(
		const FString& Authorization,
		const FString& DeploymentId,
		const TArray<FEOSKitSanctionPostPayload>& Sanctions
	);

private:
	virtual void Activate() override;

	FString Var_Authorization;
	FString Var_DeploymentId;
	TArray<FEOSKitSanctionPostPayload> Var_Sanctions;
};

