// Copyright (C) 2024, All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintAsyncActionBase.h"
#include "EOSUnlockAchievementAsync.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnUnlockAchievementComplete);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnUnlockAchievementFail, const FString&, ErrorMessage);

/**
 * Unlock an achievement for the local player
 */
UCLASS()
class EOSKITACHIEVEMENTS_API UEOSUnlockAchievementAsync : public UBlueprintAsyncActionBase
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintAssignable)
	FOnUnlockAchievementComplete OnSuccess;

	UPROPERTY(BlueprintAssignable)
	FOnUnlockAchievementFail OnFail;

	/**
	 * Unlock an achievement for the local player
	 * 
	 * @param WorldContextObject World context
	 * @param LocalUserId The Product User ID of the local player
	 * @param AchievementIds Array of achievement IDs to unlock
	 */
	UFUNCTION(BlueprintCallable, meta = (BlueprintInternalUseOnly = "true", WorldContext = "WorldContextObject"), Category = "EOSKit|Achievements")
	static UEOSUnlockAchievementAsync* UnlockAchievements(
		UObject* WorldContextObject,
		const FString& LocalUserId,
		const TArray<FString>& AchievementIds
	);

	virtual void Activate() override;

	void PerformUnlock();

	UObject* WorldContextObject;
	FString UserId;
	TArray<FString> AchievementIdsToUnlock;
};
