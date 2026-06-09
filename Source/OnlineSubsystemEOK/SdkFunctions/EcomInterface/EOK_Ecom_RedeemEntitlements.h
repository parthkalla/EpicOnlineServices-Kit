// Copyright (c) 2023 Betide Studio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintAsyncActionBase.h"
#include "OnlineSubsystemEOK/SdkFunctions/EOK_SharedFunctionFile.h"
#include "EOK_Ecom_RedeemEntitlements.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FEOK_Ecom_RedeemEntitlementsCallback, const FEOK_EpicAccountId&, LocalUserId, const TEnumAsByte<EEOK_Result>, ResultCode, int32, RedeemedEntitlementIdsCount);

UCLASS()
class OnlineSubsystemEOK_API UEOK_Ecom_RedeemEntitlements : public UBlueprintAsyncActionBase
{
	GENERATED_BODY()

public:

	//Requests that the provided entitlement be marked redeemed. This will cause that entitlement to no longer be returned from QueryEntitlements unless the include redeemed request flag is set true.
	UFUNCTION(BlueprintCallable, Category = "Epic Online Services-Kit V2 | SDK Functions | Ecom Interface", DisplayName="EOS_Ecom_RedeemEntitlements")
	static UEOK_Ecom_RedeemEntitlements* EOK_Ecom_RedeemEntitlements(const FEOK_EpicAccountId& LocalUserId, const TArray<FEOK_Ecom_EntitlementId>& EntitlementIds);
	
	UPROPERTY(BlueprintAssignable, Category = "Epic Online Services-Kit V2 | SDK Functions | Ecom Interface")
	FEOK_Ecom_RedeemEntitlementsCallback OnCallback;
private:
	static void EOS_CALL OnRedeemEntitlementsCallback(const EOS_Ecom_RedeemEntitlementsCallbackInfo* Data);
	virtual void Activate() override;
	FEOK_EpicAccountId Var_LocalUserId;
	TArray<FEOK_Ecom_EntitlementId> Var_EntitlementIds;
};
