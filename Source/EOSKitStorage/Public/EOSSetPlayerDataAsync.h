// Copyright (C) 2024, All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintAsyncActionBase.h"
#include "EOSSetPlayerDataAsync.generated.h"

#if WITH_EOS_SDK
#include "Windows/AllowWindowsPlatformTypes.h"
#include "eos_playerdatastorage_types.h"
#include "Windows/HideWindowsPlatformTypes.h"
#endif

/**
 * Delegate for set player data completion
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnEOSSetPlayerDataComplete);

/**
 * Async node to write player data to cloud storage
 * Uses PlayerDataStorage interface to write file data
 */
UCLASS()
class EOSKITSTORAGE_API UEOSSetPlayerDataAsync : public UBlueprintAsyncActionBase
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintAssignable, Category = "EOSKit|PlayerDataStorage")
	FOnEOSSetPlayerDataComplete OnSuccess;

	UPROPERTY(BlueprintAssignable, Category = "EOSKit|PlayerDataStorage")
	FOnEOSSetPlayerDataComplete OnFailure;

	/**
	 * Write player data to cloud storage
	 * @param WorldContextObject - World context object
	 * @param Filename - Name of the file to write
	 * @param DataToSave - Data to save (as byte array)
	 * @return Async node instance
	 */
	UFUNCTION(BlueprintCallable, DisplayName = "Set Player Data",
		meta = (BlueprintInternalUseOnly = "true", WorldContext = "WorldContextObject",
		ToolTip = "Write player data to cloud storage. Replaces existing file if it exists."),
		Category = "EOSKit|PlayerDataStorage")
	static UEOSSetPlayerDataAsync* SetPlayerData(UObject* WorldContextObject, const FString& Filename, const TArray<uint8>& DataToSave);

	virtual void Activate() override;

private:
	static EOS_PlayerDataStorage_EWriteResult EOS_CALL OnWriteFileDataCallback(
		const EOS_PlayerDataStorage_WriteFileDataCallbackInfo* Data,
		void* OutDataBuffer,
		uint32_t* OutDataWritten);

	static void EOS_CALL OnWriteFileComplete(const EOS_PlayerDataStorage_WriteFileCallbackInfo* Data);

	UObject* WorldContextObject;
	FString FilenameString;
	TArray<uint8> DataToWrite;
	int32 CurrentWriteOffset;
};

