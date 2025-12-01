// Copyright (C) 2024, All Rights Reserved.

#include "OnlineUserCloudEOSKit.h"
#include "OnlineSubsystemEOSKit.h"
#include "EOSKitPlayerDataStorageSubsystem.h"
#include "Engine/GameInstance.h"
#include "Engine/World.h"

#if WITH_EOS_SDK

FOnlineUserCloudEOSKit::FOnlineUserCloudEOSKit(FOnlineSubsystemEOSKit* InSubsystem)
	: EOSKitSubsystem(InSubsystem)
	, PlayerDataStorageHandle(nullptr)
{
	if (EOSKitSubsystem)
	{
		PlayerDataStorageHandle = EOSKitSubsystem->PlayerDataStorageHandle;
	}
}

FOnlineUserCloudEOSKit::~FOnlineUserCloudEOSKit()
{
}

bool FOnlineUserCloudEOSKit::GetFileContents(const FUniqueNetId& UserId, const FString& FileName, TArray<uint8>& FileContents)
{
	FScopeLock ScopeLock(&UserCloudLock);
	const TArray<uint8>* Contents = CachedFiles.Find(FileName);
	if (Contents)
	{
		FileContents = *Contents;
		return true;
	}
	return false;
}

bool FOnlineUserCloudEOSKit::ClearFiles(const FUniqueNetId& UserId)
{
	FScopeLock ScopeLock(&UserCloudLock);
	CachedFiles.Empty();
	return true;
}

bool FOnlineUserCloudEOSKit::ClearFile(const FUniqueNetId& UserId, const FString& FileName)
{
	FScopeLock ScopeLock(&UserCloudLock);
	return CachedFiles.Remove(FileName) > 0;
}

void FOnlineUserCloudEOSKit::EnumerateUserFiles(const FUniqueNetId& UserId)
{
	// Delegate to EOSKitPlayerDataStorageSubsystem
	if (GEngine)
	{
		for (const FWorldContext& Context : GEngine->GetWorldContexts())
		{
			if (Context.World() && Context.World()->GetGameInstance())
			{
				UEOSKitPlayerDataStorageSubsystem* PlayerDataStorageSubsystem = Context.World()->GetGameInstance()->GetSubsystem<UEOSKitPlayerDataStorageSubsystem>();
				if (PlayerDataStorageSubsystem)
				{
					UE_LOG_ONLINE(Log, TEXT("FOnlineUserCloudEOSKit::EnumerateUserFiles: Delegating to UEOSKitPlayerDataStorageSubsystem"));
					// PlayerDataStorageSubsystem->QueryFileList(...);
				}
			}
		}
	}
}

void FOnlineUserCloudEOSKit::GetUserFileList(const FUniqueNetId& UserId, TArray<FCloudFileHeader>& UserFiles)
{
	// Get file list from cache or query
	UserFiles.Empty();
}

bool FOnlineUserCloudEOSKit::WriteUserFile(const FUniqueNetId& UserId, const FString& FileName, TArray<uint8>& FileContents, bool bCompressBeforeUpload)
{
	// Delegate to EOSKitPlayerDataStorageSubsystem
	if (GEngine)
	{
		for (const FWorldContext& Context : GEngine->GetWorldContexts())
		{
			if (Context.World() && Context.World()->GetGameInstance())
			{
				UEOSKitPlayerDataStorageSubsystem* PlayerDataStorageSubsystem = Context.World()->GetGameInstance()->GetSubsystem<UEOSKitPlayerDataStorageSubsystem>();
				if (PlayerDataStorageSubsystem)
				{
					UE_LOG_ONLINE(Log, TEXT("FOnlineUserCloudEOSKit::WriteUserFile: Delegating to UEOSKitPlayerDataStorageSubsystem"));
					// PlayerDataStorageSubsystem->WriteFile(...);
					return true;
				}
			}
		}
	}
	return false;
}

bool FOnlineUserCloudEOSKit::ReadUserFile(const FUniqueNetId& UserId, const FString& FileName)
{
	// Delegate to EOSKitPlayerDataStorageSubsystem
	if (GEngine)
	{
		for (const FWorldContext& Context : GEngine->GetWorldContexts())
		{
			if (Context.World() && Context.World()->GetGameInstance())
			{
				UEOSKitPlayerDataStorageSubsystem* PlayerDataStorageSubsystem = Context.World()->GetGameInstance()->GetSubsystem<UEOSKitPlayerDataStorageSubsystem>();
				if (PlayerDataStorageSubsystem)
				{
					UE_LOG_ONLINE(Log, TEXT("FOnlineUserCloudEOSKit::ReadUserFile: Delegating to UEOSKitPlayerDataStorageSubsystem"));
					// PlayerDataStorageSubsystem->ReadFile(...);
					return true;
				}
			}
		}
	}
	return false;
}

bool FOnlineUserCloudEOSKit::DeleteUserFile(const FUniqueNetId& UserId, const FString& FileName, bool bShouldCloudDelete, bool bShouldLocallyDelete)
{
	// Delegate to EOSKitPlayerDataStorageSubsystem
	if (bShouldLocallyDelete)
	{
		ClearFile(UserId, FileName);
	}
	if (bShouldCloudDelete)
	{
		// PlayerDataStorageSubsystem->DeleteFile(...);
	}
	return true;
}

void FOnlineUserCloudEOSKit::DumpCloudState(const FUniqueNetId& UserId)
{
	FScopeLock ScopeLock(&UserCloudLock);
	UE_LOG_ONLINE(Log, TEXT("FOnlineUserCloudEOSKit: Dumping cloud state - %d cached files"), CachedFiles.Num());
}

void FOnlineUserCloudEOSKit::DumpCloudFileState(const FUniqueNetId& UserId, const FString& FileName)
{
	FScopeLock ScopeLock(&UserCloudLock);
	const TArray<uint8>* Contents = CachedFiles.Find(FileName);
	if (Contents)
	{
		UE_LOG_ONLINE(Log, TEXT("FOnlineUserCloudEOSKit: File %s - %d bytes"), *FileName, Contents->Num());
	}
	else
	{
		UE_LOG_ONLINE(Log, TEXT("FOnlineUserCloudEOSKit: File %s - not cached"), *FileName);
	}
}

void FOnlineUserCloudEOSKit::CancelWriteUserFile(const FUniqueNetId& UserId, const FString& FileName)
{
	// Cancel write user file - not implemented yet
	UE_LOG_ONLINE(Log, TEXT("FOnlineUserCloudEOSKit::CancelWriteUserFile: Not implemented"));
}

bool FOnlineUserCloudEOSKit::RequestUsageInfo(const FUniqueNetId& UserId)
{
	// Request usage info - not implemented yet
	return false;
}

#endif // WITH_EOS_SDK

