// Copyright (C) 2024, All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintAsyncActionBase.h"
#include "EOSKitAchievementsTypes.h"
#include "EOSQueryDefinitionsAsync.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnQueryDefinitionsComplete, const TArray<FEOSAchievementDefinition>&, Definitions);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnQueryDefinitionsFail, const FString&, ErrorMessage);

/**
 * Query achievement definitions from EOS
 */
UCLASS()
class EOSKITSTATS_API UEOSQueryDefinitionsAsync : public UBlueprintAsyncActionBase
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintAssignable)
	FOnQueryDefinitionsComplete OnSuccess;

	UPROPERTY(BlueprintAssignable)
	FOnQueryDefinitionsFail OnFail;

	/**
	 * Query all achievement definitions from EOS
	 * 
	 * @param WorldContextObject World context
	 * @param LocalUserId The Product User ID of the local player
	 */
	UFUNCTION(BlueprintCallable, meta = (BlueprintInternalUseOnly = "true", WorldContext = "WorldContextObject"), Category = "EOSKit|Achievements")
	static UEOSQueryDefinitionsAsync* QueryAchievementDefinitions(
		UObject* WorldContextObject,
		const FString& LocalUserId
	);

	virtual void Activate() override;

	void PerformQuery();

	UObject* WorldContextObject;
	FString UserId;
};
