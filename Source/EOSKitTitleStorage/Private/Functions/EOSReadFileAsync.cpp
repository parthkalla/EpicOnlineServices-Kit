// Copyright (C) 2024, All Rights Reserved.

#include "Functions/EOSReadFileAsync.h"
#include "EOSKitSubsystem.h"
#include "Kismet/GameplayStatics.h"
#include "eos_titlestorage.h"
#include "eos_titlestorage_types.h"
#include "Async/Async.h"

UEOSReadFileAsync* UEOSReadFileAsync::ReadFile(
	UObject* WorldContextObject,
	const FString& LocalUserId,
	const FString& Filename)
{
	UEOSReadFileAsync* Node = NewObject<UEOSReadFileAsync>();
	Node->WorldContextObject = WorldContextObject;
	Node->UserId = LocalUserId;
	Node->FileName = Filename;
	Node->TotalBytesReceived = 0;
	return Node;
}

void UEOSReadFileAsync::Activate()
{
	PerformRead();
}

void UEOSReadFileAsync::PerformRead()
{
	if (!WorldContextObject)
	{
		OnFail.Broadcast(TEXT("Invalid World Context"));
		SetReadyToDestroy();
		return;
	}

	if (FileName.IsEmpty())
	{
		OnFail.Broadcast(TEXT("Filename is empty"));
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

	// Convert filename
	FTCHARToUTF8 FilenameConverter(*FileName);

	// Setup read file options with callbacks
	EOS_TitleStorage_ReadFileOptions ReadOptions = {};
	ReadOptions.ApiVersion = EOS_TITLESTORAGE_READFILEOPTIONS_API_LATEST;
	ReadOptions.LocalUserId = LocalPUID;
	ReadOptions.Filename = FilenameConverter.Get();

	// Read file data callback - receives chunks of data
	ReadOptions.ReadFileDataCallback = [](const EOS_TitleStorage_ReadFileDataCallbackInfo* Data) -> EOS_TitleStorage_EReadResult
	{
		if (!Data || !Data->ClientData)
		{
			return EOS_TitleStorage_EReadResult::EOS_TS_RR_FailRequest;
		}

		UEOSReadFileAsync* Self = static_cast<UEOSReadFileAsync*>(Data->ClientData);

		// Append chunk to buffer
		if (Data->DataChunk && Data->DataChunkLengthBytes > 0)
		{
			const uint8* ChunkData = static_cast<const uint8*>(Data->DataChunk);
			Self->FileDataBuffer.Append(ChunkData, Data->DataChunkLengthBytes);
			Self->TotalBytesReceived += Data->DataChunkLengthBytes;
			
			UE_LOG(LogTemp, Verbose, TEXT("EOSKitTitleStorage: Received chunk %d bytes (Total: %d bytes)"), 
				Data->DataChunkLengthBytes, Self->TotalBytesReceived);
		}

		return EOS_TitleStorage_EReadResult::EOS_TS_RR_ContinueReading;
	};

	// File transfer progress callback
	ReadOptions.FileTransferProgressCallback = [](const EOS_TitleStorage_FileTransferProgressCallbackInfo* Data)
	{
		if (Data)
		{
			UE_LOG(LogTemp, Verbose, TEXT("EOSKitTitleStorage: Transfer progress %d / %d bytes"), 
				Data->BytesTransferred, Data->TotalFileSizeBytes);
		}
	};

	UE_LOG(LogTemp, Log, TEXT("EOSKitTitleStorage: Reading file: %s"), *FileName);

	// Perform read - use lambda wrapper for completion callback
	EOS_TitleStorage_ReadFile(TitleStorageHandle, &ReadOptions, this, [](const EOS_TitleStorage_ReadFileCallbackInfo* Data)
	{
		if (!Data || !Data->ClientData)
		{
			return;
		}

		UEOSReadFileAsync* Self = static_cast<UEOSReadFileAsync*>(Data->ClientData);

		AsyncTask(ENamedThreads::GameThread, [Self, Data]()
		{
			if (Data->ResultCode == EOS_EResult::EOS_Success)
			{
				// Convert buffer to string (assume UTF-8 text content)
				FString FileContent;
				
				if (Self->FileDataBuffer.Num() > 0)
				{
					// Add null terminator for string conversion
					Self->FileDataBuffer.Add(0);
					
					// Convert UTF-8 to FString
					FUTF8ToTCHAR Converter(reinterpret_cast<const ANSICHAR*>(Self->FileDataBuffer.GetData()), Self->FileDataBuffer.Num() - 1);
					FileContent = FString(Converter.Length(), Converter.Get());
				}

				UE_LOG(LogTemp, Log, TEXT("EOSKitTitleStorage: File read successful. Size: %d bytes"), Self->TotalBytesReceived);
				Self->OnSuccess.Broadcast(FileContent, Self->TotalBytesReceived);
			}
			else
			{
				FString ErrorMessage = FString::Printf(TEXT("Read file failed: %s"), 
					UTF8_TO_TCHAR(EOS_EResult_ToString(Data->ResultCode)));
				UE_LOG(LogTemp, Error, TEXT("EOSKitTitleStorage: %s"), *ErrorMessage);
				Self->OnFail.Broadcast(ErrorMessage);
			}

			Self->SetReadyToDestroy();
		});
	});
}
