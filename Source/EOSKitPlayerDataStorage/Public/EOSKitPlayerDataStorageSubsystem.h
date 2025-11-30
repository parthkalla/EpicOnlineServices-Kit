// Copyright (C) 2024, All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "EOSKitPlayerDataStorageTypes.h"
#include "EOSKitSharedTypes.h"
#include "EOSKitSubsystem.h"

THIRD_PARTY_INCLUDES_START
#include "eos_playerdatastorage.h"
#include "eos_playerdatastorage_types.h"
THIRD_PARTY_INCLUDES_END

// .generated.h must always be the last include
#include "EOSKitPlayerDataStorageSubsystem.generated.h"

/**
 * Player Data Storage subsystem for managing EOS PlayerDataStorage interface
 */
UCLASS()
class EOSKITPLAYERDATASTORAGE_API UEOSKitPlayerDataStorageSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	// USubsystem interface
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	// ========================================
	// SDK Function Wrappers
	// ========================================

	/**
	 * Query file list for a user
	 * @param LocalUserId - Product User ID of the local user
	 * @return Result code (Note: This is async, returns immediately)
	 */
	UFUNCTION(BlueprintCallable, Category = "EOSKit|PlayerDataStorage")
	EEOSResult QueryFileList(const FEOSKitProductUserId& LocalUserId);

	/**
	 * Get file metadata count
	 * @param LocalUserId - Product User ID of the local user
	 * @return Number of files (0 if error)
	 */
	UFUNCTION(BlueprintCallable, Category = "EOSKit|PlayerDataStorage")
	int32 GetFileMetadataCount(const FEOSKitProductUserId& LocalUserId);

	/**
	 * Copy file metadata at index
	 * @param LocalUserId - Product User ID of the local user
	 * @param Index - Index of the file
	 * @param OutFileMetadata - Output file metadata
	 * @return True if successful
	 */
	UFUNCTION(BlueprintCallable, Category = "EOSKit|PlayerDataStorage")
	bool CopyFileMetadataAtIndex(const FEOSKitProductUserId& LocalUserId, int32 Index, FEOSKitFileMetadata& OutFileMetadata);

	/**
	 * Copy file metadata by filename
	 * @param LocalUserId - Product User ID of the local user
	 * @param Filename - Name of the file
	 * @param OutFileMetadata - Output file metadata
	 * @return True if successful
	 */
	UFUNCTION(BlueprintCallable, Category = "EOSKit|PlayerDataStorage")
	bool CopyFileMetadataByFilename(const FEOSKitProductUserId& LocalUserId, const FString& Filename, FEOSKitFileMetadata& OutFileMetadata);

private:
	UEOSKitSubsystem* GetEOSKitSubsystem() const;
	EOS_HPlayerDataStorage GetPlayerDataStorageHandle() const;
};

