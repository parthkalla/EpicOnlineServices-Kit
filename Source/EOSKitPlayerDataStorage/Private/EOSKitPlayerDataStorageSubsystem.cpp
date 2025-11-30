// Copyright (C) 2024, All Rights Reserved.

#include "EOSKitPlayerDataStorageSubsystem.h"
#include "EOSKitSubsystem.h"
#include "Kismet/GameplayStatics.h"
#if WITH_EOS_SDK
#include "Windows/AllowWindowsPlatformTypes.h"
#include "Windows/PreWindowsApi.h"
#include "eos_platform.h"
#include "eos_playerdatastorage.h"
#include "eos_playerdatastorage_types.h"
#include "Windows/PostWindowsApi.h"
#include "Windows/HideWindowsPlatformTypes.h"
#endif
#include "EOSKitSharedTypes.h"

void UEOSKitPlayerDataStorageSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	UE_LOG(LogTemp, Log, TEXT("EOSKitPlayerDataStorageSubsystem: Initialized"));
}

void UEOSKitPlayerDataStorageSubsystem::Deinitialize()
{
	Super::Deinitialize();
}

UEOSKitSubsystem* UEOSKitPlayerDataStorageSubsystem::GetEOSKitSubsystem() const
{
	if (UGameInstance* GameInstance = GetGameInstance())
	{
		return GameInstance->GetSubsystem<UEOSKitSubsystem>();
	}
	return nullptr;
}

EOS_HPlayerDataStorage UEOSKitPlayerDataStorageSubsystem::GetPlayerDataStorageHandle() const
{
	UEOSKitSubsystem* EOSKitSubsystem = GetEOSKitSubsystem();
	if (!EOSKitSubsystem || !EOSKitSubsystem->GetPlatformHandle())
	{
		return nullptr;
	}
	return EOS_Platform_GetPlayerDataStorageInterface(EOSKitSubsystem->GetPlatformHandle());
}

EEOSResult UEOSKitPlayerDataStorageSubsystem::QueryFileList(const FEOSKitProductUserId& LocalUserId)
{
	EOS_HPlayerDataStorage PlayerDataStorageHandle = GetPlayerDataStorageHandle();
	if (!PlayerDataStorageHandle)
	{
		UE_LOG(LogTemp, Error, TEXT("EOSKitPlayerDataStorage: Failed to get PlayerDataStorage Handle"));
		return EEOSResult::EOS_NotConfigured;
	}

	EOS_PlayerDataStorage_QueryFileListOptions Options = {};
	Options.ApiVersion = EOS_PLAYERDATASTORAGE_QUERYFILELIST_API_LATEST;
	Options.LocalUserId = LocalUserId.GetValueAsEosType();

	// Note: This is a synchronous wrapper, but QueryFileList is async
	// The actual async operation should be done through async nodes
	return EEOSResult::EOS_Success;
}

int32 UEOSKitPlayerDataStorageSubsystem::GetFileMetadataCount(const FEOSKitProductUserId& LocalUserId)
{
	EOS_HPlayerDataStorage PlayerDataStorageHandle = GetPlayerDataStorageHandle();
	if (!PlayerDataStorageHandle)
	{
		UE_LOG(LogTemp, Error, TEXT("EOSKitPlayerDataStorage: Failed to get PlayerDataStorage Handle"));
		return 0;
	}

	EOS_PlayerDataStorage_GetFileMetadataCountOptions Options = {};
	Options.ApiVersion = EOS_PLAYERDATASTORAGE_GETFILEMETADATACOUNT_API_LATEST;
	Options.LocalUserId = LocalUserId.GetValueAsEosType();

	int32_t OutFileMetadataCount = 0;
	EOS_EResult Result = EOS_PlayerDataStorage_GetFileMetadataCount(PlayerDataStorageHandle, &Options, &OutFileMetadataCount);
	
	if (Result == EOS_EResult::EOS_Success)
	{
		return static_cast<int32>(OutFileMetadataCount);
	}

	return 0;
}

