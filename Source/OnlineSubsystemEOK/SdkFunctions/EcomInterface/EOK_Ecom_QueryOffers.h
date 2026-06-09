// Copyright (c) 2023 Betide Studio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintAsyncActionBase.h"
#include "OnlineSubsystemEOK/SdkFunctions/EOK_SharedFunctionFile.h"
#include "EOK_Ecom_QueryOffers.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FEOK_Ecom_QueryOffersCallback, FEOK_EpicAccountId, LocalUserId, const TEnumAsByte<EEOK_Result>&, ResultCode);

UCLASS()
class OnlineSubsystemEOK_API UEOK_Ecom_QueryOffers : public UBlueprintAsyncActionBase
{
	GENERATED_BODY()

public:
	
	//Query for a list of catalog offers defined with Epic Online Services. This data will be cached for a limited time and retrieved again from the backend when necessary.
	UFUNCTION(BlueprintCallable, Category = "Epic Online Services-Kit V2 | SDK Functions | Ecom Interface", DisplayName = "EOS_Ecom_QueryOffers")
	static UEOK_Ecom_QueryOffers* EOK_Ecom_QueryOffers(FEOK_EpicAccountId LocalUserId, FString OverrideCatalogNamespace);

	UPROPERTY(BlueprintAssignable)
	FEOK_Ecom_QueryOffersCallback OnCallback;
private:
	FEOK_EpicAccountId Var_LocalUserId;
	FString Var_OverrideCatalogNamespace;
	static void EOS_CALL OnQueryOffersCallback(const EOS_Ecom_QueryOffersCallbackInfo* Data);
	virtual void Activate() override;
};
