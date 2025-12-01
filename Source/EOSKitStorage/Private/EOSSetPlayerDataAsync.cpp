// Copyright (C) 2024, All Rights Reserved.

#include "EOSSetPlayerDataAsync.h"
#include "EOSKitSubsystem.h"
#include "EOSKitLoginHelpers.h"
#include "Kismet/GameplayStatics.h"
#if WITH_EOS_SDK
#include "Windows/AllowWindowsPlatformTypes.h"
#include "eos_sdk.h"
#include "eos_playerdatastorage.h"
#include "eos_playerdatastorage_types.h"
#include "Windows/HideWindowsPlatformTypes.h"
#endif

UEOSSetPlayerDataAsync* UEOSSetPlayerDataAsync::SetPlayerData(UObject* WorldContextObject, const FString& Filename, const TArray<uint8>& DataToSave)
{
	UEOSSetPlayerDataAsync* Node = NewObject<UEOSSetPlayerDataAsync>();
	Node->WorldContextObject = WorldContextObject;
	Node->FilenameString = Filename;
	Node->DataToWrite = DataToSave;
	Node->CurrentWriteOffset = 0;
	return Node;
}

void UEOSSetPlayerDataAsync::Activate()
{
	if (!WorldContextObject)
	{
		OnFailure.Broadcast();
		SetReadyToDestroy();
		return;
	}

	if (FilenameString.IsEmpty())
	{
		OnFailure.Broadcast();
		SetReadyToDestroy();
		return;
	}

	if (DataToWrite.Num() == 0)
	{
		OnFailure.Broadcast();
		SetReadyToDestroy();
		return;
	}

	UGameInstance* GameInstance = UGameplayStatics::GetGameInstance(WorldContextObject);
	if (!GameInstance)
	{
		OnFailure.Broadcast();
		SetReadyToDestroy();
		return;
	}

	UEOSKitSubsystem* EOSSubsystem = GameInstance->GetSubsystem<UEOSKitSubsystem>();
	if (!EOSSubsystem || !EOSSubsystem->GetPlatformHandle())
	{
		OnFailure.Broadcast();
		SetReadyToDestroy();
		return;
	}

	// Get Product User ID from subsystem
	FString ProductUserIdString = UEOSKitLoginHelpers::GetProductUserIdString(WorldContextObject);
	if (ProductUserIdString.IsEmpty())
	{
		OnFailure.Broadcast();
		SetReadyToDestroy();
		return;
	}

	EOS_HPlayerDataStorage PlayerDataStorageHandle = EOS_Platform_GetPlayerDataStorageInterface(EOSSubsystem->GetPlatformHandle());
	if (!PlayerDataStorageHandle)
	{
		OnFailure.Broadcast();
		SetReadyToDestroy();
		return;
	}

	// Convert Product User ID
	EOS_ProductUserId LocalUserId = EOS_ProductUserId_FromString(TCHAR_TO_UTF8(*ProductUserIdString));
	if (!EOS_ProductUserId_IsValid(LocalUserId))
	{
		OnFailure.Broadcast();
		SetReadyToDestroy();
		return;
	}

	// Setup options
	EOS_PlayerDataStorage_WriteFileOptions Options = {};
	Options.ApiVersion = EOS_PLAYERDATASTORAGE_WRITEFILE_API_LATEST;
	Options.LocalUserId = LocalUserId;
	Options.Filename = TCHAR_TO_UTF8(*FilenameString);
	Options.ChunkLengthBytes = 64 * 1024; // 64KB chunks
	Options.WriteFileDataCallback = &UEOSSetPlayerDataAsync::OnWriteFileDataCallback;
	Options.FileTransferProgressCallback = nullptr; // Optional

	// Reset write offset
	CurrentWriteOffset = 0;

	// Write file
	EOS_PlayerDataStorage_WriteFile(PlayerDataStorageHandle, &Options, this, &UEOSSetPlayerDataAsync::OnWriteFileComplete);
}

EOS_PlayerDataStorage_EWriteResult EOS_CALL UEOSSetPlayerDataAsync::OnWriteFileDataCallback(
	const EOS_PlayerDataStorage_WriteFileDataCallbackInfo* Data,
	void* OutDataBuffer,
	uint32_t* OutDataWritten)
{
	if (!Data || !Data->ClientData || !OutDataBuffer || !OutDataWritten)
	{
		return EOS_PlayerDataStorage_EWriteResult::EOS_WR_FailRequest;
	}

	UEOSSetPlayerDataAsync* Self = static_cast<UEOSSetPlayerDataAsync*>(Data->ClientData);

	// Calculate how much data we can write
	uint32_t RemainingBytes = Self->DataToWrite.Num() - Self->CurrentWriteOffset;
	uint32_t BytesToWrite = FMath::Min(RemainingBytes, Data->DataBufferLengthBytes);

	if (BytesToWrite > 0)
	{
		// Copy data to output buffer
		FMemory::Memcpy(OutDataBuffer, Self->DataToWrite.GetData() + Self->CurrentWriteOffset, BytesToWrite);
		*OutDataWritten = BytesToWrite;
		Self->CurrentWriteOffset += BytesToWrite;
	}
	else
	{
		*OutDataWritten = 0;
	}

	// Check if we've written all data
	if (Self->CurrentWriteOffset >= Self->DataToWrite.Num())
	{
		return EOS_PlayerDataStorage_EWriteResult::EOS_WR_CompleteRequest;
	}

	return EOS_PlayerDataStorage_EWriteResult::EOS_WR_ContinueWriting;
}

void EOS_CALL UEOSSetPlayerDataAsync::OnWriteFileComplete(const EOS_PlayerDataStorage_WriteFileCallbackInfo* Data)
{
	if (!Data || !Data->ClientData)
	{
		return;
	}

	UEOSSetPlayerDataAsync* Self = static_cast<UEOSSetPlayerDataAsync*>(Data->ClientData);

	if (Data->ResultCode == EOS_EResult::EOS_Success)
	{
		Self->OnSuccess.Broadcast();
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("EOSSetPlayerDataAsync: Write failed with result: %d"), (int32)Data->ResultCode);
		Self->OnFailure.Broadcast();
	}

	Self->SetReadyToDestroy();
}

