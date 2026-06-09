// Copyright (c) 2023 Betide Studio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintAsyncActionBase.h"
#include "eos_achievements.h"
#include "OnlineSubsystemEOK/SdkFunctions/EOK_SharedFunctionFile.h"
#include "EOK_Achievements_QueryPlayerAchievements.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnEOK_Achievements_QueryPlayerAchievementsComplete, TEnumAsByte<EEOK_Result>, ResultCode, FEOK_ProductUserId, LocalUserId, FEOK_ProductUserId, TargetUserId);
UCLASS()
class OnlineSubsystemEOK_API UEOK_Achievements_QueryPlayerAchievements : public UBlueprintAsyncActionBase
{
	GENERATED_BODY()

public:

	//Query for a list of achievements for a specific player, including progress towards completion for each achievement.
	UFUNCTION(BlueprintCallable, Category="Epic Online Services-Kit V2 | SDK Functions | Achievements Interface", DisplayName="EOS_Achievements_QueryPlayerAchievements")
	static UEOK_Achievements_QueryPlayerAchievements* EOK_Achievements_QueryPlayerAchievements(FEOK_ProductUserId LocalUserId, FEOK_ProductUserId TargetUserId);

	UPROPERTY(BlueprintAssignable)
	FOnEOK_Achievements_QueryPlayerAchievementsComplete OnCallback;
private:
	FEOK_ProductUserId Var_LocalUserId;
	FEOK_ProductUserId Var_TargetUserId;
	virtual void Activate() override;
};
