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

#include "EOSFindLobbiesAsync.h"
#include "EOSKitSubsystem.h"
#include "eos_lobby.h"
#include "eos_sdk.h"

UEOSFindLobbiesAsync* UEOSFindLobbiesAsync::FindLobbies(UObject* WorldContextObject)
{
	UEOSFindLobbiesAsync* FindLobbiesAsync = NewObject<UEOSFindLobbiesAsync>();
	FindLobbiesAsync->WorldContextObject = WorldContextObject;
	return FindLobbiesAsync;
}

void UEOSFindLobbiesAsync::Activate()
{
	if (UGameInstance* GameInstance = WorldContextObject->GetWorld()->GetGameInstance())
	{
		UEOSKitSubsystem* EOSKitSubsystem = GameInstance->GetSubsystem<UEOSKitSubsystem>();
		if (EOSKitSubsystem && EOSKitSubsystem->GetPlatformHandle())
		{
			EOS_HLobby LobbyHandle = EOS_Platform_GetLobbyInterface(EOSKitSubsystem->GetPlatformHandle());

			EOS_Lobby_CreateLobbySearchOptions CreateSearchOptions = {};
			CreateSearchOptions.ApiVersion = EOS_LOBBY_CREATELOBBYSEARCH_API_LATEST;
			CreateSearchOptions.MaxResults = 20;

			if (EOS_Lobby_CreateLobbySearch(LobbyHandle, &CreateSearchOptions, &SearchHandle) != EOS_EResult::EOS_Success)
			{
				OnFailure.Broadcast({});
				return;
			}

			EOS_LobbySearch_FindOptions FindOptions = {};
			FindOptions.ApiVersion = EOS_LOBBYSEARCH_FIND_API_LATEST;
			FindOptions.LocalUserId = EOSKitSubsystem->GetProductUserId();

			EOS_LobbySearch_Find(SearchHandle, &FindOptions, this, OnFindLobbiesComplete);
			return;
		}
	}
	OnFailure.Broadcast({});
}

void UEOSFindLobbiesAsync::OnFindLobbiesComplete(const EOS_LobbySearch_FindCallbackInfo* Data)
{
	UEOSFindLobbiesAsync* This = static_cast<UEOSFindLobbiesAsync*>(Data->ClientData);
	if (Data->ResultCode == EOS_EResult::EOS_Success)
	{
		TArray<FEOSLobbyInfo> FoundLobbies;
		
		EOS_LobbySearch_GetSearchResultCountOptions CountOptions = {};
		CountOptions.ApiVersion = EOS_LOBBYSEARCH_GETSEARCHRESULTCOUNT_API_LATEST;
		uint32_t ResultCount = EOS_LobbySearch_GetSearchResultCount(This->SearchHandle, &CountOptions);

		for (uint32_t i = 0; i < ResultCount; ++i)
		{
			EOS_LobbySearch_CopySearchResultByIndexOptions CopyOptions = {};
			CopyOptions.ApiVersion = EOS_LOBBYSEARCH_COPYSEARCHRESULTBYINDEX_API_LATEST;
			CopyOptions.LobbyIndex = i;
			
			EOS_HLobbyDetails LobbyDetailsHandle;
			if (EOS_LobbySearch_CopySearchResultByIndex(This->SearchHandle, &CopyOptions, &LobbyDetailsHandle) == EOS_EResult::EOS_Success)
			{
				FEOSLobbyInfo LobbyInfo;
				
				EOS_LobbyDetails_Info* LobbyDetailsInfo = nullptr;
				EOS_LobbyDetails_CopyInfoOptions CopyInfoOptions = {};
				CopyInfoOptions.ApiVersion = EOS_LOBBYDETAILS_COPYINFO_API_LATEST;
				EOS_LobbyDetails_CopyInfo(LobbyDetailsHandle, &CopyInfoOptions, &LobbyDetailsInfo);

				if (LobbyDetailsInfo)
				{
					LobbyInfo.LobbyId = LobbyDetailsInfo->LobbyId;
					LobbyInfo.MaxPlayers = LobbyDetailsInfo->MaxMembers;
					LobbyInfo.CurrentPlayers = LobbyDetailsInfo->MaxMembers - LobbyDetailsInfo->AvailableSlots;
					LobbyInfo.bIsPublic = LobbyDetailsInfo->PermissionLevel == EOS_ELobbyPermissionLevel::EOS_LPL_PUBLICADVERTISED;
					
					FoundLobbies.Add(LobbyInfo);
					EOS_LobbyDetails_Info_Release(LobbyDetailsInfo);
				}
				EOS_LobbyDetails_Release(LobbyDetailsHandle);
			}
		}
		This->OnSuccess.Broadcast(FoundLobbies);
	}
	else
	{
		This->OnFailure.Broadcast({});
	}

	EOS_LobbySearch_Release(This->SearchHandle);
	This->SetReadyToDestroy();
}
