// Copyright (C) 2024, All Rights Reserved.

#include "EOSRejectLobbyInviteAsync.h"
#include "EOSKitSubsystem.h"
#include "Kismet/GameplayStatics.h"
#include "eos_lobby.h"
#include "eos_lobby_types.h"
#include "Async/Async.h"

UEOSRejectLobbyInviteAsync* UEOSRejectLobbyInviteAsync::RejectLobbyInvite(FString LocalUserId, FString InviteId)
{
	UEOSRejectLobbyInviteAsync* Node = NewObject<UEOSRejectLobbyInviteAsync>();
	Node->VarLocalUserId = LocalUserId;
	Node->VarInviteId = InviteId;
	return Node;
}

void UEOSRejectLobbyInviteAsync::Activate()
{
	PerformReject();
	Super::Activate();
}

void UEOSRejectLobbyInviteAsync::PerformReject()
{
	UE_LOG(LogTemp, Warning, TEXT("EOSKit: ========================================"));
	UE_LOG(LogTemp, Warning, TEXT("EOSKit: Rejecting Lobby Invite"));
	UE_LOG(LogTemp, Warning, TEXT("EOSKit: Local User ID: %s"), *VarLocalUserId);
	UE_LOG(LogTemp, Warning, TEXT("EOSKit: Invite ID: %s"), *VarInviteId);
	UE_LOG(LogTemp, Warning, TEXT("EOSKit: ?? This action is permanent - user must be re-invited"));
	UE_LOG(LogTemp, Warning, TEXT("EOSKit: ========================================"));
	
	// Get the EOSKit subsystem
	UGameInstance* GameInstance = GetWorld()->GetGameInstance();
	if (!GameInstance)
	{
		UE_LOG(LogTemp, Error, TEXT("EOSKit: Failed to get Game Instance"));
		OnComplete.Broadcast(false);
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
		OnComplete.Broadcast(false);
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
		OnComplete.Broadcast(false);
		SetReadyToDestroy();
#if ENGINE_MAJOR_VERSION == 5
		MarkAsGarbage();
#else
		MarkPendingKill();
#endif
		return;
	}
	
	// Validate Product User ID
	EOS_ProductUserId LocalUserId = EOS_ProductUserId_FromString(TCHAR_TO_UTF8(*VarLocalUserId));
	if (!LocalUserId || !EOS_ProductUserId_IsValid(LocalUserId))
	{
		UE_LOG(LogTemp, Error, TEXT("EOSKit: Invalid Product User ID format"));
		OnComplete.Broadcast(false);
		SetReadyToDestroy();
#if ENGINE_MAJOR_VERSION == 5
		MarkAsGarbage();
#else
		MarkPendingKill();
#endif
		return;
	}
	
	// Validate Invite ID
	if (VarInviteId.IsEmpty())
	{
		UE_LOG(LogTemp, Error, TEXT("EOSKit: Invite ID is empty"));
		OnComplete.Broadcast(false);
		SetReadyToDestroy();
#if ENGINE_MAJOR_VERSION == 5
		MarkAsGarbage();
#else
		MarkPendingKill();
#endif
		return;
	}
	
	// Setup Reject Invite Options
	EOS_Lobby_RejectInviteOptions RejectOptions = {};
	RejectOptions.ApiVersion = EOS_LOBBY_REJECTINVITE_API_LATEST;
	RejectOptions.LocalUserId = LocalUserId;
	
	FTCHARToUTF8 InviteIdConverter(*VarInviteId);
	RejectOptions.InviteId = InviteIdConverter.Get();
	
	// Create callback context
	struct FRejectInviteContext
	{
		UEOSRejectLobbyInviteAsync* AsyncNode;
		FString UserId;
		FString InviteId;
	};
	
	FRejectInviteContext* CallbackContext = new FRejectInviteContext();
	CallbackContext->AsyncNode = this;
	CallbackContext->UserId = VarLocalUserId;
	CallbackContext->InviteId = VarInviteId;
	
	// Call EOS SDK
	EOS_Lobby_RejectInvite(LobbyHandle, &RejectOptions, CallbackContext,
		[](const EOS_Lobby_RejectInviteCallbackInfo* Data)
		{
			FRejectInviteContext* Context = static_cast<FRejectInviteContext*>(Data->ClientData);
			
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
					UE_LOG(LogTemp, Warning, TEXT("EOSKit: ? Successfully rejected lobby invite!"));
					UE_LOG(LogTemp, Warning, TEXT("EOSKit: User: %s"), *Context->UserId);
					UE_LOG(LogTemp, Warning, TEXT("EOSKit: Invite ID: %s"), *Context->InviteId);
					UE_LOG(LogTemp, Warning, TEXT("EOSKit: ========================================"));
					UE_LOG(LogTemp, Warning, TEXT("EOSKit: ? Invite removed from local cache"));
					UE_LOG(LogTemp, Warning, TEXT("EOSKit: ? Inviter notified (backend)"));
					UE_LOG(LogTemp, Warning, TEXT("EOSKit: ?? User must be re-invited to join this lobby"));
					UE_LOG(LogTemp, Warning, TEXT("EOSKit: ========================================"));
					
					if (Context->AsyncNode)
					{
						Context->AsyncNode->OnComplete.Broadcast(true);
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
					UE_LOG(LogTemp, Error, TEXT("EOSKit: ? Failed to reject lobby invite: %s"), 
						*FString(EOS_EResult_ToString(Data->ResultCode)));
					
					// Common error explanations
					if (Data->ResultCode == EOS_EResult::EOS_InvalidParameters)
					{
						UE_LOG(LogTemp, Error, TEXT("EOSKit: Invalid parameters - check invite ID format"));
					}
					else if (Data->ResultCode == EOS_EResult::EOS_InvalidUser)
					{
						UE_LOG(LogTemp, Error, TEXT("EOSKit: Invalid user - Product User ID not valid"));
					}
					else if (Data->ResultCode == EOS_EResult::EOS_NotFound)
					{
						UE_LOG(LogTemp, Error, TEXT("EOSKit: Invite not found - may have expired or been withdrawn"));
						UE_LOG(LogTemp, Error, TEXT("EOSKit: Invite ID: %s"), *Context->InviteId);
					}
					else if (Data->ResultCode == EOS_EResult::EOS_InvalidState)
					{
						UE_LOG(LogTemp, Error, TEXT("EOSKit: Invalid state - invite may have already been accepted or rejected"));
					}
					else if (Data->ResultCode == EOS_EResult::EOS_TooManyRequests)
					{
						UE_LOG(LogTemp, Error, TEXT("EOSKit: Too many requests - rate limited"));
						UE_LOG(LogTemp, Error, TEXT("EOSKit: Wait before trying again"));
					}
					
					if (Context->AsyncNode)
					{
						Context->AsyncNode->OnComplete.Broadcast(false);
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
