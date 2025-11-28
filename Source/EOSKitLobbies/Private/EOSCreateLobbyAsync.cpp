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

#include "EOSCreateLobbyAsync.h"
#include "EOSKitSubsystem.h"
#include "eos_lobby.h"
#include "eos_sdk.h"

UEOSCreateLobbyAsync* UEOSCreateLobbyAsync::CreateLobby(UObject* WorldContextObject, int32 MaxPlayers, const FString& BucketId, bool bIsPublic)
{
	UEOSCreateLobbyAsync* CreateLobbyAsync = NewObject<UEOSCreateLobbyAsync>();
	CreateLobbyAsync->WorldContextObject = WorldContextObject;
	CreateLobbyAsync->MaxPlayers = MaxPlayers;
	CreateLobbyAsync->BucketId = BucketId;
	CreateLobbyAsync->bIsPublic = bIsPublic;
	return CreateLobbyAsync;
}

void UEOSCreateLobbyAsync::Activate()
{
	if (UGameInstance* GameInstance = WorldContextObject->GetWorld()->GetGameInstance())
	{
		UEOSKitSubsystem* EOSKitSubsystem = GameInstance->GetSubsystem<UEOSKitSubsystem>();
		if (EOSKitSubsystem && EOSKitSubsystem->GetPlatformHandle())
		{
			EOS_HLobby LobbyHandle = EOS_Platform_GetLobbyInterface(EOSKitSubsystem->GetPlatformHandle());

			EOS_Lobby_CreateLobbyOptions CreateLobbyOptions = {};
			CreateLobbyOptions.ApiVersion = EOS_LOBBY_CREATELOBBY_API_LATEST;
			
			EOS_ProductUserId LocalUserId = EOSKitSubsystem->GetProductUserId();
			if (!EOS_ProductUserId_IsValid(LocalUserId))
			{
				OnFailure.Broadcast("Invalid Local User ID");
				return;
			}
			CreateLobbyOptions.LocalUserId = LocalUserId;
			CreateLobbyOptions.MaxLobbyMembers = MaxPlayers;
			CreateLobbyOptions.PermissionLevel = bIsPublic ? EOS_ELobbyPermissionLevel::EOS_LPL_PUBLICADVERTISED : EOS_ELobbyPermissionLevel::EOS_LPL_INVITEONLY;
			
			// Add bucket id as an attribute
			EOS_Lobby_AttributeData BucketAttribute;
			BucketAttribute.ApiVersion = EOS_LOBBY_ATTRIBUTEDATA_API_LATEST;
			BucketAttribute.Key = "BUCKET";
			BucketAttribute.Value.AsUtf8 = TCHAR_TO_UTF8(*BucketId);
			
			EOS_Lobby_CreateLobbyOptions ExtendedOptions = {};
			ExtendedOptions.ApiVersion = EOS_LOBBY_CREATELOBBY_API_LATEST;
			ExtendedOptions.LocalUserId = LocalUserId;
			ExtendedOptions.MaxLobbyMembers = MaxPlayers;
			ExtendedOptions.PermissionLevel = bIsPublic ? EOS_ELobbyPermissionLevel::EOS_LPL_PUBLICADVERTISED : EOS_ELobbyPermissionLevel::EOS_LPL_INVITEONLY;
			
			EOS_Lobby_CreateLobby(LobbyHandle, &ExtendedOptions, this, OnCreateLobbyComplete);
			return;
		}
	}
	OnFailure.Broadcast("EOSKitSubsystem not available.");
}

void UEOSCreateLobbyAsync::OnCreateLobbyComplete(const EOS_Lobby_CreateLobbyCallbackInfo* Data)
{
	UEOSCreateLobbyAsync* This = static_cast<UEOSCreateLobbyAsync*>(Data->ClientData);
	if (Data->ResultCode == EOS_EResult::EOS_Success)
	{
		This->OnSuccess.Broadcast(Data->LobbyId);
	}
	else
	{
		This->OnFailure.Broadcast(EOS_EResult_ToString(Data->ResultCode));
	}
	This->SetReadyToDestroy();
}
