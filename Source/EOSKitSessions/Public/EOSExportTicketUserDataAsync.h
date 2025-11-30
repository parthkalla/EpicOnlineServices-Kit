// Copyright (C) 2024, All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintAsyncActionBase.h"
#include "EOSExportTicketUserDataAsync.generated.h"

/**
 * Message data structure
 */
USTRUCT(BlueprintType)
struct FEOSKitTicketMessageData
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "EOSKit|Support Tickets")
	int32 TicketId = 0;

	UPROPERTY(BlueprintReadOnly, Category = "EOSKit|Support Tickets")
	FString Message = TEXT("");

	UPROPERTY(BlueprintReadOnly, Category = "EOSKit|Support Tickets")
	FString SenderName = TEXT("");

	UPROPERTY(BlueprintReadOnly, Category = "EOSKit|Support Tickets")
	FString SenderEmail = TEXT("");

	UPROPERTY(BlueprintReadOnly, Category = "EOSKit|Support Tickets")
	FString Timestamp = TEXT("");
};

/**
 * Conversation data structure
 */
USTRUCT(BlueprintType)
struct FEOSKitTicketConversationData
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "EOSKit|Support Tickets")
	FString Guid = TEXT("");

	UPROPERTY(BlueprintReadOnly, Category = "EOSKit|Support Tickets")
	FString Subject = TEXT("");

	UPROPERTY(BlueprintReadOnly, Category = "EOSKit|Support Tickets")
	FString Message = TEXT("");

	UPROPERTY(BlueprintReadOnly, Category = "EOSKit|Support Tickets")
	FString SenderName = TEXT("");

	UPROPERTY(BlueprintReadOnly, Category = "EOSKit|Support Tickets")
	FString SenderEmail = TEXT("");

	UPROPERTY(BlueprintReadOnly, Category = "EOSKit|Support Tickets")
	FString Timestamp = TEXT("");

	UPROPERTY(BlueprintReadOnly, Category = "EOSKit|Support Tickets")
	TArray<FEOSKitTicketMessageData> Messages;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnExportTicketUserDataComplete,
	const FString&, ResponseStr,
	const TArray<FEOSKitTicketConversationData>&, Conversations,
	int32, HTTPResponseCode);

/**
 * Async node to export ticket user data using Epic Games Developer Portal API
 */
UCLASS()
class EOSKITSESSIONS_API UEOSExportTicketUserDataAsync : public UBlueprintAsyncActionBase
{
	GENERATED_BODY()

public:
	/**
	 * Export ticket user data
	 * @param WorldContextObject - World context
	 * @param Email - Email address of the user
	 * @param ApiKey - Epic Games Developer Portal API key (required)
	 * @return Async node instance
	 */
	UFUNCTION(BlueprintCallable, Category = "EOSKit|Support Tickets",
		meta = (BlueprintInternalUseOnly = "true", WorldContext = "WorldContextObject"),
		DisplayName = "Export Ticket User Data")
	static UEOSExportTicketUserDataAsync* ExportTicketUserData(
		UObject* WorldContextObject,
		const FString& Email,
		const FString& ApiKey
	);

	UPROPERTY(BlueprintAssignable, Category = "EOSKit|Support Tickets")
	FOnExportTicketUserDataComplete OnSuccess;

	UPROPERTY(BlueprintAssignable, Category = "EOSKit|Support Tickets")
	FOnExportTicketUserDataComplete OnFailure;

private:
	virtual void Activate() override;

	void ExportTicketData();
	void OnResponseReceived(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful);

	UPROPERTY()
	UObject* WorldContextObject;

	FString Email;
	FString ApiKey;
};

