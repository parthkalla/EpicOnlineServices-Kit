// Copyright (c) 2023 Betide Studio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintAsyncActionBase.h"
#include "OnlineSubsystemEOK/SdkFunctions/EOK_SharedFunctionFile.h"
#include "OnlineSubsystemEOS.h"
#include "Async/Async.h"
#include "OnlineSubsystemEOK/SdkFunctions/ConnectInterface/EOK_ConnectSubsystem.h"
#include "EOK_PlayerDataStorage_WriteFile.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FEOK_PlayerDataStorage_WriteFileDelegate, const TEnumAsByte<EEOK_Result>&, Result, const FEOK_ProductUserId&, LocalUserId);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_FourParams(FEOK_PlayerDataStorage_OnFileWriteTransferProgressCallback, const FEOK_ProductUserId&, LocalUserId, const FString&, Filename, int32, BytesTransferred, int32, TotalFileSizeBytes);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FEOK_PlayerDataStorage_OnWriteFileDataCallback);
UCLASS()
class OnlineSubsystemEOK_API UEOK_PlayerDataStorage_WriteFile : public UBlueprintAsyncActionBase
{
	GENERATED_BODY()

public:

	//Write new data to a specific file, potentially overwriting any existing file by the same name, to the cloud. This request will occur asynchronously, potentially over multiple frames. All callbacks for this function will come from the same thread that the SDK is ticked from. If specified, the FileTransferProgressCallback will always be called at least once if the request is started successfully.
	UFUNCTION(BlueprintCallable, Category = "Epic Online Services-Kit V2 | SDK Functions | Player Data Storage Interface", DisplayName="EOS_PlayerDataStorage_WriteFile")
	static UEOK_PlayerDataStorage_WriteFile* EOK_PlayerDataStorage_WriteFile(FEOK_ProductUserId LocalUserId, FString Filename, const TArray<uint8>& Data, int32 DataLengthBytes, int32 ChunkLengthBytes);

	UPROPERTY(BlueprintAssignable)
	FEOK_PlayerDataStorage_WriteFileDelegate OnCallback;

	UPROPERTY(BlueprintAssignable)
	FEOK_PlayerDataStorage_OnFileWriteTransferProgressCallback OnFileTransferProgressCallback;

	UPROPERTY(BlueprintAssignable)
	FEOK_PlayerDataStorage_OnWriteFileDataCallback OnWriteFileDataCallback;
	
private:
	static void EOS_CALL EOS_PlayerDataStorage_OnFileTransferProgress(const EOS_PlayerDataStorage_FileTransferProgressCallbackInfo* Data);
	static EOS_PlayerDataStorage_EWriteResult EOS_PlayerDataStorage_OnWriteFileData(const EOS_PlayerDataStorage_WriteFileDataCallbackInfo* Data, void* OutDataBuffer, uint32_t* OutDataWritten);
	virtual void Activate() override;
	static void EOS_CALL EOS_PlayerDataStorage_OnWriteFileComplete(const EOS_PlayerDataStorage_WriteFileCallbackInfo* Data);
	FEOK_ProductUserId Var_LocalUserId;
	FString Var_Filename;
	TArray<uint8> Var_Data;
	int32 Var_DataLengthBytes;
	int32 BytesWritten = 0;
	int32 Var_ChunkLengthBytes = 1024;
};
