// Copyright (C) 2024, All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintAsyncActionBase.h"
#include "EOSReadFileAsync.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnReadFileComplete, const FString&, FileContent, int32, FileSizeBytes);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnReadFileFail, const FString&, ErrorMessage);

/**
 * Read a file from EOS Title Storage
 */
UCLASS()
class EOSKITTITLESTORAGE_API UEOSReadFileAsync : public UBlueprintAsyncActionBase
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintAssignable)
	FOnReadFileComplete OnSuccess;

	UPROPERTY(BlueprintAssignable)
	FOnReadFileFail OnFail;

	/**
	 * Read a file from EOS Title Storage
	 * 
	 * @param WorldContextObject World context
	 * @param LocalUserId The Product User ID of the local player
	 * @param Filename The name of the file to read
	 */
	UFUNCTION(BlueprintCallable, meta = (BlueprintInternalUseOnly = "true", WorldContext = "WorldContextObject"), Category = "EOSKit|TitleStorage")
	static UEOSReadFileAsync* ReadFile(
		UObject* WorldContextObject,
		const FString& LocalUserId,
		const FString& Filename
	);

	virtual void Activate() override;

	void PerformRead();

	UObject* WorldContextObject;
	FString UserId;
	FString FileName;
	TArray<uint8> FileDataBuffer;
	int32 TotalBytesReceived;
};
