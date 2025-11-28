// Copyright (C) 2024, All Rights Reserved.

#include "EOSKickMemberAsync.h"
#include "EOSKitSubsystem.h"
#include "Kismet/GameplayStatics.h"
#include "eos_lobby.h"
#include "eos_lobby_types.h"
#include "Async/Async.h"

UEOSKickMemberAsync* UEOSKickMemberAsync::KickMember(FString LobbyId, FString LocalUserId, FString TargetUserId)
{
	UEOSKickMemberAsync* Node = NewObject<UEOSKickMemberAsync>();
	Node->VarLobbyId = LobbyId;
	Node->VarLocalUserId = LocalUserId;
	Node->VarTargetUserId = TargetUserId;
	return Node;
}

void UEOSKickMemberAsync::Activate()
{
	PerformKick();
	Super::Activate();
}

void UEOSKickMemberAsync::PerformKick()
{
	UE_LOG(LogTemp, Warning, TEXT("EOSKit: ========================================"));
	UE_LOG(LogTemp, Warning, TEXT("EOSKit: Kicking Member from Lobby"));
	UE_LOG(LogTemp, Warning, TEXT("EOSKit: Lobby ID: %s"), *VarLobbyId);
	UE_LOG(LogTemp, Warning, TEXT("EOSKit: Local User ID: %s"), *VarLocalUserId);
	UE_LOG(LogTemp, Warning, TEXT("EOSKit: Target User ID: %s"), *VarTargetUserId);
	UE_LOG(LogTemp, Warning, TEXT("EOSKit: WARNING: Member will be immediately disconnected!"));
	UE_LOG(LogTemp, Warning, TEXT("EOSKit: ========================================"));
	
	// Get the EOSKit subsystem
	UGameInstance* GameInstance = GetWorld()->GetGameInstance();
	if (!GameInstance)
	{
		UE_LOG(LogTemp, Error, TEXT("EOSKit: Failed to get Game Instance"));
		OnComplete.Broadcast(false, TEXT(""));
		SetReadyToDestroy();
#if ENGINE_MAJOR_VERSION == 5
		MarkAsGarbage();
#else
		MarkPendingKill();
#endif
		return;
	}
	
	UEOSKitSubsystem* EOSKitSubsystem = GameInstance->GetSubsystem<UEOSKitSubsystem>();
	if (!EOSKitSubsystem || !EOSKitSubsystem->GetPlatformHandle())
	{
		UE_LOG(LogTemp, Error, TEXT("EOSKit: EOSKit Subsystem or Platform Handle is null"));
		OnComplete.Broadcast(false, TEXT(""));
		SetReadyToDestroy();
#if ENGINE_MAJOR_VERSION == 5
		MarkAsGarbage();
#else
		MarkPendingKill();
#endif
		return;
	}
	
	EOS_HPlatform PlatformHandle = EOSKitSubsystem->GetPlatformHandle();
	EOS_HLobby LobbyHandle = EOS_Platform_GetLobbyInterface(PlatformHandle);
	
	if (!LobbyHandle)
	{
		UE_LOG(LogTemp, Error, TEXT("EOSKit: Failed to get Lobby Handle"));
		OnComplete.Broadcast(false, TEXT(""));
		SetReadyToDestroy();
#if ENGINE_MAJOR_VERSION == 5
		MarkAsGarbage();
#else
		MarkPendingKill();
#endif
		return;
	}
	
	// Validate Local Product User ID (must be lobby owner)
	EOS_ProductUserId LocalUserId = EOS_ProductUserId_FromString(TCHAR_TO_UTF8(*VarLocalUserId));
	if (!LocalUserId || !EOS_ProductUserId_IsValid(LocalUserId))
	{
		UE_LOG(LogTemp, Error, TEXT("EOSKit: Invalid Local Product User ID format"));
		OnComplete.Broadcast(false, TEXT(""));
		SetReadyToDestroy();
#if ENGINE_MAJOR_VERSION == 5
		MarkAsGarbage();
#else
		MarkPendingKill();
#endif
		return;
	}
	
	// Validate Target Product User ID (member to kick)
	EOS_ProductUserId TargetUserId = EOS_ProductUserId_FromString(TCHAR_TO_UTF8(*VarTargetUserId));
	if (!TargetUserId || !EOS_ProductUserId_IsValid(TargetUserId))
	{
		UE_LOG(LogTemp, Error, TEXT("EOSKit: Invalid Target Product User ID format"));
		OnComplete.Broadcast(false, TEXT(""));
		SetReadyToDestroy();
#if ENGINE_MAJOR_VERSION == 5
		MarkAsGarbage();
#else
		MarkPendingKill();
#endif
		return;
	}
	
	// Validate Lobby ID
	if (VarLobbyId.IsEmpty())
	{
		UE_LOG(LogTemp, Error, TEXT("EOSKit: Lobby ID is empty"));
		OnComplete.Broadcast(false, TEXT(""));
		SetReadyToDestroy();
#if ENGINE_MAJOR_VERSION == 5
		MarkAsGarbage();
#else
		MarkPendingKill();
#endif
		return;
	}
	
	// Setup Kick Member Options
	EOS_Lobby_KickMemberOptions KickOptions = {};
	KickOptions.ApiVersion = EOS_LOBBY_KICKMEMBER_API_LATEST;
	
	FTCHARToUTF8 LobbyIdConverter(*VarLobbyId);
	KickOptions.LobbyId = LobbyIdConverter.Get();
	KickOptions.LocalUserId = LocalUserId;
	KickOptions.TargetUserId = TargetUserId;
	
	// Create callback context
	struct FKickMemberContext
	{
		UEOSKickMemberAsync* AsyncNode;
		FString LobbyId;
		FString LocalUserId;
		FString TargetUserId;
	};
	
	FKickMemberContext* CallbackContext = new FKickMemberContext();
	CallbackContext->AsyncNode = this;
	CallbackContext->LobbyId = VarLobbyId;
	CallbackContext->LocalUserId = VarLocalUserId;
	CallbackContext->TargetUserId = VarTargetUserId;
	
	// Call EOS SDK
	EOS_Lobby_KickMember(LobbyHandle, &KickOptions, CallbackContext,
		[](const EOS_Lobby_KickMemberCallbackInfo* Data)
		{
			FKickMemberContext* Context = static_cast<FKickMemberContext*>(Data->ClientData);
			
			if (!Context || !Context->AsyncNode)
			{
				delete Context;
				return;
			}
			
			// Execute on game thread
			AsyncTask(ENamedThreads::GameThread, [Context, Data]()
			{
				if (Data->ResultCode == EOS_EResult::EOS_Success)
				{
					FString LobbyId = UTF8_TO_TCHAR(Data->LobbyId);
					
					UE_LOG(LogTemp, Warning, TEXT("EOSKit: ? Member kicked successfully!"));
					UE_LOG(LogTemp, Warning, TEXT("EOSKit: Lobby ID: %s"), *LobbyId);
					UE_LOG(LogTemp, Warning, TEXT("EOSKit: Kicked User: %s"), *Context->TargetUserId);
					UE_LOG(LogTemp, Warning, TEXT("EOSKit: Member has been disconnected from the lobby"));
					UE_LOG(LogTemp, Warning, TEXT("EOSKit: All remaining members have been notified"));
					
					if (Context->AsyncNode)
					{
						Context->AsyncNode->OnComplete.Broadcast(true, LobbyId);
						Context->AsyncNode->SetReadyToDestroy();
#if ENGINE_MAJOR_VERSION == 5
						Context->AsyncNode->MarkAsGarbage();
#else
						Context->AsyncNode->MarkPendingKill();
#endif
					}
				}
				else
				{
					UE_LOG(LogTemp, Error, TEXT("EOSKit: ? Failed to kick member: %s"), 
						*FString(EOS_EResult_ToString(Data->ResultCode)));
					
					// Common error explanations
					if (Data->ResultCode == EOS_EResult::EOS_InvalidParameters)
					{
						UE_LOG(LogTemp, Error, TEXT("EOSKit: Invalid parameters - check user IDs and lobby ID"));
					}
					else if (Data->ResultCode == EOS_EResult::EOS_InvalidUser)
					{
						UE_LOG(LogTemp, Error, TEXT("EOSKit: Invalid user - Product User ID not valid"));
					}
					else if (Data->ResultCode == EOS_EResult::EOS_NotFound)
					{
						UE_LOG(LogTemp, Error, TEXT("EOSKit: Not found - lobby or target user doesn't exist"));
					}
					else if (Data->ResultCode == EOS_EResult::EOS_Lobby_NotOwner)
					{
						UE_LOG(LogTemp, Error, TEXT("EOSKit: Not owner - only the lobby owner can kick members"));
						UE_LOG(LogTemp, Error, TEXT("EOSKit: Local User: %s is not the owner"), *Context->LocalUserId);
					}
					else if (Data->ResultCode == EOS_EResult::EOS_InvalidState)
					{
						UE_LOG(LogTemp, Error, TEXT("EOSKit: Invalid state - lobby may be in transition or target user not in lobby"));
						UE_LOG(LogTemp, Error, TEXT("EOSKit: Target User: %s"), *Context->TargetUserId);
					}
					else if (Data->ResultCode == EOS_EResult::EOS_UnexpectedError)
					{
						UE_LOG(LogTemp, Error, TEXT("EOSKit: Unexpected error - cannot kick lobby owner"));
					}
					
					if (Context->AsyncNode)
					{
						Context->AsyncNode->OnComplete.Broadcast(false, TEXT(""));
						Context->AsyncNode->SetReadyToDestroy();
#if ENGINE_MAJOR_VERSION == 5
						Context->AsyncNode->MarkAsGarbage();
#else
						Context->AsyncNode->MarkPendingKill();
#endif
					}
				}
				
				delete Context;
			});
		});
}
