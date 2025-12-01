// Copyright (C) 2024, All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Interfaces/OnlineTitleFileInterface.h"
#include "OnlineSubsystemEOSKit.h"

#if WITH_EOS_SDK
	#include "eos_titlestorage.h"
#endif

class FOnlineSubsystemEOSKit;

// ETitleFileFileType enum - may not be in UE 5.5 interface, define it here for backward compatibility
enum class ETitleFileFileType : uint8
{
	Binary = 0,
	Text = 1
};

#if WITH_EOS_SDK

/**
 * Interface for EOS title storage
 */
class ONLINESUBSYSTEMEOSKIT_API FOnlineTitleFileEOSKit :
	public IOnlineTitleFile
	, public TSharedFromThis<FOnlineTitleFileEOSKit, ESPMode::ThreadSafe>
{
public:
	FOnlineTitleFileEOSKit() = delete;
	explicit FOnlineTitleFileEOSKit(FOnlineSubsystemEOSKit* InSubsystem);
	virtual ~FOnlineTitleFileEOSKit();

	// IOnlineTitleFile interface
	virtual bool EnumerateFiles(const FPagedQuery& Page = FPagedQuery()) override;
	virtual void GetFileList(TArray<FCloudFileHeader>& OutFiles) override;
	virtual bool ReadFile(const FString& FileName) override;
	// Some methods were removed from IOnlineTitleFile in UE 5.5, but kept for backward compatibility
	bool GetFileList(const TArray<FString>& FileTags, const FOnEnumerateFilesCompleteDelegate& Delegate = FOnEnumerateFilesCompleteDelegate());
	bool ReadFile(const FString& FileName, const FOnReadFileCompleteDelegate& Delegate = FOnReadFileCompleteDelegate());
	bool ReadFile(const FString& FileName, const FOnReadFileCompleteDelegate& Delegate, const TArray<FString>& FileTags);
	virtual bool ClearFiles() override;
	virtual bool ClearFile(const FString& FileName) override;
	virtual bool GetFileContents(const FString& FileName, TArray<uint8>& FileContents);
	virtual bool ClearCachedFiles();
	virtual bool ClearCachedFile(const FString& FileName);
	virtual FString GetFileContents(const FString& FileName) const;
	virtual void DeleteCachedFiles(bool bSkipLastAccessed) override;
	virtual bool DeleteCachedFile(const FString& FileName);
	
	// GetFileType is not part of IOnlineTitleFile interface in UE 5.5, kept for backward compatibility
	ETitleFileFileType GetFileType(const FString& FileName) const;

private:
	FOnlineSubsystemEOSKit* EOSKitSubsystem;
	EOS_HTitleStorage TitleStorageHandle;
	
	mutable FCriticalSection TitleFileLock;
	TMap<FString, TArray<uint8>> CachedFiles;
};

#endif // WITH_EOS_SDK

