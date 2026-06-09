// Copyright (c) 2023 Betide Studio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "OnlineSubsystemEOK/SdkFunctions/EOK_SharedFunctionFile.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "EOK_AchievementsSubsystem.generated.h"

DECLARE_DYNAMIC_DELEGATE_ThreeParams(FOnAchievementsUnlockedV2Callback, FEOK_ProductUserId, UserId, FString, AchievementId, int64, UnlockTime);

UCLASS()
class OnlineSubsystemEOK_API UEOK_AchievementsSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	FOnAchievementsUnlockedV2Callback OnAchievementsUnlockedV2;

	UFUNCTION(BlueprintCallable, Category="Epic Online Services-Kit V2 | SDK Functions | Achievements Interface", DisplayName="EOS_Achievements_AddNotifyAchievementsUnlockedV2")
	FEOK_NotificationId EOK_Achievements_AddNotifyAchievementsUnlockedV2(FOnAchievementsUnlockedV2Callback Callback);

	UFUNCTION(BlueprintCallable, Category="Epic Online Services-Kit V2 | SDK Functions | Achievements Interface", DisplayName="EOS_Achievements_CopyAchievementDefinitionV2ByAchievementId")
	TEnumAsByte<EEOK_Result> EOK_Achievements_CopyAchievementDefinitionV2ByAchievementId(FString AchievementId, FEOK_Achievements_DefinitionV2& OutAchievementDefinition);

	//Fetches an achievement definition from a given index.
	UFUNCTION(BlueprintCallable, Category="Epic Online Services-Kit V2 | SDK Functions | Achievements Interface", DisplayName="EOS_Achievements_CopyAchievementDefinitionV2ByIndex")
	TEnumAsByte<EEOK_Result> EOK_Achievements_CopyAchievementDefinitionV2ByIndex(int32 Index, FEOK_Achievements_DefinitionV2& OutAchievementDefinition);

	//Fetches a player achievement from a given achievement ID.
	UFUNCTION(BlueprintCallable, Category="Epic Online Services-Kit V2 | SDK Functions | Achievements Interface", DisplayName="EOS_Achievements_CopyPlayerAchievementByAchievementId")
	TEnumAsByte<EEOK_Result> EOK_Achievements_CopyPlayerAchievementByAchievementId(FEOK_ProductUserId TargetUserId, FString AchievementId, FEOK_ProductUserId LocalUserId, FEOK_Achievements_PlayerAchievement& OutPlayerAchievement);

	//Fetches a player achievement from a given index.
	UFUNCTION(BlueprintCallable, Category="Epic Online Services-Kit V2 | SDK Functions | Achievements Interface", DisplayName="EOS_Achievements_CopyPlayerAchievementByIndex")
	TEnumAsByte<EEOK_Result> EOK_Achievements_CopyPlayerAchievementByIndex(FEOK_ProductUserId TargetUserId, int32 Index, FEOK_ProductUserId LocalUserId, FEOK_Achievements_PlayerAchievement& OutPlayerAchievement);

	//Release the memory associated with EOS_Achievements_DefinitionV2. This must be called on data retrieved from EOS_Achievements_CopyAchievementDefinitionV2ByIndex or EOS_Achievements_CopyAchievementDefinitionV2ByAchievementId.
	UFUNCTION(BlueprintCallable, Category="Epic Online Services-Kit V2 | SDK Functions | Achievements Interface", DisplayName="EOS_Achievements_DefinitionV2_Release")
	void EOK_Achievements_DefinitionV2_Release(FEOK_Achievements_DefinitionV2& AchievementDefinition);

	//Fetch the number of achievement definitions that are cached locally.
	UFUNCTION(BlueprintCallable, Category="Epic Online Services-Kit V2 | SDK Functions | Achievements Interface", DisplayName="EOS_Achievements_GetAchievementDefinitionCount")
	int32 EOK_Achievements_GetAchievementDefinitionCount();

	//Fetch the number of player achievements that are cached locally.
	UFUNCTION(BlueprintCallable, Category="Epic Online Services-Kit V2 | SDK Functions | Achievements Interface", DisplayName="EOS_Achievements_GetPlayerAchievementCount")
	int32 EOK_Achievements_GetPlayerAchievementCount(FEOK_ProductUserId UserId);

	//Release the memory associated with a player achievement. This must be called on data retrieved from EOS_Achievements_CopyPlayerAchievementByIndex or EOS_Achievements_CopyPlayerAchievementByAchievementId.
	UFUNCTION(BlueprintCallable, Category="Epic Online Services-Kit V2 | SDK Functions | Achievements Interface", DisplayName="EOS_Achievements_PlayerAchievement_Release")
	void EOK_Achievements_PlayerAchievement_Release(FEOK_Achievements_PlayerAchievement& PlayerAchievement);

	//Query for a list of definitions for all existing achievements, including localized text, icon IDs and whether an achievement is hidden.
	UFUNCTION(BlueprintCallable, Category="Epic Online Services-Kit V2 | SDK Functions | Achievements Interface", DisplayName="EOS_Achievements_QueryDefinitions")
	TEnumAsByte<EEOK_Result> EOK_Achievements_QueryDefinitions(FEOK_ProductUserId UserId);

	//Unregister from receiving achievement unlocked notifications.
	UFUNCTION(BlueprintCallable, Category="Epic Online Services-Kit V2 | SDK Functions | Achievements Interface", DisplayName="EOS_Achievements_RemoveNotifyAchievementsUnlocked")
	void EOK_Achievements_RemoveNotifyAchievementsUnlocked(FEOK_NotificationId Id);



};
