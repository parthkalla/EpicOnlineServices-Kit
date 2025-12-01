// Copyright (C) 2024, All Rights Reserved.
//
// This file is part of the EOSKit Plugin.
//
// EOSKit is free software: you can redistribute it and/or modify
// it under the terms of the MIT License as published by the Open
// Source Initiative.
//
// EOSKit is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// MIT License for more details.
//
// You should have received a copy of the MIT License along with
// EOSKit. If not, see <https://opensource.org/licenses/MIT>.

#include "EOSWritePlayerDataAsync.h"
#include "EOSKitSubsystem.h"
#if WITH_EOS_SDK
#include "Windows/AllowWindowsPlatformTypes.h"
#include "eos_sdk.h"
#include "eos_playerdatastorage.h"
#include "Windows/HideWindowsPlatformTypes.h"
#endif

UEOSWritePlayerDataAsync* UEOSWritePlayerDataAsync::WritePlayerData(UObject* WorldContextObject, const FString& FileName, const TArray<uint8>& Data)
{
	UEOSWritePlayerDataAsync* WritePlayerDataAsync = NewObject<UEOSWritePlayerDataAsync>();
	WritePlayerDataAsync->WorldContextObject = WorldContextObject;
	WritePlayerDataAsync->FileName = FileName;
	WritePlayerDataAsync->Data = Data;
	WritePlayerDataAsync->BytesWritten = 0;
	return WritePlayerDataAsync;
}

void UEOSWritePlayerDataAsync::Activate()
{
	if (UGameInstance* GameInstance = WorldContextObject->GetWorld()->GetGameInstance())
	{
		UEOSKitSubsystem* EOSKitSubsystem = GameInstance->GetSubsystem<UEOSKitSubsystem>();
		if (EOSKitSubsystem && EOSKitSubsystem->GetPlatformHandle())
		{
			EOS_HPlayerDataStorage PlayerDataStorageHandle = EOS_Platform_GetPlayerDataStorageInterface(EOSKitSubsystem->GetPlatformHandle());

			EOS_PlayerDataStorage_WriteFileOptions WriteOptions = {};
			WriteOptions.ApiVersion = EOS_PLAYERDATASTORAGE_WRITEFILE_API_LATEST;
			WriteOptions.LocalUserId = EOSKitSubsystem->GetProductUserId();
			WriteOptions.Filename = TCHAR_TO_UTF8(*FileName);
			WriteOptions.ChunkLengthBytes = 4096;
			WriteOptions.WriteFileDataCallback = OnWriteFileDataCallback;
			WriteOptions.FileTransferProgressCallback = OnFileTransferProgressCallback;

			EOS_PlayerDataStorage_WriteFile(PlayerDataStorageHandle, &WriteOptions, this, OnWriteFileCompleteCallback);
			return;
		}
	}
	OnFailure.Broadcast("EOSKitSubsystem not available.");
}

void UEOSWritePlayerDataAsync::OnWriteFileCompleteCallback(const EOS_PlayerDataStorage_WriteFileCallbackInfo* Data)
{
	UEOSWritePlayerDataAsync* This = static_cast<UEOSWritePlayerDataAsync*>(Data->ClientData);
	if (Data->ResultCode == EOS_EResult::EOS_Success)
	{
		This->OnSuccess.Broadcast("");
	}
	else
	{
		This->OnFailure.Broadcast(EOS_EResult_ToString(Data->ResultCode));
	}
	This->SetReadyToDestroy();
}

EOS_PlayerDataStorage_EWriteResult UEOSWritePlayerDataAsync::OnWriteFileDataCallback(const EOS_PlayerDataStorage_WriteFileDataCallbackInfo* Data, void* OutDataBuffer, uint32_t* OutDataWritten)
{
	UEOSWritePlayerDataAsync* This = static_cast<UEOSWritePlayerDataAsync*>(Data->ClientData);
	
	uint32_t RemainingBytes = This->Data.Num() - This->BytesWritten;
	if (RemainingBytes == 0)
	{
		*OutDataWritten = 0;
		return EOS_PlayerDataStorage_EWriteResult::EOS_WR_CompleteRequest;
	}

	uint32_t BytesToWrite = FMath::Min(RemainingBytes, Data->DataBufferLengthBytes);
	FMemory::Memcpy(OutDataBuffer, This->Data.GetData() + This->BytesWritten, BytesToWrite);
	This->BytesWritten += BytesToWrite;
	*OutDataWritten = BytesToWrite;

	if (This->BytesWritten >= (uint32_t)This->Data.Num())
	{
		return EOS_PlayerDataStorage_EWriteResult::EOS_WR_CompleteRequest;
	}

	return EOS_PlayerDataStorage_EWriteResult::EOS_WR_ContinueWriting;
}

void UEOSWritePlayerDataAsync::OnFileTransferProgressCallback(const EOS_PlayerDataStorage_FileTransferProgressCallbackInfo* Data)
{
	// Optional: Handle progress updates
}
