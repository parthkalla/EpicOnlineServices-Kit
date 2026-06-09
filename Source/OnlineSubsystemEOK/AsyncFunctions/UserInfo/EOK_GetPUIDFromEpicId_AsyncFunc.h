// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintAsyncActionBase.h"
#include "eos_connect.h"
#include "eos_connect_types.h"
#include "EOK_GetPUIDFromEpicId_AsyncFunc.generated.h"

USTRUCT(BlueprintType)
struct FProductUserIdAndEpicId
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, Category = "Epic Online Services-Kit V2 || UserInfo")
	FString EpicAccountId = "";

	UPROPERTY(BlueprintReadWrite, Category = "Epic Online Services-Kit V2 || UserInfo")
	FString ProductUserId = "";

};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FGetPUIDFromEpicIdDelegate, const TArray<FProductUserIdAndEpicId>&, UserInfo);


UCLASS()
class OnlineSubsystemEOK_API UEOK_GetPUIDFromEpicId_AsyncFunc : public UBlueprintAsyncActionBase
{
	GENERATED_BODY()

public:

	UFUNCTION(BlueprintCallable, DisplayName = "Get Product User Id From Epic Id", meta = (BlueprintInternalUseOnly = "true"), Category = "Epic Online Services-Kit V2 || UserInfo")
	static UEOK_GetPUIDFromEpicId_AsyncFunc* EOKGetPUIDFromEpicId(TArray<FString> TargetEpicAccountIds, FString LocalProductUserId);

	TArray<FString> Var_TargetUserIds;

	FString Var_LocalProductUserId;

	void GetPUIDFromEpicId();

	static void OnQueryExternalAccountMappingsComplete(const EOS_Connect_QueryExternalAccountMappingsCallbackInfo* Data);

	void QueryExternalAccountMappingsSuccess();

	void QueryExternalAccountMappingsFailure();

	virtual void Activate() override; 


	UPROPERTY(BlueprintAssignable)
	FGetPUIDFromEpicIdDelegate Success;

	UPROPERTY(BlueprintAssignable)
	FGetPUIDFromEpicIdDelegate Failure;
	
};
