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
#if WITH_EOS_SDK
#include "Windows/AllowWindowsPlatformTypes.h"
#include "eos_sdk.h"
#include "eos_lobby.h"
#include "Windows/HideWindowsPlatformTypes.h"
#endif

// Context struct to keep string data alive during async operation
struct FCreateLobbyContext
{
	UEOSCreateLobbyAsync* AsyncNode = nullptr;
	TArray<uint8> BucketIdUTF8;
};

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
	UE_LOG(LogTemp, Warning, TEXT("EOSKit: ========================================"));
	UE_LOG(LogTemp, Warning, TEXT("EOSKit: CreateLobby::Activate() called"));
	UE_LOG(LogTemp, Warning, TEXT("EOSKit: MaxPlayers: %d, BucketId: %s, bIsPublic: %s"), 
		MaxPlayers, 
		*BucketId, 
		bIsPublic ? TEXT("true") : TEXT("false"));
	UE_LOG(LogTemp, Warning, TEXT("EOSKit: ========================================"));

	if (!WorldContextObject || !WorldContextObject->GetWorld())
	{
		UE_LOG(LogTemp, Error, TEXT("EOSKit: CreateLobby failed - WorldContextObject or World is null"));
		OnFailure.Broadcast(TEXT("WorldContextObject or World is null"));
		SetReadyToDestroy();
		return;
	}

	UGameInstance* GameInstance = WorldContextObject->GetWorld()->GetGameInstance();
	if (!GameInstance)
	{
		UE_LOG(LogTemp, Error, TEXT("EOSKit: CreateLobby failed - GameInstance is null"));
		OnFailure.Broadcast(TEXT("GameInstance is null"));
		SetReadyToDestroy();
		return;
	}

	UEOSKitSubsystem* EOSKitSubsystem = GameInstance->GetSubsystem<UEOSKitSubsystem>();
	if (!EOSKitSubsystem)
	{
		UE_LOG(LogTemp, Error, TEXT("EOSKit: CreateLobby failed - EOSKitSubsystem is null"));
		OnFailure.Broadcast(TEXT("EOSKitSubsystem is null"));
		SetReadyToDestroy();
		return;
	}

	if (!EOSKitSubsystem->GetPlatformHandle())
	{
		UE_LOG(LogTemp, Error, TEXT("EOSKit: CreateLobby failed - PlatformHandle is null"));
		OnFailure.Broadcast(TEXT("PlatformHandle is null"));
		SetReadyToDestroy();
		return;
	}

	EOS_HLobby LobbyHandle = EOS_Platform_GetLobbyInterface(EOSKitSubsystem->GetPlatformHandle());
	if (!LobbyHandle)
	{
		UE_LOG(LogTemp, Error, TEXT("EOSKit: CreateLobby failed - LobbyHandle is null"));
		OnFailure.Broadcast(TEXT("LobbyHandle is null"));
		SetReadyToDestroy();
		return;
	}

	EOS_ProductUserId LocalUserId = EOSKitSubsystem->GetProductUserId();
	if (!EOS_ProductUserId_IsValid(LocalUserId))
	{
		UE_LOG(LogTemp, Error, TEXT("EOSKit: CreateLobby failed - Invalid Local User ID. User must be logged in."));
		OnFailure.Broadcast(TEXT("Invalid Local User ID - User must be logged in"));
		SetReadyToDestroy();
		return;
	}

	// Log user ID for debugging
	char ProductUserIdStr[EOS_PRODUCTUSERID_MAX_LENGTH + 1];
	int32 ProductUserIdStrSize = sizeof(ProductUserIdStr);
	EOS_ProductUserId_ToString(LocalUserId, ProductUserIdStr, &ProductUserIdStrSize);
	UE_LOG(LogTemp, Log, TEXT("EOSKit: CreateLobby - LocalUserId: %s"), UTF8_TO_TCHAR(ProductUserIdStr));

	// Create context to keep data alive during async operation
	FCreateLobbyContext* Context = new FCreateLobbyContext();
	Context->AsyncNode = this;

	// Ensure BucketId has a default value if empty (EOS requires this)
	FString EffectiveBucketId = BucketId.IsEmpty() ? TEXT("MyGameBucket") : BucketId;
	UE_LOG(LogTemp, Log, TEXT("EOSKit: CreateLobby - Using BucketId: %s"), *EffectiveBucketId);

	// Convert BucketId to UTF8 and store in context
	FTCHARToUTF8 BucketIdConverter(*EffectiveBucketId);
	Context->BucketIdUTF8.Append((uint8*)BucketIdConverter.Get(), BucketIdConverter.Length() + 1);

	// Setup lobby creation options
	EOS_Lobby_CreateLobbyOptions CreateLobbyOptions = {};
	CreateLobbyOptions.ApiVersion = EOS_LOBBY_CREATELOBBY_API_LATEST;
	CreateLobbyOptions.LocalUserId = LocalUserId;
	CreateLobbyOptions.MaxLobbyMembers = MaxPlayers > 0 ? MaxPlayers : 4; // Ensure at least 1 connection
	CreateLobbyOptions.PermissionLevel = bIsPublic ? EOS_ELobbyPermissionLevel::EOS_LPL_PUBLICADVERTISED : EOS_ELobbyPermissionLevel::EOS_LPL_INVITEONLY;
	CreateLobbyOptions.bPresenceEnabled = EOS_TRUE; // Required for lobbies (bUsesPresence = true)
	CreateLobbyOptions.bAllowInvites = EOS_TRUE;
	CreateLobbyOptions.BucketId = (const char*)Context->BucketIdUTF8.GetData(); // CRITICAL: Set BucketId

	UE_LOG(LogTemp, Log, TEXT("EOSKit: CreateLobby - Settings: MaxMembers=%d, PermissionLevel=%d, bPresenceEnabled=true, bAllowInvites=true"),
		CreateLobbyOptions.MaxLobbyMembers, static_cast<int32>(CreateLobbyOptions.PermissionLevel));

	EOS_Lobby_CreateLobby(LobbyHandle, &CreateLobbyOptions, Context, OnCreateLobbyComplete);
	UE_LOG(LogTemp, Log, TEXT("EOSKit: CreateLobby - EOS_Lobby_CreateLobby called, waiting for callback..."));
}

