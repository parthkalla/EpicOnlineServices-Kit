// Copyright (C) 2024, All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintAsyncActionBase.h"
#include "EOSSendSupportTicketAsync.generated.h"

/**
 * Support ticket subject enumeration
 */
UENUM(BlueprintType)
enum class EEOSKitSupportTicketSubject : uint8
{
	OpenQuestion = 0			UMETA(DisplayName = "Open Question"),
	TechnicalSupport = 1		UMETA(DisplayName = "Technical Support"),
	BanAppeal = 2			UMETA(DisplayName = "Ban Appeal")
};

/**
 * Support ticket data structure
 */
USTRUCT(BlueprintType)
struct FEOSKitSupportTicketData
{
	GENERATED_BODY()

	/** Subject of the support ticket (required) */
	UPROPERTY(BlueprintReadWrite, Category = "EOSKit|Support Tickets")
	EEOSKitSupportTicketSubject Subject = EEOSKitSupportTicketSubject::OpenQuestion;

	/** Message or description for the support ticket (required) */
	UPROPERTY(BlueprintReadWrite, Category = "EOSKit|Support Tickets")
	FString Message = TEXT("");

	/** Email address of the sender (required) */
	UPROPERTY(BlueprintReadWrite, Category = "EOSKit|Support Tickets")
	FString SenderEmail = TEXT("");

	/** Name of the sender */
	UPROPERTY(BlueprintReadWrite, Category = "EOSKit|Support Tickets")
	FString SenderName = TEXT("");

	/** Unique identifier for the end-user */
	UPROPERTY(BlueprintReadWrite, Category = "EOSKit|Support Tickets")
	FString Guid = TEXT("");

	/** Error code for technical support emails */
	UPROPERTY(BlueprintReadWrite, Category = "EOSKit|Support Tickets")
	FString ErrorCode = TEXT("");

	/** Operating system of the end-user */
	UPROPERTY(BlueprintReadWrite, Category = "EOSKit|Support Tickets")
	FString SystemOS = TEXT("");

	/** Anti-malware system of the end-user */
	UPROPERTY(BlueprintReadWrite, Category = "EOSKit|Support Tickets")
	FString SystemAntiMalware = TEXT("");

	/** Other information about the end-user's system */
	UPROPERTY(BlueprintReadWrite, Category = "EOSKit|Support Tickets")
	FString SystemOther = TEXT("");
};

/**
 * Support ticket response data structure
 */
USTRUCT(BlueprintType)
struct FEOSKitSupportTicketResponseData
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "EOSKit|Support Tickets")
	FString ProdName = TEXT("");

	UPROPERTY(BlueprintReadOnly, Category = "EOSKit|Support Tickets")
	FString ProdSlug = TEXT("");

	UPROPERTY(BlueprintReadOnly, Category = "EOSKit|Support Tickets")
	FString Guid = TEXT("");

	UPROPERTY(BlueprintReadOnly, Category = "EOSKit|Support Tickets")
	FString SenderName = TEXT("");

	UPROPERTY(BlueprintReadOnly, Category = "EOSKit|Support Tickets")
	FString SenderEmail = TEXT("");

	UPROPERTY(BlueprintReadOnly, Category = "EOSKit|Support Tickets")
	FString Subject = TEXT("");

	UPROPERTY(BlueprintReadOnly, Category = "EOSKit|Support Tickets")
	FString Message = TEXT("");

	UPROPERTY(BlueprintReadOnly, Category = "EOSKit|Support Tickets")
	FString ErrorCode = TEXT("");

	UPROPERTY(BlueprintReadOnly, Category = "EOSKit|Support Tickets")
	FString SystemOS = TEXT("");

	UPROPERTY(BlueprintReadOnly, Category = "EOSKit|Support Tickets")
	FString SystemAntiMalware = TEXT("");

	UPROPERTY(BlueprintReadOnly, Category = "EOSKit|Support Tickets")
	FString SystemOther = TEXT("");

	UPROPERTY(BlueprintReadOnly, Category = "EOSKit|Support Tickets")
	FString Timestamp = TEXT("");
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnSendSupportTicketComplete,
	const FString&, ResponseStr,
	const FEOSKitSupportTicketResponseData&, ResponseData,
	int32, HTTPResponseCode);

/**
 * Async node to send a support ticket using Epic Games Developer Portal API
 */
UCLASS()
class EOSKITSESSIONS_API UEOSSendSupportTicketAsync : public UBlueprintAsyncActionBase
{
	GENERATED_BODY()

public:
	/**
	 * Send a support ticket
	 * @param WorldContextObject - World context
	 * @param TicketData - Support ticket data
	 * @param ApiKey - Epic Games Developer Portal API key (required)
	 * @return Async node instance
	 */
	UFUNCTION(BlueprintCallable, Category = "EOSKit|Support Tickets",
		meta = (BlueprintInternalUseOnly = "true", WorldContext = "WorldContextObject"),
		DisplayName = "Send Support Ticket")
	static UEOSSendSupportTicketAsync* SendSupportTicket(
		UObject* WorldContextObject,
		const FEOSKitSupportTicketData& TicketData,
		const FString& ApiKey
	);

	UPROPERTY(BlueprintAssignable, Category = "EOSKit|Support Tickets")
	FOnSendSupportTicketComplete OnSuccess;

	UPROPERTY(BlueprintAssignable, Category = "EOSKit|Support Tickets")
	FOnSendSupportTicketComplete OnFailure;

private:
	virtual void Activate() override;

	void SendTicketRequest();
	void OnResponseReceived(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful);

	UPROPERTY()
	UObject* WorldContextObject;

	FEOSKitSupportTicketData TicketData;
	FString ApiKey;
};

