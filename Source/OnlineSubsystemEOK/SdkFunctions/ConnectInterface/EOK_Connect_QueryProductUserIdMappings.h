// Copyright (c) 2023 Betide Studio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintAsyncActionBase.h"
#include "OnlineSubsystemEOK/SdkFunctions/EOK_SharedFunctionFile.h"
#include "EOK_Connect_QueryProductUserIdMappings.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FEOK_Connect_QueryProductUserIdMappings_Delegate, const FEOK_ProductUserId&, LocalUserId, TEnumAsByte<EEOK_Result>, Result);

UCLASS()
class OnlineSubsystemEOK_API UEOK_Connect_QueryProductUserIdMappings : public UBlueprintAsyncActionBase
{
	GENERATED_BODY()

public:

	//Retrieve the equivalent external account mappings from a list of Product User IDs. The values will be cached and retrievable via EOS_Connect_GetProductUserIdMapping, EOS_Connect_CopyProductUserExternalAccountByIndex, EOS_Connect_CopyProductUserExternalAccountByAccountType or EOS_Connect_CopyProductUserExternalAccountByAccountId.
	UFUNCTION(BlueprintCallable, Category = "Epic Online Services-Kit V2 | SDK Functions | Connect Interface", DisplayName="EOS_Connect_QueryProductUserIdMappings")
	static UEOK_Connect_QueryProductUserIdMappings* EOK_Connect_QueryProductUserIdMappings(FEOK_ProductUserId LocalUserId, const TArray<FEOK_ProductUserId>& TargetProductUserIds);

	UPROPERTY(BlueprintAssignable, Category = "Epic Online Services-Kit V2")
	FEOK_Connect_QueryProductUserIdMappings_Delegate OnCallback;
	
private:
	FEOK_ProductUserId Var_LocalUserId;
	TArray<FEOK_ProductUserId> Var_TargetProductUserIds;
	static void OnQueryProductUserIdMappingsCallback(const EOS_Connect_QueryProductUserIdMappingsCallbackInfo* Data);
	virtual void Activate() override;
	
};
