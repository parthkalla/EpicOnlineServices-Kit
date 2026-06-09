// Copyright (c) 2023 Betide Studio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintAsyncActionBase.h"
#include "OnlineSubsystemEOK/SdkFunctions/EOK_SharedFunctionFile.h"
#include "EOK_Connect_QueryExternalAccountMappings.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FEOK_Connect_QueryExternalAccountMappings_Delegate, const FEOK_ProductUserId&, ProductUserId, TEnumAsByte<EEOK_Result>, Result);

UCLASS()
class OnlineSubsystemEOK_API UEOK_Connect_QueryExternalAccountMappings : public UBlueprintAsyncActionBase
{
	GENERATED_BODY()

public:

	//Retrieve the equivalent Product User IDs from a list of external account IDs from supported account providers. The values will be cached and retrievable through EOS_Connect_GetExternalAccountMapping. Queries using external account IDs of another account system may not be available, depending on the account system specifics.
	UFUNCTION(BlueprintCallable, Category = "Epic Online Services-Kit V2 | SDK Functions | Connect Interface", DisplayName="EOS_Connect_QueryExternalAccountMappings")
	static UEOK_Connect_QueryExternalAccountMappings* EOK_Connect_QueryExternalAccountMappings(FEOK_ProductUserId ProductUserId, TEnumAsByte<EEOK_EExternalAccountType> AccountType, const TArray<FString>& ExternalAccountIds);

	UPROPERTY(BlueprintAssignable, Category = "Epic Online Services-Kit V2")
	FEOK_Connect_QueryExternalAccountMappings_Delegate OnCallback;
private:
	static void OnQueryExternalAccountMappingsCallback(const EOS_Connect_QueryExternalAccountMappingsCallbackInfo* Data);
	virtual void Activate() override;
	
	FEOK_ProductUserId Var_ProductUserId;
	TEnumAsByte<EEOK_EExternalAccountType> Var_AccountType;
	TArray<FString> Var_ExternalAccountIds;
	

	
};
