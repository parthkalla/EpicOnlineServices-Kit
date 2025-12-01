// Copyright (C) 2024, All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "EOSKitSharedTypes.h"
#include "EOSKitPlayerDataStorageTypes.generated.h"

/**
 * File metadata information
 */
USTRUCT(BlueprintType, Category = "EOSKit|PlayerDataStorage")
struct EOSKITSTORAGE_API FEOSKitFileMetadata
{
	GENERATED_BODY()

	/** The name of the file */
	UPROPERTY(BlueprintReadWrite, Category = "EOSKit|PlayerDataStorage")
	FString Filename = TEXT("");

	/** The total size of the file in bytes */
	UPROPERTY(BlueprintReadWrite, Category = "EOSKit|PlayerDataStorage")
	int32 FileSizeBytes = 0;

	/** The MD5 hash of the file */
	UPROPERTY(BlueprintReadWrite, Category = "EOSKit|PlayerDataStorage")
	FString MD5Hash = TEXT("");

	/** The time when the file was last modified */
	UPROPERTY(BlueprintReadWrite, Category = "EOSKit|PlayerDataStorage")
	FDateTime LastModifiedTime;
};

