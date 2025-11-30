// Copyright (C) 2024, All Rights Reserved.

#include "EOSSendLobbyInviteAsync.h"
#include "EOSKitSubsystem.h"
#include "Kismet/GameplayStatics.h"
#if WITH_EOS_SDK
#include "Windows/AllowWindowsPlatformTypes.h"
#include "Windows/PreWindowsApi.h"
#include "eos_platform.h"
#include "eos_lobby.h"
#include "eos_lobby_types.h"
#include "Windows/PostWindowsApi.h"
#include "Windows/HideWindowsPlatformTypes.h"
#endif
#include "Async/Async.h"

UEOSSendLobbyInviteAsync* UEOSSendLobbyInviteAsync::SendLobbyInvite(FString LocalUserId, FString LobbyId, FString TargetUserId)
{
	UEOSSendLobbyInviteAsync* Node = NewObject<UEOSSendLobbyInviteAsync>();
	Node->VarLocalUserId = LocalUserId;
	Node->VarLobbyId = LobbyId;
	Node->VarTargetUserId = TargetUserId;
	return Node;
}

void UEOSSendLobbyInviteAsync::Activate()
{
	PerformSendInvite();
	Super::Activate();
}

void UEOSSendLobbyInviteAsync::PerformSendInvite()
{
	UE_LOG(LogTemp, Warning, TEXT("EOSKit: ========================================"));
	UE_LOG(LogTemp, Warning, TEXT("EOSKit: Sending Lobby Invite"));
	UE_LOG(LogTemp, Warning, TEXT("EOSKit: Lobby ID: %s"), *VarLobbyId);
	UE_LOG(LogTemp, Warning, TEXT("EOSKit: Inviter: %s"), *VarLocalUserId);
	UE_LOG(LogTemp, Warning, TEXT("EOSKit: Target: %s"), *VarTargetUserId);
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
	
	// Validate Local Product User ID (must be lobby member)
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
	
	// Validate Target Product User ID (user to invite)
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
	
	// Check if trying to invite self
	if (VarLocalUserId == VarTargetUserId)
	{
		UE_LOG(LogTemp, Error, TEXT("EOSKit: Cannot invite yourself"));
		OnComplete.Broadcast(false, TEXT(""));
		SetReadyToDestroy();
#if ENGINE_MAJOR_VERSION == 5
		MarkAsGarbage();
#else
		MarkPendingKill();
#endif
		return;
	}
	
	// Setup Send Invite Options
	EOS_Lobby_SendInviteOptions SendOptions = {};
	SendOptions.ApiVersion = EOS_LOBBY_SENDINVITE_API_LATEST;
	SendOptions.LocalUserId = LocalUserId;
	
	FTCHARToUTF8 LobbyIdConverter(*VarLobbyId);
	SendOptions.LobbyId = LobbyIdConverter.Get();
	SendOptions.TargetUserId = TargetUserId;
	
	// Create callback context
	struct FSendInviteContext
	{
		UEOSSendLobbyInviteAsync* AsyncNode;
		FString LobbyId;
		FString InviterId;
		FString TargetId;
	};
	
	FSendInviteContext* CallbackContext = new FSendInviteContext();
	CallbackContext->AsyncNode = this;
	CallbackContext->LobbyId = VarLobbyId;
	CallbackContext->InviterId = VarLocalUserId;
	CallbackContext->TargetId = VarTargetUserId;
	
	// Call EOS SDK
	EOS_Lobby_SendInvite(LobbyHandle, &SendOptions, CallbackContext,
		[](const EOS_Lobby_SendInviteCallbackInfo* Data)
		{
			FSendInviteContext* Context = static_cast<FSendInviteContext*>(Data->ClientData);
			
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
					
					UE_LOG(LogTemp, Warning, TEXT("EOSKit: ? Lobby invite sent successfully!"));
					UE_LOG(LogTemp, Warning, TEXT("EOSKit: Lobby ID: %s"), *LobbyId);
					UE_LOG(LogTemp, Warning, TEXT("EOSKit: ========================================"));
					UE_LOG(LogTemp, Warning, TEXT("EOSKit: ?? Invite Details:"));
					UE_LOG(LogTemp, Warning, TEXT("EOSKit: From: %s"), *Context->InviterId);
					UE_LOG(LogTemp, Warning, TEXT("EOSKit: To: %s"), *Context->TargetId);
					UE_LOG(LogTemp, Warning, TEXT("EOSKit: ========================================"));
					UE_LOG(LogTemp, Warning, TEXT("EOSKit: ? Target user will receive notification"));
					UE_LOG(LogTemp, Warning, TEXT("EOSKit: ? Target can accept or reject the invite"));
					UE_LOG(LogTemp, Warning, TEXT("EOSKit: ? Invite added to target's invite list"));
					
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
					UE_LOG(LogTemp, Error, TEXT("EOSKit: ? Failed to send lobby invite: %s"), 
						*FString(EOS_EResult_ToString(Data->ResultCode)));
					
					// Common error explanations
					if (Data->ResultCode == EOS_EResult::EOS_InvalidParameters)
					{
						UE_LOG(LogTemp, Error, TEXT("EOSKit: Invalid parameters - check user IDs and lobby ID"));
					}
					else if (Data->ResultCode == EOS_EResult::EOS_InvalidUser)
					{
						UE_LOG(LogTemp, Error, TEXT("EOSKit: Invalid user - one of the Product User IDs is not valid"));
					}
					else if (Data->ResultCode == EOS_EResult::EOS_NotFound)
					{
						UE_LOG(LogTemp, Error, TEXT("EOSKit: Lobby not found or target user doesn't exist"));
					}
					else if (Data->ResultCode == EOS_EResult::EOS_Lobby_NotOwner)
					{
						UE_LOG(LogTemp, Error, TEXT("EOSKit: Not a lobby member - local user must be in the lobby"));
						UE_LOG(LogTemp, Error, TEXT("EOSKit: Inviter: %s"), *Context->InviterId);
					}
					else if (Data->ResultCode == EOS_EResult::EOS_Lobby_InviteFailed)
					{
						UE_LOG(LogTemp, Error, TEXT("EOSKit: Invite failed - target may already be in lobby or invited"));
						UE_LOG(LogTemp, Error, TEXT("EOSKit: Target: %s"), *Context->TargetId);
					}
					else if (Data->ResultCode == EOS_EResult::EOS_InvalidState)
					{
						UE_LOG(LogTemp, Error, TEXT("EOSKit: Invalid state - lobby may not allow invites"));
						UE_LOG(LogTemp, Error, TEXT("EOSKit: Check lobby's bAllowInvites setting"));
					}
					else if (Data->ResultCode == EOS_EResult::EOS_Lobby_NotAllowed)
					{
						UE_LOG(LogTemp, Error, TEXT("EOSKit: Not allowed - user may not have invite privileges"));
					}
					else if (Data->ResultCode == EOS_EResult::EOS_TooManyRequests)
					{
						UE_LOG(LogTemp, Error, TEXT("EOSKit: Too many requests - rate limited"));
						UE_LOG(LogTemp, Error, TEXT("EOSKit: Wait before sending more invites"));
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
