// Copyright (C) 2024, All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Interfaces/OnlineSharedCloudInterface.h"
#include "OnlineSubsystemEOSKit.h"

#if WITH_EOS_SDK

class FOnlineSubsystemEOSKit;

/**
 * Interface for EOS shared cloud (stub - EOS doesn't have shared cloud)
 */
class ONLINESUBSYSTEMEOSKIT_API FOnlineSharedCloudEOSKit :
	public IOnlineSharedCloud
	, public TSharedFromThis<FOnlineSharedCloudEOSKit, ESPMode::ThreadSafe>
{
public:
	FOnlineSharedCloudEOSKit() = delete;
	explicit FOnlineSharedCloudEOSKit(FOnlineSubsystemEOSKit* InSubsystem);
	virtual ~FOnlineSharedCloudEOSKit();

	// IOnlineSharedCloud interface
	virtual bool GetSharedFileContents(const FSharedContentHandle& Handle, TArray<uint8>& OutFileContents) override;
	virtual bool ClearSharedFiles() override;
	virtual bool ClearSharedFile(const FSharedContentHandle& Handle) override;
	virtual bool ReadSharedFile(const FSharedContentHandle& Handle) override;
	virtual bool WriteSharedFile(const FUniqueNetId& UserId, const FString& FileName, TArray<uint8>& FileContents) override;
	virtual void GetDummySharedHandlesForTest(TArray<TSharedRef<FSharedContentHandle, ESPMode::ThreadSafe>>& OutHandles) override;
	// These methods were removed from IOnlineSharedCloud in UE 5.5, but kept for backward compatibility
	bool GetFileContents(const FString& FileName, TArray<uint8>& FileContents);
	bool ClearFiles();
	bool ClearFile(const FString& FileName);
	void EnumerateSharedFiles(const FUniqueNetId& UserId);
	void GetSharedFileList(const FUniqueNetId& UserId, TArray<FCloudFileHeader>& SharedFiles);
	bool WriteSharedFile(const FString& FileName, TArray<uint8>& FileContents, bool bCompressBeforeUpload = false);
	bool ReadSharedFile(const FUniqueNetId& UserId, const FString& FileName);
	bool DeleteSharedFile(const FString& FileName, bool bShouldCloudDelete, bool bShouldLocallyDelete);

private:
	FOnlineSubsystemEOSKit* EOSKitSubsystem;
};

#endif // WITH_EOS_SDK

