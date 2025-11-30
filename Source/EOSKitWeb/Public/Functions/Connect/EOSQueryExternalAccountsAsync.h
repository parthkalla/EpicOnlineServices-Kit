// Copyright (C) 2024, All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "EOSKitBaseWebApi.h"
#include "EOSQueryExternalAccountsAsync.generated.h"

/**
 * Async node to query external accounts via Web API
 * Returns associated Product User IDs from a list of external account IDs
 */
UCLASS()
class EOSKITWEB_API UEOSQueryExternalAccountsAsync : public UEOSKitBaseWebApi
{
	GENERATED_BODY()

public:
	/**
	 * Query external accounts
	 * @param Authorization - Bearer token for authentication
	 * @param AccountIds - Array of external account IDs to query
	 * @param IdentityProviderId - Identity provider ID (e.g., "steam", "epic")
	 * @param Environment - Environment (e.g., "prod", "sandbox")
	 * @return Async node instance
	 */
	UFUNCTION(BlueprintCallable, Category = "EOSKit|Web|Connect",
		meta = (BlueprintInternalUseOnly = "true"),
		DisplayName = "Query External Accounts")
	static UEOSQueryExternalAccountsAsync* QueryExternalAccounts(
		const FString& Authorization,
		const TArray<FString>& AccountIds,
		const FString& IdentityProviderId = TEXT(""),
		const FString& Environment = TEXT("")
	);

private:
	virtual void Activate() override;

	FString Var_Authorization;
	TArray<FString> Var_AccountIds;
	FString Var_IdentityProviderId;
	FString Var_Environment;
};

