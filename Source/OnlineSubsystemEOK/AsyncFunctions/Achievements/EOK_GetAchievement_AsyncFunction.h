// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Runtime/Launch/Resources/Version.h"
#if ENGINE_MAJOR_VERSION == 5
#include "Online/CoreOnline.h"
#else
#include "UObject/CoreOnline.h"
#endif
#include "Kismet/BlueprintAsyncActionBase.h"
#include "EOK_GetAchievement_AsyncFunction.generated.h"

USTRUCT(BlueprintType)
struct FEOK_Achievement
{
	GENERATED_BODY()

public:
	/** The id of the achievement */
	UPROPERTY(BlueprintReadOnly, Category="EOK Struct")
	FString Id = "";

	/** The progress towards completing this achievement: 0.0-100.0 */
	UPROPERTY(BlueprintReadOnly, Category="EOK Struct")
	float Progress = 0.0;


};


/**
 * 
 */

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FAchievement_Delegate, const TArray<FEOK_Achievement>&, Achievements);

UCLASS()
class OnlineSubsystemEOK_API UEOK_GetAchievement_AsyncFunction : public UBlueprintAsyncActionBase
{
	GENERATED_BODY()

public:

	UPROPERTY(BlueprintAssignable, DisplayName="Success")
	FAchievement_Delegate OnSuccess;
	UPROPERTY(BlueprintAssignable, DisplayName="Failure")
	FAchievement_Delegate OnFail;
	
	/*
	This C++ method gets the logged in user's achievements from the EOS backend.
	Documentation link: https://betide-studio.gitbook.io/eos-integration-kit/authentication/
	*/
	UFUNCTION(BlueprintCallable, DisplayName="Get EOK Achievements",meta = (BlueprintInternalUseOnly = "true"), Category="Epic Online Services-Kit V2 | Achievements")
	static UEOK_GetAchievement_AsyncFunction* GetEOKAchievements();


	void Activate() override;


	void GetAchievements();

	bool bDelegateCalled = false;


	void OnAchievementsCompleted(const FUniqueNetId& UniqueNetId, bool bWasSuccess);


};
