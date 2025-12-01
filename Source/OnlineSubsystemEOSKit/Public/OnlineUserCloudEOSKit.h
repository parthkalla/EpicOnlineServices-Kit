// Copyright (C) 2024, All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Interfaces/OnlineUserCloudInterface.h"
#include "OnlineSubsystemEOSKit.h"

#if WITH_EOS_SDK
	#include "eos_playerdatastorage.h"
#endif

class FOnlineSubsystemEOSKit;

#if WITH_EOS_SDK

/**
 * Interface for EOS player data storage (user cloud)
 */
class ONLINESUBSYSTEMEOSKIT_API FOnlineUserCloudEOSKit :
	public IOnlineUserCloud
	, public TSharedFromThis<FOnlineUserCloudEOSKit, ESPMode::ThreadSafe>
{
public:
	FOnlineUserCloudEOSKit() = delete;
	explicit FOnlineUserCloudEOSKit(FOnlineSubsystemEOSKit* InSubsystem);
	virtual ~FOnlineUserCloudEOSKit();

	// IOnlineUserCloud interface
	virtual bool GetFileContents(const FUniqueNetId& UserId, const FString& FileName, TArray<uint8>& FileContents) override;
	virtual bool ClearFiles(const FUniqueNetId& UserId) override;
	virtual bool ClearFile(const FUniqueNetId& UserId, const FString& FileName) override;
	virtual void EnumerateUserFiles(const FUniqueNetId& UserId) override;
	virtual void GetUserFileList(const FUniqueNetId& UserId, TArray<FCloudFileHeader>& UserFiles) override;
	virtual bool WriteUserFile(const FUniqueNetId& UserId, const FString& FileName, TArray<uint8>& FileContents, bool bCompressBeforeUpload = false) override;
	virtual bool ReadUserFile(const FUniqueNetId& UserId, const FString& FileName) override;
	virtual bool DeleteUserFile(const FUniqueNetId& UserId, const FString& FileName, bool bShouldCloudDelete, bool bShouldLocallyDelete) override;
	virtual void DumpCloudState(const FUniqueNetId& UserId) override;
	virtual void DumpCloudFileState(const FUniqueNetId& UserId, const FString& FileName) override;
	virtual void CancelWriteUserFile(const FUniqueNetId& UserId, const FString& FileName) override;
	virtual bool RequestUsageInfo(const FUniqueNetId& UserId) override;

private:
	FOnlineSubsystemEOSKit* EOSKitSubsystem;
	EOS_HPlayerDataStorage PlayerDataStorageHandle;
	
	mutable FCriticalSection UserCloudLock;
	TMap<FString, TArray<uint8>> CachedFiles;
};

#endif // WITH_EOS_SDK

