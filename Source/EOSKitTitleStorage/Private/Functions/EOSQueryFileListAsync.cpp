// Copyright (C) 2024, All Rights Reserved.

#include "Functions/EOSQueryFileListAsync.h"
#include "EOSKitSubsystem.h"
#include "Kismet/GameplayStatics.h"
#if WITH_EOS_SDK
#include "Windows/AllowWindowsPlatformTypes.h"
#include "eos_sdk.h"
#include "eos_titlestorage.h"
#include "eos_titlestorage_types.h"
#include "Windows/HideWindowsPlatformTypes.h"
#endif
#include "Async/Async.h"

UEOSQueryFileListAsync* UEOSQueryFileListAsync::QueryFileList(
	UObject* WorldContextObject,
	const FString& LocalUserId,
	const TArray<FString>& Tags)
{
	UEOSQueryFileListAsync* Node = NewObject<UEOSQueryFileListAsync>();
	Node->WorldContextObject = WorldContextObject;
	Node->UserId = LocalUserId;
	Node->FileTags = Tags;
	return Node;
}

void UEOSQueryFileListAsync::Activate()
{
	PerformQuery();
}

void UEOSQueryFileListAsync::PerformQuery()
{
	if (!WorldContextObject)
	{
		OnFail.Broadcast(TEXT("Invalid World Context"));
		SetReadyToDestroy();
		return;
	}

	UGameInstance* GameInstance = UGameplayStatics::GetGameInstance(WorldContextObject);
	if (!GameInstance)
	{
		OnFail.Broadcast(TEXT("Failed to get Game Instance"));
		SetReadyToDestroy();
		return;
	}

	UEOSKitSubsystem* EOSSubsystem = GameInstance->GetSubsystem<UEOSKitSubsystem>();
	if (!EOSSubsystem || !EOSSubsystem->GetPlatformHandle())
	{
		OnFail.Broadcast(TEXT("EOSKit Subsystem not initialized"));
		SetReadyToDestroy();
		return;
	}

	EOS_HPlatform PlatformHandle = EOSSubsystem->GetPlatformHandle();
	EOS_HTitleStorage TitleStorageHandle = EOS_Platform_GetTitleStorageInterface(PlatformHandle);

	if (!TitleStorageHandle)
	{
		OnFail.Broadcast(TEXT("Failed to get Title Storage interface"));
		SetReadyToDestroy();
		return;
	}

	// Convert user ID
	EOS_ProductUserId LocalPUID = EOS_ProductUserId_FromString(TCHAR_TO_UTF8(*UserId));
	if (!LocalPUID)
	{
		OnFail.Broadcast(TEXT("Invalid Product User ID"));
		SetReadyToDestroy();
		return;
	}

	// Convert tags to ANSI
	TArray<FTCHARToUTF8> TagConverters;
	TArray<const char*> TagPtrs;
	
	for (const FString& Tag : FileTags)
	{
		TagConverters.Emplace(*Tag);
	}
	
	for (const FTCHARToUTF8& Converter : TagConverters)
	{
		TagPtrs.Add(Converter.Get());
	}

	// Setup query options
	EOS_TitleStorage_QueryFileListOptions QueryOptions = {};
	QueryOptions.ApiVersion = EOS_TITLESTORAGE_QUERYFILELISTOPTIONS_API_LATEST;
	QueryOptions.LocalUserId = LocalPUID;
	QueryOptions.ListOfTags = TagPtrs.Num() > 0 ? TagPtrs.GetData() : nullptr;
	QueryOptions.ListOfTagsCount = TagPtrs.Num();

	UE_LOG(LogTemp, Log, TEXT("EOSKitTitleStorage: Querying file list"));

	// Perform query - use lambda wrapper
	EOS_TitleStorage_QueryFileList(TitleStorageHandle, &QueryOptions, this, [](const EOS_TitleStorage_QueryFileListCallbackInfo* Data)
	{
		if (!Data || !Data->ClientData)
		{
			return;
		}

		UEOSQueryFileListAsync* Self = static_cast<UEOSQueryFileListAsync*>(Data->ClientData);

		AsyncTask(ENamedThreads::GameThread, [Self, Data]()
		{
			if (Data->ResultCode == EOS_EResult::EOS_Success)
			{
				UGameInstance* GameInstance = UGameplayStatics::GetGameInstance(Self->WorldContextObject);
				if (GameInstance)
				{
					UEOSKitSubsystem* EOSSubsystem = GameInstance->GetSubsystem<UEOSKitSubsystem>();
					if (EOSSubsystem && EOSSubsystem->GetPlatformHandle())
					{
						EOS_HTitleStorage TitleStorageHandle = EOS_Platform_GetTitleStorageInterface(EOSSubsystem->GetPlatformHandle());
						if (TitleStorageHandle)
						{
							// Get file count
							EOS_TitleStorage_GetFileMetadataCountOptions CountOptions = {};
							CountOptions.ApiVersion = EOS_TITLESTORAGE_GETFILEMETADATACOUNTOPTIONS_API_LATEST;
							CountOptions.LocalUserId = Data->LocalUserId;

							uint32_t FileCount = EOS_TitleStorage_GetFileMetadataCount(TitleStorageHandle, &CountOptions);
							
							TArray<FEOSTitleStorageFileInfo> Files;
							Files.Reserve(FileCount);

							// Get each file metadata
							for (uint32_t i = 0; i < FileCount; i++)
							{
								EOS_TitleStorage_CopyFileMetadataAtIndexOptions CopyOptions = {};
								CopyOptions.ApiVersion = EOS_TITLESTORAGE_COPYFILEMETADATAATINDEXOPTIONS_API_LATEST;
								CopyOptions.LocalUserId = Data->LocalUserId;
								CopyOptions.Index = i;

								EOS_TitleStorage_FileMetadata* FileMetadata = nullptr;
								EOS_EResult CopyResult = EOS_TitleStorage_CopyFileMetadataAtIndex(TitleStorageHandle, &CopyOptions, &FileMetadata);

								if (CopyResult == EOS_EResult::EOS_Success && FileMetadata)
								{
									FEOSTitleStorageFileInfo FileInfo;
									
									if (FileMetadata->Filename)
									{
										FileInfo.Filename = UTF8_TO_TCHAR(FileMetadata->Filename);
									}
									if (FileMetadata->MD5Hash)
									{
										FileInfo.MD5Hash = UTF8_TO_TCHAR(FileMetadata->MD5Hash);
									}
									
									FileInfo.FileSizeBytes = FileMetadata->FileSizeBytes;

									Files.Add(FileInfo);

									// Release the file metadata
									EOS_TitleStorage_FileMetadata_Release(FileMetadata);
								}
							}

							UE_LOG(LogTemp, Log, TEXT("EOSKitTitleStorage: Query file list successful. Found %d files"), Files.Num());
							Self->OnSuccess.Broadcast(Files);
							Self->SetReadyToDestroy();
							return;
						}
					}
				}

				Self->OnFail.Broadcast(TEXT("Failed to retrieve file list"));
				Self->SetReadyToDestroy();
			}
			else
			{
				FString ErrorMessage = FString::Printf(TEXT("Query file list failed: %s"), 
					UTF8_TO_TCHAR(EOS_EResult_ToString(Data->ResultCode)));
				UE_LOG(LogTemp, Error, TEXT("EOSKitTitleStorage: %s"), *ErrorMessage);
				Self->OnFail.Broadcast(ErrorMessage);
				Self->SetReadyToDestroy();
			}
		});
	});
}
