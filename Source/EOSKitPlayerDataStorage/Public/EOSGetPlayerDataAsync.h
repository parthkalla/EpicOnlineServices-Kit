// Copyright (C) 2024, All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintAsyncActionBase.h"
#include "EOSGetPlayerDataAsync.generated.h"

#if WITH_EOS_SDK
#include "Windows/AllowWindowsPlatformTypes.h"
#include "eos_playerdatastorage_types.h"
#include "Windows/HideWindowsPlatformTypes.h"
#endif

/**
 * Delegate for get player data completion
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnEOSGetPlayerDataComplete, bool, bWasSuccess, const TArray<uint8>&, SavedData);

/**
 * Async node to read player data from cloud storage
 * Uses PlayerDataStorage interface to read file data
 */
UCLASS()
class EOSKITPLAYERDATASTORAGE_API UEOSGetPlayerDataAsync : public UBlueprintAsyncActionBase
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintAssignable, Category = "EOSKit|PlayerDataStorage")
	FOnEOSGetPlayerDataComplete OnSuccess;

	UPROPERTY(BlueprintAssignable, Category = "EOSKit|PlayerDataStorage")
	FOnEOSGetPlayerDataComplete OnFailure;

	/**
	 * Read player data from cloud storage
	 * @param WorldContextObject - World context object
	 * @param Filename - Name of the file to read
	 * @return Async node instance
	 */
	UFUNCTION(BlueprintCallable, DisplayName = "Get Player Data",
		meta = (BlueprintInternalUseOnly = "true", WorldContext = "WorldContextObject",
		ToolTip = "Read player data from cloud storage. Returns file contents as byte array."),
		Category = "EOSKit|PlayerDataStorage")
	static UEOSGetPlayerDataAsync* GetPlayerData(UObject* WorldContextObject, const FString& Filename);

	virtual void Activate() override;

private:
	static EOS_PlayerDataStorage_EReadResult EOS_CALL OnReadFileDataCallback(
		const EOS_PlayerDataStorage_ReadFileDataCallbackInfo* Data);

	static void EOS_CALL OnReadFileComplete(const EOS_PlayerDataStorage_ReadFileCallbackInfo* Data);

	UObject* WorldContextObject;
	FString FilenameString;
	TArray<uint8> ReadData;
};

