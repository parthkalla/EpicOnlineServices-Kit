// Copyright (C) 2024, All Rights Reserved.

#include "OnlineTitleFileEOSKit.h"
#include "OnlineSubsystemEOSKit.h"
#include "Engine/GameInstance.h"
#include "Engine/World.h"

#if WITH_EOS_SDK

FOnlineTitleFileEOSKit::FOnlineTitleFileEOSKit(FOnlineSubsystemEOSKit* InSubsystem)
	: EOSKitSubsystem(InSubsystem)
	, TitleStorageHandle(nullptr)
{
	if (EOSKitSubsystem)
	{
		TitleStorageHandle = EOSKitSubsystem->TitleStorageHandle;
	}
}

FOnlineTitleFileEOSKit::~FOnlineTitleFileEOSKit()
{
}

bool FOnlineTitleFileEOSKit::EnumerateFiles(const FPagedQuery& Page)
{
	// Enumerate files - not implemented yet
	return false;
}

void FOnlineTitleFileEOSKit::GetFileList(TArray<FCloudFileHeader>& OutFiles)
{
	// Get file list - not implemented yet
	OutFiles.Empty();
}

bool FOnlineTitleFileEOSKit::ReadFile(const FString& FileName)
{
	// Read file - not implemented yet
	return false;
}

bool FOnlineTitleFileEOSKit::GetFileList(const TArray<FString>& FileTags, const FOnEnumerateFilesCompleteDelegate& Delegate)
{
	// Delegate to EOS Title Storage
	UE_LOG_ONLINE(Log, TEXT("FOnlineTitleFileEOSKit::GetFileList: Not fully implemented yet"));
	// In UE 5.5, FOnEnumerateFilesCompleteDelegate takes (bool, const FString&) - (bWasSuccessful, ErrorMessage)
	Delegate.ExecuteIfBound(false, TEXT("Not implemented"));
	return false;
}

bool FOnlineTitleFileEOSKit::ReadFile(const FString& FileName, const FOnReadFileCompleteDelegate& Delegate)
{
	return ReadFile(FileName, Delegate, TArray<FString>());
}

bool FOnlineTitleFileEOSKit::ReadFile(const FString& FileName, const FOnReadFileCompleteDelegate& Delegate, const TArray<FString>& FileTags)
{
	// Delegate to EOS Title Storage
	UE_LOG_ONLINE(Log, TEXT("FOnlineTitleFileEOSKit::ReadFile: Not fully implemented yet"));
	// In UE 5.5, FOnReadFileCompleteDelegate takes (bool, const FString&) - (bWasSuccessful, ErrorMessage)
	Delegate.ExecuteIfBound(false, TEXT("Not implemented"));
	return false;
}

bool FOnlineTitleFileEOSKit::ClearFiles()
{
	FScopeLock ScopeLock(&TitleFileLock);
	CachedFiles.Empty();
	return true;
}

bool FOnlineTitleFileEOSKit::ClearFile(const FString& FileName)
{
	FScopeLock ScopeLock(&TitleFileLock);
	CachedFiles.Remove(FileName);
	return true;
}

bool FOnlineTitleFileEOSKit::GetFileContents(const FString& FileName, TArray<uint8>& FileContents)
{
	FScopeLock ScopeLock(&TitleFileLock);
	const TArray<uint8>* Contents = CachedFiles.Find(FileName);
	if (Contents)
	{
		FileContents = *Contents;
		return true;
	}
	return false;
}

bool FOnlineTitleFileEOSKit::ClearCachedFiles()
{
	ClearFiles();
	return true;
}

bool FOnlineTitleFileEOSKit::ClearCachedFile(const FString& FileName)
{
	ClearFile(FileName);
	return true;
}

ETitleFileFileType FOnlineTitleFileEOSKit::GetFileType(const FString& FileName) const
{
	return ETitleFileFileType::Binary;
}

FString FOnlineTitleFileEOSKit::GetFileContents(const FString& FileName) const
{
	FScopeLock ScopeLock(&TitleFileLock);
	const TArray<uint8>* Contents = CachedFiles.Find(FileName);
	if (Contents)
	{
		return FString(ANSI_TO_TCHAR(reinterpret_cast<const char*>(Contents->GetData())));
	}
	return FString();
}

void FOnlineTitleFileEOSKit::DeleteCachedFiles(bool bSkipLastAccessed)
{
	ClearFiles();
}

bool FOnlineTitleFileEOSKit::DeleteCachedFile(const FString& FileName)
{
	ClearFile(FileName);
	return true;
}

#endif // WITH_EOS_SDK

