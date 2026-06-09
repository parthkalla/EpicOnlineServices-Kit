// Copyright (c) 2023 Betide Studio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintAsyncActionBase.h"
#include "OnlineSubsystemEOK/SdkFunctions/EOK_SharedFunctionFile.h"
#include "OnlineSubsystemEOS.h"
#include "Async/Async.h"
#include "OnlineSubsystemEOK/SdkFunctions/ConnectInterface/EOK_ConnectSubsystem.h"
#include "EOK_PlayerDataStorage_QueryFileList.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FEOK_PlayerDataStorage_QueryFileListDelegate, const TEnumAsByte<EEOK_Result>&, Result, const FEOK_ProductUserId&, LocalUserId, int32, FileCount);

UCLASS()
class OnlineSubsystemEOK_API UEOK_PlayerDataStorage_QueryFileList : public UBlueprintAsyncActionBase
{
	GENERATED_BODY()

public:

	//Query the file metadata, such as file names, size, and a MD5 hash of the data, for all files owned by this user for this application. This is not required before a file may be opened, saved, copied, or deleted.
	UFUNCTION(BlueprintCallable, Category = "Epic Online Services-Kit V2 | SDK Functions | Player Data Storage Interface", DisplayName="EOS_PlayerDataStorage_QueryFileList")
	static UEOK_PlayerDataStorage_QueryFileList* EOK_PlayerDataStorage_QueryFileList(FEOK_ProductUserId LocalUserId);

	UPROPERTY(BlueprintAssignable)
	FEOK_PlayerDataStorage_QueryFileListDelegate OnCallback;

private:
	virtual void Activate() override;
	static void EOS_CALL EOS_PlayerDataStorage_OnQueryFileListComplete(const EOS_PlayerDataStorage_QueryFileListCallbackInfo* Data);
	FEOK_ProductUserId Var_LocalUserId;
};