void UEOSCreateLobbyAsync::OnCreateLobbyComplete(const EOS_Lobby_CreateLobbyCallbackInfo* Data)
{
	FCreateLobbyContext* Context = static_cast<FCreateLobbyContext*>(Data->ClientData);
	
	if (!Context || !Context->AsyncNode)
	{
		UE_LOG(LogTemp, Error, TEXT("EOSKit: CreateLobby callback - Invalid context"));
		if (Context)
		{
			delete Context;
		}
		return;
	}

	UEOSCreateLobbyAsync* This = Context->AsyncNode;

	if (Data->ResultCode == EOS_EResult::EOS_Success)
	{
		UE_LOG(LogTemp, Log, TEXT("EOSKit: ========================================"));
		UE_LOG(LogTemp, Log, TEXT("EOSKit: CreateLobby SUCCESS!"));
		UE_LOG(LogTemp, Log, TEXT("EOSKit: LobbyId: %s"), UTF8_TO_TCHAR(Data->LobbyId));
		UE_LOG(LogTemp, Log, TEXT("EOSKit: ========================================"));
		This->OnSuccess.Broadcast(UTF8_TO_TCHAR(Data->LobbyId));
	}
	else
	{
		const char* ResultStr = EOS_EResult_ToString(Data->ResultCode);
		UE_LOG(LogTemp, Error, TEXT("EOSKit: ========================================"));
		UE_LOG(LogTemp, Error, TEXT("EOSKit: CreateLobby FAILED!"));
		UE_LOG(LogTemp, Error, TEXT("EOSKit: Error Code: %s (%d)"), UTF8_TO_TCHAR(ResultStr), static_cast<int32>(Data->ResultCode));
		UE_LOG(LogTemp, Error, TEXT("EOSKit: ========================================"));
		This->OnFailure.Broadcast(UTF8_TO_TCHAR(ResultStr));
	}

	This->SetReadyToDestroy();
	delete Context;
}
