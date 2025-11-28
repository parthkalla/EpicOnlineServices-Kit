// Copyright (C) 2024, All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintAsyncActionBase.h"
#include "EOSReportPlayerAsync.generated.h"

UENUM(BlueprintType)
enum class EEOSPlayerReportCategory : uint8
{
	Invalid UMETA(DisplayName = "Invalid"),
	Cheating UMETA(DisplayName = "Cheating"),
	Exploiting UMETA(DisplayName = "Exploiting"),
	OffensiveProfile UMETA(DisplayName = "Offensive Profile"),
	VerbalAbuse UMETA(DisplayName = "Verbal Abuse"),
	Scamming UMETA(DisplayName = "Scamming"),
	Spamming UMETA(DisplayName = "Spamming"),
	Other UMETA(DisplayName = "Other")
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnReportPlayerComplete);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnReportPlayerFail, const FString&, ErrorMessage);

/**
 * Report a player for behavior violation
 */
UCLASS()
class EOSKITWEB_API UEOSReportPlayerAsync : public UBlueprintAsyncActionBase
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintAssignable)
	FOnReportPlayerComplete OnSuccess;

	UPROPERTY(BlueprintAssignable)
	FOnReportPlayerFail OnFail;

	/**
	 * Send a player behavior report to EOS
	 * 
	 * @param WorldContextObject World context
	 * @param ReporterUserId The Product User ID of the player making the report
	 * @param ReportedUserId The Product User ID of the player being reported
	 * @param Category The category of the report
	 * @param Message Optional message describing the issue
	 * @param Context Optional JSON context data
	 */
	UFUNCTION(BlueprintCallable, meta = (BlueprintInternalUseOnly = "true", WorldContext = "WorldContextObject"), Category = "EOSKit|Web|PlayerReports")
	static UEOSReportPlayerAsync* ReportPlayer(
		UObject* WorldContextObject,
		const FString& ReporterUserId,
		const FString& ReportedUserId,
		EEOSPlayerReportCategory Category,
		const FString& Message = TEXT(""),
		const FString& Context = TEXT("")
	);

	virtual void Activate() override;

	void PerformReport();

	UObject* WorldContextObject;
	FString ReporterId;
	FString ReportedId;
	EEOSPlayerReportCategory ReportCategory;
	FString ReportMessage;
	FString ReportContext;

	// Helper to convert enum to EOS type
	static uint32_t ConvertReportCategory(EEOSPlayerReportCategory Category);
};
