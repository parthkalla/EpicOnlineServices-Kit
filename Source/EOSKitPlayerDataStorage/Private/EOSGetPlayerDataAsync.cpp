// Copyright (C) 2024, All Rights Reserved.

#include "EOSGetPlayerDataAsync.h"
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

UEOSGetPlayerDataAsync* UEOSGetPlayerDataAsync::GetPlayerData(UObject* WorldContextObject, const FString& Filename)
{
	UEOSGetPlayerDataAsync* Node = NewObject<UEOSGetPlayerDataAsync>();
	Node->WorldContextObject = WorldContextObject;
	Node->FilenameString = Filename;
	Node->ReadData.Empty();
	return Node;
}

void UEOSGetPlayerDataAsync::Activate()
{
	if (!WorldContextObject)
	{
		OnFailure.Broadcast(false, TArray<uint8>());
		SetReadyToDestroy();
		return;
	}

	if (FilenameString.IsEmpty())
	{
		OnFailure.Broadcast(false, TArray<uint8>());
		SetReadyToDestroy();
		return;
	}

	UGameInstance* GameInstance = UGameplayStatics::GetGameInstance(WorldContextObject);
	if (!GameInstance)
	{
		OnFailure.Broadcast(false, TArray<uint8>());
		SetReadyToDestroy();
		return;
	}

	UEOSKitSubsystem* EOSSubsystem = GameInstance->GetSubsystem<UEOSKitSubsystem>();
	if (!EOSSubsystem || !EOSSubsystem->GetPlatformHandle())
	{
		OnFailure.Broadcast(false, TArray<uint8>());
		SetReadyToDestroy();
		return;
	}

	// Get Product User ID from subsystem
	FString ProductUserIdString = UEOSKitLoginHelpers::GetProductUserIdString(WorldContextObject);
	if (ProductUserIdString.IsEmpty())
	{
		OnFailure.Broadcast(false, TArray<uint8>());
		SetReadyToDestroy();
		return;
	}

	EOS_HPlayerDataStorage PlayerDataStorageHandle = EOS_Platform_GetPlayerDataStorageInterface(EOSSubsystem->GetPlatformHandle());
	if (!PlayerDataStorageHandle)
	{
		OnFailure.Broadcast(false, TArray<uint8>());
		SetReadyToDestroy();
		return;
	}

	// Convert Product User ID
	EOS_ProductUserId LocalUserId = EOS_ProductUserId_FromString(TCHAR_TO_UTF8(*ProductUserIdString));
	if (!EOS_ProductUserId_IsValid(LocalUserId))
	{
		OnFailure.Broadcast(false, TArray<uint8>());
		SetReadyToDestroy();
		return;
	}

	// Setup options
	EOS_PlayerDataStorage_ReadFileOptions Options = {};
	Options.ApiVersion = EOS_PLAYERDATASTORAGE_READFILE_API_LATEST;
	Options.LocalUserId = LocalUserId;
	Options.Filename = TCHAR_TO_UTF8(*FilenameString);
	Options.ReadChunkLengthBytes = 64 * 1024; // 64KB chunks
	Options.ReadFileDataCallback = &UEOSGetPlayerDataAsync::OnReadFileDataCallback;
	Options.FileTransferProgressCallback = nullptr; // Optional

	// Clear read data
	ReadData.Empty();

	// Read file
	EOS_PlayerDataStorage_ReadFile(PlayerDataStorageHandle, &Options, this, &UEOSGetPlayerDataAsync::OnReadFileComplete);
}

EOS_PlayerDataStorage_EReadResult EOS_CALL UEOSGetPlayerDataAsync::OnReadFileDataCallback(
	const EOS_PlayerDataStorage_ReadFileDataCallbackInfo* Data)
{
	if (!Data || !Data->ClientData)
	{
		return EOS_PlayerDataStorage_EReadResult::EOS_RR_FailRequest;
	}

	UEOSGetPlayerDataAsync* Self = static_cast<UEOSGetPlayerDataAsync*>(Data->ClientData);

	// Append data to our buffer
	if (Data->DataChunk && Data->DataChunkLengthBytes > 0)
	{
		int32 CurrentSize = Self->ReadData.Num();
		Self->ReadData.SetNum(CurrentSize + Data->DataChunkLengthBytes, EAllowShrinking::No);
		FMemory::Memcpy(Self->ReadData.GetData() + CurrentSize, Data->DataChunk, Data->DataChunkLengthBytes);
	}

	// Continue reading
	return EOS_PlayerDataStorage_EReadResult::EOS_RR_ContinueReading;
}

void EOS_CALL UEOSGetPlayerDataAsync::OnReadFileComplete(const EOS_PlayerDataStorage_ReadFileCallbackInfo* Data)
{
	if (!Data || !Data->ClientData)
	{
		return;
	}

	UEOSGetPlayerDataAsync* Self = static_cast<UEOSGetPlayerDataAsync*>(Data->ClientData);

	if (Data->ResultCode == EOS_EResult::EOS_Success)
	{
		if (Self->ReadData.Num() > 0)
		{
			Self->OnSuccess.Broadcast(true, Self->ReadData);
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("EOSGetPlayerDataAsync: Read succeeded but no data was read"));
			Self->OnFailure.Broadcast(false, TArray<uint8>());
		}
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("EOSGetPlayerDataAsync: Read failed with result: %d"), (int32)Data->ResultCode);
		Self->OnFailure.Broadcast(false, TArray<uint8>());
	}

	Self->SetReadyToDestroy();
}

