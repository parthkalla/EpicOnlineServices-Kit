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

#include "EOSQueryLeaderboardsAsync.h"
#include "EOSKitSubsystem.h"
#if WITH_EOS_SDK
#include "Windows/AllowWindowsPlatformTypes.h"
#include "Windows/PreWindowsApi.h"
#include "eos_platform.h"
#include "eos_leaderboards.h"
#include "eos_sdk.h"
#include "Windows/PostWindowsApi.h"
#include "Windows/HideWindowsPlatformTypes.h"
#endif

UEOSQueryLeaderboardsAsync* UEOSQueryLeaderboardsAsync::QueryLeaderboards(UObject* WorldContextObject)
{
	UEOSQueryLeaderboardsAsync* QueryLeaderboardsAsync = NewObject<UEOSQueryLeaderboardsAsync>();
	QueryLeaderboardsAsync->WorldContextObject = WorldContextObject;
	return QueryLeaderboardsAsync;
}

void UEOSQueryLeaderboardsAsync::Activate()
{
	if (UGameInstance* GameInstance = WorldContextObject->GetWorld()->GetGameInstance())
	{
		UEOSKitSubsystem* EOSKitSubsystem = GameInstance->GetSubsystem<UEOSKitSubsystem>();
		if (EOSKitSubsystem && EOSKitSubsystem->GetPlatformHandle())
		{
			EOS_HLeaderboards LeaderboardsHandle = EOS_Platform_GetLeaderboardsInterface(EOSKitSubsystem->GetPlatformHandle());

			EOS_Leaderboards_QueryLeaderboardDefinitionsOptions QueryOptions = {};
			QueryOptions.ApiVersion = EOS_LEADERBOARDS_QUERYLEADERBOARDDEFINITIONS_API_LATEST;
			QueryOptions.LocalUserId = EOSKitSubsystem->GetProductUserId();
			
			EOS_Leaderboards_QueryLeaderboardDefinitions(LeaderboardsHandle, &QueryOptions, this, OnQueryLeaderboardsComplete);
			return;
		}
	}
	OnFailure.Broadcast({});
}

void UEOSQueryLeaderboardsAsync::OnQueryLeaderboardsComplete(const EOS_Leaderboards_OnQueryLeaderboardDefinitionsCompleteCallbackInfo* Data)
{
	UEOSQueryLeaderboardsAsync* This = static_cast<UEOSQueryLeaderboardsAsync*>(Data->ClientData);
	if (Data->ResultCode == EOS_EResult::EOS_Success)
	{
		TArray<FEOSLeaderboardDefinition> FoundLeaderboards;
		uint32_t DefinitionCount = 0;
		
		UGameInstance* GameInstance = This->WorldContextObject->GetWorld()->GetGameInstance();
		UEOSKitSubsystem* EOSKitSubsystem = GameInstance->GetSubsystem<UEOSKitSubsystem>();
		EOS_HLeaderboards LeaderboardsHandle = EOS_Platform_GetLeaderboardsInterface(EOSKitSubsystem->GetPlatformHandle());

		EOS_Leaderboards_GetLeaderboardDefinitionCountOptions CountOptions = {};
		CountOptions.ApiVersion = EOS_LEADERBOARDS_GETLEADERBOARDDEFINITIONCOUNT_API_LATEST;
		DefinitionCount = EOS_Leaderboards_GetLeaderboardDefinitionCount(LeaderboardsHandle, &CountOptions);

		for (uint32_t i = 0; i < DefinitionCount; ++i)
		{
			EOS_Leaderboards_CopyLeaderboardDefinitionByIndexOptions CopyOptions = {};
			CopyOptions.ApiVersion = EOS_LEADERBOARDS_COPYLEADERBOARDDEFINITIONBYINDEX_API_LATEST;
			CopyOptions.LeaderboardIndex = i;
			
			EOS_Leaderboards_Definition* Def = nullptr;
			if (EOS_Leaderboards_CopyLeaderboardDefinitionByIndex(LeaderboardsHandle, &CopyOptions, &Def) == EOS_EResult::EOS_Success)
			{
				FEOSLeaderboardDefinition LeaderboardDef;
				LeaderboardDef.LeaderboardId = Def->LeaderboardId;
				LeaderboardDef.StatName = Def->StatName;
				FoundLeaderboards.Add(LeaderboardDef);
				EOS_Leaderboards_Definition_Release(Def);
			}
		}
		This->OnSuccess.Broadcast(FoundLeaderboards);
	}
	else
	{
		This->OnFailure.Broadcast({});
	}
	This->SetReadyToDestroy();
}
