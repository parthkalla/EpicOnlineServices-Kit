// Copyright (c) 2023 Betide Studio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintAsyncActionBase.h"
#include "OnlineSubsystemEOK/SdkFunctions/EOK_SharedFunctionFile.h"
#include "OnlineSubsystemEOS.h"
#include "Async/Async.h"
#include "OnlineSubsystemEOK/SdkFunctions/ConnectInterface/EOK_ConnectSubsystem.h"
#include "EOK_PlayerDataStorage_DuplicateFile.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FEOK_PlayerDataStorage_DuplicateFileDelegate, const TEnumAsByte<EEOK_Result>&, Result, const FEOK_ProductUserId&, LocalUserId);

UCLASS()
class OnlineSubsystemEOK_API UEOK_PlayerDataStorage_DuplicateFile : public UBlueprintAsyncActionBase
{
	GENERATED_BODY()

public:

	//Copies the data of an existing file to a new filename. This action happens entirely on the server and will not upload the contents of the source destination file from the host. This function paired with a subsequent EOS_PlayerDataStorage_DeleteFile can be used to rename a file. If successful, the destination file's metadata will be updated in our local cache.
	UFUNCTION(BlueprintCallable, Category = "Epic Online Services-Kit V2 | SDK Functions | Player Data Storage Interface", DisplayName="EOS_PlayerDataStorage_DuplicateFile")
	static UEOK_PlayerDataStorage_DuplicateFile* EOK_PlayerDataStorage_DuplicateFile(FEOK_ProductUserId LocalUserId, FString SourceFilename, FString DestinationFilename);

	UPROPERTY(BlueprintAssignable)
	FEOK_PlayerDataStorage_DuplicateFileDelegate OnCallback;
	
private:
	virtual void Activate() override;
	static void EOS_CALL EOS_PlayerDataStorage_OnDuplicateFileComplete(const EOS_PlayerDataStorage_DuplicateFileCallbackInfo* Data);
	FEOK_ProductUserId Var_LocalUserId;
	FString Var_SourceFilename;
	FString Var_DestinationFilename;
};
