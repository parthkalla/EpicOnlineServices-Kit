// Copyright (c) 2023 Betide Studio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintAsyncActionBase.h"
#include "OnlineSubsystemEOK/SdkFunctions/EOK_SharedFunctionFile.h"
#include "EOK_P2P_QueryNATType.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FEOK_P2P_QueryNATTypeDelegate, const TEnumAsByte<EEOK_Result>&, Result, const TEnumAsByte<EEOK_ENATType>&, NATType);

UCLASS()
class OnlineSubsystemEOK_API UEOK_P2P_QueryNATType : public UBlueprintAsyncActionBase
{
	GENERATED_BODY()

public:

	//Query the current NAT-type of our connection.
	UFUNCTION(BlueprintCallable, Category = "Epic Online Services-Kit V2 | SDK Functions | P2P Interface", DisplayName="EOS_P2P_QueryNATType")
	static UEOK_P2P_QueryNATType* EOK_P2P_QueryNATType();

	UPROPERTY(BlueprintAssignable)
	FEOK_P2P_QueryNATTypeDelegate OnCallback;
	
private:
	virtual void Activate() override;
	static void EOS_CALL EOS_P2P_QueryNATType_Callback(const EOS_P2P_OnQueryNATTypeCompleteInfo* Data);
	
};
