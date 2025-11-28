// Copyright (C) 2024, All Rights Reserved.

#include "EOSUpdateLobbyAsync.h"
#include "EOSKitSubsystem.h"
#include "Kismet/GameplayStatics.h"
#include "eos_lobby.h"
#include "eos_lobby_types.h"
#include "Async/Async.h"

UEOSUpdateLobbyAsync* UEOSUpdateLobbyAsync::UpdateLobby(FEOSKitLobbyModificationHandle LobbyModificationHandle)
{
	UEOSUpdateLobbyAsync* Node = NewObject<UEOSUpdateLobbyAsync>();
	Node->VarModificationHandle = LobbyModificationHandle;
	return Node;
}

void UEOSUpdateLobbyAsync::Activate()
{
	PerformUpdate();
	Super::Activate();
}

void UEOSUpdateLobbyAsync::PerformUpdate()
{
	UE_LOG(LogTemp, Warning, TEXT("EOSKit: ========================================"));
	UE_LOG(LogTemp, Warning, TEXT("EOSKit: Updating Lobby"));
	UE_LOG(LogTemp, Warning, TEXT("EOSKit: Applying modifications from handle..."));
	UE_LOG(LogTemp, Warning, TEXT("EOSKit: ?? Only lobby owner can update settings"));
	UE_LOG(LogTemp, Warning, TEXT("EOSKit: ========================================"));
	
	// Validate modification handle
	if (!VarModificationHandle.IsValid())
	{
		UE_LOG(LogTemp, Error, TEXT("EOSKit: Invalid lobby modification handle"));
		UE_LOG(LogTemp, Error, TEXT("EOSKit: Must create handle first via UpdateLobbyModification"));
		OnComplete.Broadcast(false, TEXT(""));
		SetReadyToDestroy();
#if ENGINE_MAJOR_VERSION == 5
		MarkAsGarbage();
#else
		MarkPendingKill();
#endif
		return;
	}
	
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
	
	// Setup Update Lobby Options
	EOS_Lobby_UpdateLobbyOptions UpdateOptions = {};
	UpdateOptions.ApiVersion = EOS_LOBBY_UPDATELOBBY_API_LATEST;
	UpdateOptions.LobbyModificationHandle = static_cast<EOS_HLobbyModification>(VarModificationHandle.Handle);
	
	// Create callback context
	struct FUpdateLobbyContext
	{
		UEOSUpdateLobbyAsync* AsyncNode;
		void* ModificationHandle;
	};
	
	FUpdateLobbyContext* CallbackContext = new FUpdateLobbyContext();
	CallbackContext->AsyncNode = this;
	CallbackContext->ModificationHandle = VarModificationHandle.Handle;
	
	// Call EOS SDK
	EOS_Lobby_UpdateLobby(LobbyHandle, &UpdateOptions, CallbackContext,
		[](const EOS_Lobby_UpdateLobbyCallbackInfo* Data)
		{
			FUpdateLobbyContext* Context = static_cast<FUpdateLobbyContext*>(Data->ClientData);
			
			if (!Context || !Context->AsyncNode)
			{
				// Cleanup modification handle
				if (Context && Context->ModificationHandle)
				{
					EOS_LobbyModification_Release(static_cast<EOS_HLobbyModification>(Context->ModificationHandle));
				}
				delete Context;
				return;
			}
			
			// Execute on game thread
			AsyncTask(ENamedThreads::GameThread, [Context, Data]()
			{
				if (Data->ResultCode == EOS_EResult::EOS_Success)
				{
					FString LobbyId = UTF8_TO_TCHAR(Data->LobbyId);
					
					UE_LOG(LogTemp, Warning, TEXT("EOSKit: ? Lobby updated successfully!"));
					UE_LOG(LogTemp, Warning, TEXT("EOSKit: Lobby ID: %s"), *LobbyId);
					UE_LOG(LogTemp, Warning, TEXT("EOSKit: ========================================"));
					UE_LOG(LogTemp, Warning, TEXT("EOSKit: ? Changes applied to lobby"));
					UE_LOG(LogTemp, Warning, TEXT("EOSKit: ? All members notified of updates"));
					UE_LOG(LogTemp, Warning, TEXT("EOSKit: ? Modification handle released"));
					UE_LOG(LogTemp, Warning, TEXT("EOSKit: ========================================"));
					
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
					UE_LOG(LogTemp, Error, TEXT("EOSKit: ? Failed to update lobby: %s"), 
						*FString(EOS_EResult_ToString(Data->ResultCode)));
					
					// Common error explanations
					if (Data->ResultCode == EOS_EResult::EOS_InvalidParameters)
					{
						UE_LOG(LogTemp, Error, TEXT("EOSKit: Invalid parameters - check modification handle"));
					}
					else if (Data->ResultCode == EOS_EResult::EOS_InvalidUser)
					{
						UE_LOG(LogTemp, Error, TEXT("EOSKit: Invalid user - Product User ID not valid"));
					}
					else if (Data->ResultCode == EOS_EResult::EOS_NotFound)
					{
						UE_LOG(LogTemp, Error, TEXT("EOSKit: Lobby not found - may have been destroyed"));
					}
					else if (Data->ResultCode == EOS_EResult::EOS_Lobby_NotOwner)
					{
						UE_LOG(LogTemp, Error, TEXT("EOSKit: Not owner - only the lobby owner can update settings"));
						UE_LOG(LogTemp, Error, TEXT("EOSKit: The user who created the modification handle must be the owner"));
					}
					else if (Data->ResultCode == EOS_EResult::EOS_InvalidState)
					{
						UE_LOG(LogTemp, Error, TEXT("EOSKit: Invalid state - lobby may be locked or in transition"));
					}
					else if (Data->ResultCode == EOS_EResult::EOS_TooManyRequests)
					{
						UE_LOG(LogTemp, Error, TEXT("EOSKit: Too many requests - rate limited"));
						UE_LOG(LogTemp, Error, TEXT("EOSKit: Wait before updating again"));
					}
					else if (Data->ResultCode == EOS_EResult::EOS_LimitExceeded)
					{
						UE_LOG(LogTemp, Error, TEXT("EOSKit: Limit exceeded - too many attributes or members"));
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
				
				// Cleanup modification handle
				if (Context->ModificationHandle)
				{
					EOS_LobbyModification_Release(static_cast<EOS_HLobbyModification>(Context->ModificationHandle));
					UE_LOG(LogTemp, Log, TEXT("EOSKit: Lobby modification handle released"));
				}
				
				delete Context;
			});
		});
}
