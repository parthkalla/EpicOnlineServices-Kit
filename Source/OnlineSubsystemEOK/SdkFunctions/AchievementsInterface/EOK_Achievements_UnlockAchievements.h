// Copyright (c) 2023 Betide Studio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintAsyncActionBase.h"
#include "eos_achievements.h"
#include "OnlineSubsystemEOK/SdkFunctions/EOK_SharedFunctionFile.h"
#include "EOK_Achievements_UnlockAchievements.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnEOK_Achievements_UnlockAchievementsComplete, TEnumAsByte<EEOK_Result>, ResultCode, FEOK_ProductUserId, UserId, int32, AchievementCount);
UCLASS()
class OnlineSubsystemEOK_API UEOK_Achievements_UnlockAchievements : public UBlueprintAsyncActionBase
{
	GENERATED_BODY()

public:

	UFUNCTION(BlueprintCallable, Category="Epic Online Services-Kit V2 | SDK Functions | Achievements Interface", DisplayName="EOS_Achievements_UnlockAchievements")
	static UEOK_Achievements_UnlockAchievements* EOK_Achievements_UnlockAchievements(FEOK_ProductUserId UserId, TArray<FString> AchievementIds);

	UPROPERTY(BlueprintAssignable)
	FOnEOK_Achievements_UnlockAchievementsComplete OnCallback;

private:
	virtual void Activate() override;
	FEOK_ProductUserId Var_UserId;
	TArray<FString> Var_AchievementIds;
};
