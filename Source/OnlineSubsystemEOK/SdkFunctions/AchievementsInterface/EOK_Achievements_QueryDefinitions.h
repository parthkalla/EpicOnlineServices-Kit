// Copyright (c) 2023 Betide Studio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintAsyncActionBase.h"
#include "OnlineSubsystemEOK/SdkFunctions/EOK_SharedFunctionFile.h"
#include "EOK_Achievements_QueryDefinitions.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnEOK_Achievements_QueryDefinitionsComplete, TEnumAsByte<EEOK_Result>, ResultCode);

UCLASS()
class OnlineSubsystemEOK_API UEOK_Achievements_QueryDefinitions : public UBlueprintAsyncActionBase
{
	GENERATED_BODY()

public:

	//Query for a list of definitions for all existing achievements, including localized text, icon IDs and whether an achievement is hidden.
	UFUNCTION(BlueprintCallable, Category="Epic Online Services-Kit V2 | SDK Functions | Achievements Interface", DisplayName="EOS_Achievements_QueryDefinitions")
	static UEOK_Achievements_QueryDefinitions* EOK_Achievements_QueryDefinitions(FEOK_ProductUserId UserId);

	UPROPERTY(BlueprintAssignable)
	FOnEOK_Achievements_QueryDefinitionsComplete OnCallback;
	
private:

	virtual void Activate() override;
	FEOK_ProductUserId Var_UserId;
	
};
