// Copyright (C) 2024, All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintAsyncActionBase.h"
#include "EOSDeleteTicketDataAsync.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnDeleteTicketDataComplete,
	const FString&, ResponseStr,
	int32, HTTPResponseCode,
	bool, bWasSuccessful);

/**
 * Async node to delete ticket data using Epic Games Developer Portal API
 */
UCLASS()
class EOSKITSESSIONS_API UEOSDeleteTicketDataAsync : public UBlueprintAsyncActionBase
{
	GENERATED_BODY()

public:
	/**
	 * Delete ticket data
	 * @param WorldContextObject - World context
	 * @param Email - Email address of the user
	 * @param ApiKey - Epic Games Developer Portal API key (required)
	 * @return Async node instance
	 */
	UFUNCTION(BlueprintCallable, Category = "EOSKit|Support Tickets",
		meta = (BlueprintInternalUseOnly = "true", WorldContext = "WorldContextObject"),
		DisplayName = "Delete Ticket Data")
	static UEOSDeleteTicketDataAsync* DeleteTicketData(
		UObject* WorldContextObject,
		const FString& Email,
		const FString& ApiKey
	);

	UPROPERTY(BlueprintAssignable, Category = "EOSKit|Support Tickets")
	FOnDeleteTicketDataComplete OnSuccess;

	UPROPERTY(BlueprintAssignable, Category = "EOSKit|Support Tickets")
	FOnDeleteTicketDataComplete OnFailure;

private:
	virtual void Activate() override;

	void DeleteTicketDataRequest();
	void OnResponseReceived(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful);

	UPROPERTY()
	UObject* WorldContextObject;

	FString Email;
	FString ApiKey;
};

