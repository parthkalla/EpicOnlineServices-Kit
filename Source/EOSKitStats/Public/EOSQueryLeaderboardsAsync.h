// Copyright (C) 2024, All Rights Reserved.
//
// This file is part of the EOSKit Plugin.
//
// EOSKit is free software: you can redistribute it and/or modify
// it under the terms of the MIT License as published by the Open
// Source Initiative.
//
// EOSKit is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// MIT License for more details.
//
// You should have received a copy of the MIT License along with
// EOSKit. If not, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintAsyncActionBase.h"
#include "EOSKitTypes.h"
#include "eos_leaderboards.h"
#include "EOSQueryLeaderboardsAsync.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnQueryLeaderboardsCallback, const TArray<FEOSLeaderboardDefinition>&, Leaderboards);

UCLASS()
class EOSKITSTATS_API UEOSQueryLeaderboardsAsync : public UBlueprintAsyncActionBase
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintAssignable)
	FOnQueryLeaderboardsCallback OnSuccess;

	UPROPERTY(BlueprintAssignable)
	FOnQueryLeaderboardsCallback OnFailure;

	UFUNCTION(BlueprintCallable, meta = (BlueprintInternalUseOnly = "true", WorldContext = "WorldContextObject"), Category = "EOSKit|Leaderboards")
	static UEOSQueryLeaderboardsAsync* QueryLeaderboards(UObject* WorldContextObject);

	virtual void Activate() override;

private:
	static void EOS_CALL OnQueryLeaderboardsComplete(const EOS_Leaderboards_OnQueryLeaderboardDefinitionsCompleteCallbackInfo* Data);

	UObject* WorldContextObject;
};
