// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Interfaces/OnlineStatsInterface.h"
#include "Runtime/Launch/Resources/Version.h"
#include "Kismet/BlueprintAsyncActionBase.h"
#include "EOK_GetStats_AsyncFunction.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FGetStatsResult, const TArray<FEOK_Stats>&, Stats);

UCLASS()
class OnlineSubsystemEOK_API UEOK_GetStats_AsyncFunction : public UBlueprintAsyncActionBase
{
	GENERATED_BODY()

public:

	TArray<FString> StatName;

	UPROPERTY(BlueprintAssignable)
	FGetStatsResult OnSuccess;
	
	UPROPERTY(BlueprintAssignable)
	FGetStatsResult OnFail;

	bool bDelegateCalled = false;
	/*
	This C++ method gets the stats in the online subsystem using the selected method and sets up a callback function to handle the response.
	Documentation link: https://betide-studio.gitbook.io/eos-integration-kit/stats/
	For Input Parameters, please refer to the documentation link above.
	*/
	UFUNCTION(BlueprintCallable, DisplayName="Get EOK Stats", meta = (BlueprintInternalUseOnly = "true"), Category="Epic Online Services-Kit V2 | Stats")
	static UEOK_GetStats_AsyncFunction* GetStats( TArray<FString> StatName);

	virtual void Activate() override;
	void GetStats();
	void OnGetStatsCompleted(const FOnlineError &ResultState, const TArray<TSharedRef<const FOnlineStatsUserStats>> &UsersStatsResult);

};
