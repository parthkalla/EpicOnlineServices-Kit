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
#include "EOK_GetAchievement_AsyncFunction.h"
#include "EOK_GetAchievementDetails_AsyncFunction.generated.h"

USTRUCT(BlueprintType)
struct FEOK_AchievementDescription
{
	GENERATED_BODY()

public:

	/** The id of the achievement */
	UPROPERTY(BlueprintReadOnly, Category="EOK Struct")
	FString Id = "";

	/** The progress towards completing this achievement: 0.0-100.0 */
	UPROPERTY(BlueprintReadOnly, Category="EOK Struct")
	float Progress = 0.0;
	
	/** The localized title of the achievement */
	UPROPERTY(BlueprintReadOnly, Category="EOK Struct")
	FText Title = FText::FromString("");

	/** The localized locked description of the achievement */
	UPROPERTY(BlueprintReadOnly, Category="EOK Struct")
	FText LockedDesc = FText::FromString("");

	/** The localized unlocked description of the achievement */
	UPROPERTY(BlueprintReadOnly, Category="EOK Struct")
	FText UnlockedDesc = FText::FromString("");

	/** Flag for whether the achievement is hidden */
	UPROPERTY(BlueprintReadOnly, Category="EOK Struct")
	bool bIsHidden = false;

	/** The date/time the achievement was unlocked */
	UPROPERTY(BlueprintReadOnly, Category="EOK Struct")
	FDateTime UnlockTime;


};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FAchievementDetails_Delegate, const FEOK_AchievementDescription&, AchievementDescription);

UCLASS()
class OnlineSubsystemEOK_API UEOK_GetAchievementDetails_AsyncFunction : public UBlueprintAsyncActionBase
{
	GENERATED_BODY()

public:

	FEOK_Achievement Var_Achievement;

	UPROPERTY(BlueprintAssignable, DisplayName="Success")
	FAchievementDetails_Delegate OnSuccess;
	UPROPERTY(BlueprintAssignable, DisplayName="Failure")
	FAchievementDetails_Delegate OnFail;
	/*
	This C++ method gets the achievement's details from the EOS backend.
	Documentation link: https://betide-studio.gitbook.io/eos-integration-kit/authentication/
	*/
	UFUNCTION(BlueprintCallable, DisplayName="Get EOK Achievement Description",meta = (BlueprintInternalUseOnly = "true"), Category="Epic Online Services-Kit V2 | Achievements")
	static UEOK_GetAchievementDetails_AsyncFunction* GetEOKAchievementDescription(FEOK_Achievement Achievement);

	void Activate() override;

	void GetAchievementDescription();

	bool bDelegateCalled = false;

	void OnAchievementDescriptionCompleted(const FUniqueNetId& UniqueNetId, bool bWasSuccess);

};
