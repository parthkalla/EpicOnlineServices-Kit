// Copyright (C) 2024, All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintAsyncActionBase.h"
#include "EOSKitSessionStructs.h"
#include "EOSQueryExternalAccountMappingsAsync.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FQueryExternalAccountMappings_Delegate, const FString&, ProductUserId, bool, bSuccess);

/**
 * Query External Account Mappings - Retrieve Product User IDs from external account IDs
 */
UCLASS()
class EOSKITSESSIONS_API UEOSQueryExternalAccountMappingsAsync : public UBlueprintAsyncActionBase
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintAssignable, DisplayName="On Complete")
	FQueryExternalAccountMappings_Delegate OnComplete;

	/**
	 * Retrieve the equivalent Product User IDs from a list of external account IDs
	 * The values will be cached and retrievable through subsystem
	 * 
	 * @param ProductUserId - Local user's Product User ID
	 * @param AccountType - Type of external account (Steam, PSN, Xbox, etc.)
	 * @param ExternalAccountIds - Array of external account IDs to query
	 */
	UFUNCTION(BlueprintCallable, DisplayName="Query External Account Mappings", meta = (BlueprintInternalUseOnly = "true"), Category="EOSKit|Sessions")
	static UEOSQueryExternalAccountMappingsAsync* QueryExternalAccountMappings(
		FString ProductUserId,
		EEOSKitExternalAccountType AccountType,
		const TArray<FString>& ExternalAccountIds
	);

	virtual void Activate() override;

private:
	void QueryMappings();
	
	FString VarProductUserId;
	EEOSKitExternalAccountType VarAccountType;
	TArray<FString> VarExternalAccountIds;
};