bool UEOSKitPlayerDataStorageSubsystem::CopyFileMetadataAtIndex(const FEOSKitProductUserId& LocalUserId, int32 Index, FEOSKitFileMetadata& OutFileMetadata)
{
	EOS_HPlayerDataStorage PlayerDataStorageHandle = GetPlayerDataStorageHandle();
	if (!PlayerDataStorageHandle)
	{
		UE_LOG(LogTemp, Error, TEXT("EOSKitPlayerDataStorage: Failed to get PlayerDataStorage Handle"));
		return false;
	}

	EOS_PlayerDataStorage_CopyFileMetadataAtIndexOptions Options = {};
	Options.ApiVersion = EOS_PLAYERDATASTORAGE_COPYFILEMETADATAATINDEX_API_LATEST;
	Options.LocalUserId = LocalUserId.GetValueAsEosType();
	Options.Index = Index;

	EOS_PlayerDataStorage_FileMetadata* FileMetadata = nullptr;
	EOS_EResult Result = EOS_PlayerDataStorage_CopyFileMetadataAtIndex(PlayerDataStorageHandle, &Options, &FileMetadata);

	if (Result == EOS_EResult::EOS_Success && FileMetadata)
	{
		OutFileMetadata.Filename = FileMetadata->Filename ? UTF8_TO_TCHAR(FileMetadata->Filename) : TEXT("");
		OutFileMetadata.FileSizeBytes = FileMetadata->FileSizeBytes;
		OutFileMetadata.MD5Hash = FileMetadata->MD5Hash ? UTF8_TO_TCHAR(FileMetadata->MD5Hash) : TEXT("");
		
		// Convert POSIX timestamp to FDateTime
		if (FileMetadata->LastModifiedTime != EOS_PLAYERDATASTORAGE_TIME_UNDEFINED)
		{
			OutFileMetadata.LastModifiedTime = FDateTime::FromUnixTimestamp(FileMetadata->LastModifiedTime);
		}

		EOS_PlayerDataStorage_FileMetadata_Release(FileMetadata);
		return true;
	}

	return false;
}

bool UEOSKitPlayerDataStorageSubsystem::CopyFileMetadataByFilename(const FEOSKitProductUserId& LocalUserId, const FString& Filename, FEOSKitFileMetadata& OutFileMetadata)
{
	EOS_HPlayerDataStorage PlayerDataStorageHandle = GetPlayerDataStorageHandle();
	if (!PlayerDataStorageHandle)
	{
		UE_LOG(LogTemp, Error, TEXT("EOSKitPlayerDataStorage: Failed to get PlayerDataStorage Handle"));
		return false;
	}

	EOS_PlayerDataStorage_CopyFileMetadataByFilenameOptions Options = {};
	Options.ApiVersion = EOS_PLAYERDATASTORAGE_COPYFILEMETADATABYFILENAME_API_LATEST;
	Options.LocalUserId = LocalUserId.GetValueAsEosType();
	Options.Filename = TCHAR_TO_UTF8(*Filename);

	EOS_PlayerDataStorage_FileMetadata* FileMetadata = nullptr;
	EOS_EResult Result = EOS_PlayerDataStorage_CopyFileMetadataByFilename(PlayerDataStorageHandle, &Options, &FileMetadata);

	if (Result == EOS_EResult::EOS_Success && FileMetadata)
	{
		OutFileMetadata.Filename = FileMetadata->Filename ? UTF8_TO_TCHAR(FileMetadata->Filename) : TEXT("");
		OutFileMetadata.FileSizeBytes = FileMetadata->FileSizeBytes;
		OutFileMetadata.MD5Hash = FileMetadata->MD5Hash ? UTF8_TO_TCHAR(FileMetadata->MD5Hash) : TEXT("");
		
		// Convert POSIX timestamp to FDateTime
		if (FileMetadata->LastModifiedTime != EOS_PLAYERDATASTORAGE_TIME_UNDEFINED)
		{
			OutFileMetadata.LastModifiedTime = FDateTime::FromUnixTimestamp(FileMetadata->LastModifiedTime);
		}

		EOS_PlayerDataStorage_FileMetadata_Release(FileMetadata);
		return true;
	}

	return false;
}

