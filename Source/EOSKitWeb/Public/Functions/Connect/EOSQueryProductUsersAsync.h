// Copyright (C) 2024, All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "EOSKitBaseWebApi.h"
#include "EOSQueryProductUsersAsync.generated.h"

/**
 * Async node to query product users via Web API
 * Returns associated accounts from a list of Product User IDs
 */
UCLASS()
class EOSKITWEB_API UEOSQueryProductUsersAsync : public UEOSKitBaseWebApi
{
	GENERATED_BODY()

public:
	/**
	 * Query product users
	 * @param Authorization - Bearer token for authentication
	 * @param ProductUserIds - Array of Product User IDs to query
	 * @return Async node instance
	 */
	UFUNCTION(BlueprintCallable, Category = "EOSKit|Web|Connect",
		meta = (BlueprintInternalUseOnly = "true"),
		DisplayName = "Query Product Users")
	static UEOSQueryProductUsersAsync* QueryProductUsers(
		const FString& Authorization,
		const TArray<FString>& ProductUserIds
	);

private:
	virtual void Activate() override;

	FString Var_Authorization;
	TArray<FString> Var_ProductUserIds;
};

