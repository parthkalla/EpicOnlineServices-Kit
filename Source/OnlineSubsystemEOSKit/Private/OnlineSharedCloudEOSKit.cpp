// Copyright (C) 2024, All Rights Reserved.

#include "OnlineSharedCloudEOSKit.h"
#include "OnlineSubsystemEOSKit.h"

#if WITH_EOS_SDK

FOnlineSharedCloudEOSKit::FOnlineSharedCloudEOSKit(FOnlineSubsystemEOSKit* InSubsystem)
	: EOSKitSubsystem(InSubsystem)
{
}

FOnlineSharedCloudEOSKit::~FOnlineSharedCloudEOSKit()
{
}

bool FOnlineSharedCloudEOSKit::GetFileContents(const FString& FileName, TArray<uint8>& FileContents)
{
	// EOS doesn't have shared cloud - this is a stub
	return false;
}

bool FOnlineSharedCloudEOSKit::ClearFiles()
{
	return false;
}

bool FOnlineSharedCloudEOSKit::ClearFile(const FString& FileName)
{
	return false;
}

void FOnlineSharedCloudEOSKit::EnumerateSharedFiles(const FUniqueNetId& UserId)
{
	// EOS doesn't have shared cloud
}

void FOnlineSharedCloudEOSKit::GetSharedFileList(const FUniqueNetId& UserId, TArray<FCloudFileHeader>& SharedFiles)
{
	SharedFiles.Empty();
}

bool FOnlineSharedCloudEOSKit::WriteSharedFile(const FString& FileName, TArray<uint8>& FileContents, bool bCompressBeforeUpload)
{
	return false;
}

bool FOnlineSharedCloudEOSKit::ReadSharedFile(const FUniqueNetId& UserId, const FString& FileName)
{
	return false;
}

bool FOnlineSharedCloudEOSKit::DeleteSharedFile(const FString& FileName, bool bShouldCloudDelete, bool bShouldLocallyDelete)
{
	return false;
}

bool FOnlineSharedCloudEOSKit::GetSharedFileContents(const FSharedContentHandle& Handle, TArray<uint8>& OutFileContents)
{
	// Get shared file contents - not implemented yet (EOS doesn't have shared cloud)
	return false;
}

bool FOnlineSharedCloudEOSKit::ClearSharedFiles()
{
	// Clear shared files - not implemented yet
	return false;
}

bool FOnlineSharedCloudEOSKit::ClearSharedFile(const FSharedContentHandle& Handle)
{
	// Clear shared file - not implemented yet
	return false;
}

bool FOnlineSharedCloudEOSKit::ReadSharedFile(const FSharedContentHandle& Handle)
{
	// Read shared file - not implemented yet
	return false;
}

bool FOnlineSharedCloudEOSKit::WriteSharedFile(const FUniqueNetId& UserId, const FString& FileName, TArray<uint8>& FileContents)
{
	// Write shared file - not implemented yet
	return false;
}

void FOnlineSharedCloudEOSKit::GetDummySharedHandlesForTest(TArray<TSharedRef<FSharedContentHandle, ESPMode::ThreadSafe>>& OutHandles)
{
	// Get dummy shared handles for test - not implemented yet
	OutHandles.Empty();
}

#endif // WITH_EOS_SDK

