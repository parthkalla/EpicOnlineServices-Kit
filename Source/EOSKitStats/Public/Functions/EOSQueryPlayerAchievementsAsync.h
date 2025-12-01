// Copyright (C) 2024, All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintAsyncActionBase.h"
#include "EOSKitAchievementsTypes.h"
#include "EOSQueryPlayerAchievementsAsync.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnQueryPlayerAchievementsComplete, const TArray<FEOSPlayerAchievement>&, Achievements);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnQueryPlayerAchievementsFail, const FString&, ErrorMessage);

/**
 * Query player achievement progress from EOS
 */
UCLASS()
class EOSKITSTATS_API UEOSQueryPlayerAchievementsAsync : public UBlueprintAsyncActionBase
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintAssignable)
	FOnQueryPlayerAchievementsComplete OnSuccess;

	UPROPERTY(BlueprintAssignable)
	FOnQueryPlayerAchievementsFail OnFail;

	/**
	 * Query player achievement progress from EOS
	 * 
	 * @param WorldContextObject World context
	 * @param LocalUserId The Product User ID of the local player
	 * @param TargetUserId The Product User ID of the player to query (can be same as LocalUserId)
	 */
	UFUNCTION(BlueprintCallable, meta = (BlueprintInternalUseOnly = "true", WorldContext = "WorldContextObject"), Category = "EOSKit|Achievements")
	static UEOSQueryPlayerAchievementsAsync* QueryPlayerAchievements(
		UObject* WorldContextObject,
		const FString& LocalUserId,
		const FString& TargetUserId
	);

	virtual void Activate() override;

	void PerformQuery();

	UObject* WorldContextObject;
	FString LocalId;
	FString TargetId;
};
