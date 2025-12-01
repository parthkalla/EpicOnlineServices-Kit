// Copyright (C) 2024, All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintAsyncActionBase.h"
#include "EOSKitSharedTypes.h"
#include "EOSSendReportAsync.generated.h"

/**
 * Player report category enumeration
 */
UENUM(BlueprintType)
enum class EEOSKitPlayerReportCategory : uint8
{
	Invalid = 0			UMETA(DisplayName = "Invalid"),
	Cheating = 1		UMETA(DisplayName = "Cheating"),
	Exploiting = 2		UMETA(DisplayName = "Exploiting"),
	OffensiveProfile = 3	UMETA(DisplayName = "Offensive Profile"),
	VerbalAbuse = 4		UMETA(DisplayName = "Verbal Abuse"),
	Scamming = 5		UMETA(DisplayName = "Scamming"),
	Spamming = 6		UMETA(DisplayName = "Spamming"),
	Other = 7			UMETA(DisplayName = "Other")
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnSendReportComplete, EEOSResult, Result);

/**
 * Async node to send a player behavior report using EOS SDK
 */
UCLASS()
class EOSKITSESSIONS_API UEOSSendReportAsync : public UBlueprintAsyncActionBase
{
	GENERATED_BODY()

public:
	/**
	 * Send a player behavior report
	 * @param WorldContextObject - World context
	 * @param ReporterUserId - Product User ID of the reporting player
	 * @param ReportedUserId - Product User ID of the reported player
	 * @param Category - Category for the player report
	 * @param Message - Optional message describing the issue (max 512 characters)
	 * @param Context - Optional JSON context data (max 4096 characters, must be valid JSON)
	 * @return Async node instance
	 */
	UFUNCTION(BlueprintCallable, Category = "EOSKit|Reports",
		meta = (BlueprintInternalUseOnly = "true", WorldContext = "WorldContextObject"),
		DisplayName = "Send Player Behavior Report")
	static UEOSSendReportAsync* SendReport(
		UObject* WorldContextObject,
		const FEOSKitProductUserId& ReporterUserId,
		const FEOSKitProductUserId& ReportedUserId,
		EEOSKitPlayerReportCategory Category,
		const FString& Message = TEXT(""),
		const FString& Context = TEXT("{}")
	);

	UPROPERTY(BlueprintAssignable, Category = "EOSKit|Reports")
	FOnSendReportComplete OnComplete;

private:
	virtual void Activate() override;

#if WITH_EOS_SDK
	static void EOS_CALL OnSendReportCompleteCallback(const void* Data);
#endif

	UPROPERTY()
	UObject* WorldContextObject;

	FEOSKitProductUserId ReporterUserId;
	FEOSKitProductUserId ReportedUserId;
	EEOSKitPlayerReportCategory Category;
	FString Message;
	FString Context;
};

