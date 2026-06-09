// Copyright (c) 2023 Betide Studio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintAsyncActionBase.h"
#include "OnlineSubsystemEOK/SdkFunctions/EOK_SharedFunctionFile.h"
#include "EOK_PlayerDataStorage_DeleteCache.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FEOK_PlayerDataStorage_DeleteCacheDelegate, const TEnumAsByte<EEOK_Result>&, Result, const FEOK_ProductUserId&, LocalUserId);
UCLASS()
class OnlineSubsystemEOK_API UEOK_PlayerDataStorage_DeleteCache : public UBlueprintAsyncActionBase
{
	GENERATED_BODY()

public:

	//Clear previously cached file data. This operation will be done asynchronously. All cached files except those corresponding to the transfers in progress will be removed. Warning: Use this with care. Cache system generally tries to clear old and unused cached files from time to time. Unnecessarily clearing cache can degrade performance as SDK will have to re-download data.
	UFUNCTION(BlueprintCallable, Category = "Epic Online Services-Kit V2 | SDK Functions | Player Data Storage Interface", DisplayName="EOS_PlayerDataStorage_DeleteCache")
	static UEOK_PlayerDataStorage_DeleteCache* EOK_PlayerDataStorage_DeleteCache(FEOK_ProductUserId LocalUserId);

	UPROPERTY(BlueprintAssignable)
	FEOK_PlayerDataStorage_DeleteCacheDelegate OnCallback;
private:
	virtual void Activate() override;
	FEOK_ProductUserId Var_LocalUserId;
	static void EOS_CALL EOS_PlayerDataStorage_OnDeleteCacheComplete(const EOS_PlayerDataStorage_DeleteCacheCallbackInfo* Data);
};
