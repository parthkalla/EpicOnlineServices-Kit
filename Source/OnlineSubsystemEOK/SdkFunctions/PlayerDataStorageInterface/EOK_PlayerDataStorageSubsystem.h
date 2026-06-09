// Copyright (c) 2023 Betide Studio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "OnlineSubsystemEOK/SdkFunctions/EOK_SharedFunctionFile.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "EOK_PlayerDataStorageSubsystem.generated.h"

/**
 * 
 */
UCLASS(DisplayName="Player Data Storage Interface", meta=(DisplayName="Player Data Storage Interface"))
class OnlineSubsystemEOK_API UEOK_PlayerDataStorageSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:

	//Get the cached copy of a file's metadata by index. The metadata will be for the last retrieved or successfully saved version, and will not include any local changes that have not been committed by calling SaveFile. The returned pointer must be released by the user when no longer needed.
	UFUNCTION(BlueprintCallable, Category = "Epic Online Services-Kit V2 | SDK Functions | Player Data Storage Interface", DisplayName="EOS_PlayerDataStorage_CopyFileMetadataAtIndex")
	TEnumAsByte<EEOK_Result>  EOK_PlayerDataStorage_CopyFileMetadataAtIndex(FEOK_ProductUserId LocalUserId, int32 Index, FEOK_PlayerDataStorage_FileMetadata& OutMetadata);

	//Create the cached copy of a file's metadata by filename. The metadata will be for the last retrieved or successfully saved version, and will not include any changes that have not completed writing. The returned pointer must be released by the user when no longer needed.
	UFUNCTION(BlueprintCallable, Category = "Epic Online Services-Kit V2 | SDK Functions | Player Data Storage Interface", DisplayName="EOS_PlayerDataStorage_CopyFileMetadataByFilename")
	TEnumAsByte<EEOK_Result>  EOK_PlayerDataStorage_CopyFileMetadataByFilename(FEOK_ProductUserId LocalUserId, FString Filename, FEOK_PlayerDataStorage_FileMetadata& OutMetadata);

	//Free the memory used by the file metadata
	UFUNCTION(BlueprintCallable, Category = "Epic Online Services-Kit V2 | SDK Functions | Player Data Storage Interface", DisplayName="EOS_PlayerDataStorage_FileMetadata_Release")
	void EOK_PlayerDataStorage_FileMetadata_Release(FEOK_PlayerDataStorage_FileMetadata& Metadata);

	//Get the count of files we have previously queried information for and files we have previously read from / written to.
	UFUNCTION(BlueprintCallable, Category = "Epic Online Services-Kit V2 | SDK Functions | Player Data Storage Interface", DisplayName="EOS_PlayerDataStorage_GetFileMetadataCount")
	TEnumAsByte<EEOK_Result>  EOK_PlayerDataStorage_GetFileMetadataCount(FEOK_ProductUserId LocalUserId, int32& OutFileMetadataCount);

	//Get the file name of the file this request is for. OutStringLength will always be set to the string length of the file name if it is not NULL.
	UFUNCTION(BlueprintCallable, Category = "Epic Online Services-Kit V2 | SDK Functions | Player Data Storage Interface", DisplayName="EOS_PlayerDataStorageFileTransferRequest_GetFilename")
	TEnumAsByte<EEOK_Result>  EOK_PlayerDataStorageFileTransferRequest_GetFilename(FEOK_HPlayerDataStorageFileTransferRequest TransferRequestHandle, FString& OutFilename);

	//Get the current state of a file request.
	UFUNCTION(BlueprintCallable, Category = "Epic Online Services-Kit V2 | SDK Functions | Player Data Storage Interface", DisplayName="EOS_PlayerDataStorageFileTransferRequest_GetFileRequestState")
	TEnumAsByte<EEOK_Result>  EOK_PlayerDataStorageFileTransferRequest_GetFileRequestState(FEOK_HPlayerDataStorageFileTransferRequest TransferRequestHandle);

	//Free the memory used by a cloud-storage file request handle. This will not cancel a request in progress.
	UFUNCTION(BlueprintCallable, Category = "Epic Online Services-Kit V2 | SDK Functions | Player Data Storage Interface", DisplayName="EOS_PlayerDataStorageFileTransferRequest_Release")
	void EOK_PlayerDataStorageFileTransferRequest_Release(FEOK_HPlayerDataStorageFileTransferRequest TransferRequestHandle);
};
