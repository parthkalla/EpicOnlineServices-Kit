// Copyright (C) 2024, All Rights Reserved.

#include "EOSDestroyLobbyAsync.h"
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

UEOSDestroyLobbyAsync* UEOSDestroyLobbyAsync::DestroyLobby(FString LocalUserId, FString LobbyId)
{
	UEOSDestroyLobbyAsync* Node = NewObject<UEOSDestroyLobbyAsync>();
	Node->VarLocalUserId = LocalUserId;
	Node->VarLobbyId = LobbyId;
	return Node;
}

void UEOSDestroyLobbyAsync::Activate()
{
	PerformDestroy();
	Super::Activate();
}

void UEOSDestroyLobbyAsync::PerformDestroy()
{
	UE_LOG(LogTemp, Warning, TEXT("EOSKit: ========================================"));
	UE_LOG(LogTemp, Warning, TEXT("EOSKit: Destroying Lobby"));
	UE_LOG(LogTemp, Warning, TEXT("EOSKit: Local User ID: %s"), *VarLocalUserId);
	UE_LOG(LogTemp, Warning, TEXT("EOSKit: Lobby ID: %s"), *VarLobbyId);
	UE_LOG(LogTemp, Warning, TEXT("EOSKit: WARNING: All members will be disconnected!"));
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
	
	// Validate Product User ID
	EOS_ProductUserId LocalUserId = EOS_ProductUserId_FromString(TCHAR_TO_UTF8(*VarLocalUserId));
	if (!LocalUserId || !EOS_ProductUserId_IsValid(LocalUserId))
	{
		UE_LOG(LogTemp, Error, TEXT("EOSKit: Invalid Product User ID format"));
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
	
	// Setup Destroy Lobby Options
	EOS_Lobby_DestroyLobbyOptions DestroyOptions = {};
	DestroyOptions.ApiVersion = EOS_LOBBY_DESTROYLOBBY_API_LATEST;
	DestroyOptions.LocalUserId = LocalUserId;
	
	FTCHARToUTF8 LobbyIdConverter(*VarLobbyId);
	DestroyOptions.LobbyId = LobbyIdConverter.Get();
	
	// Create callback context
	struct FDestroyLobbyContext
	{
		UEOSDestroyLobbyAsync* AsyncNode;
		FString LobbyId;
		FString UserId;
	};
	
	FDestroyLobbyContext* CallbackContext = new FDestroyLobbyContext();
	CallbackContext->AsyncNode = this;
	CallbackContext->LobbyId = VarLobbyId;
	CallbackContext->UserId = VarLocalUserId;
	
	// Call EOS SDK
	EOS_Lobby_DestroyLobby(LobbyHandle, &DestroyOptions, CallbackContext,
		[](const EOS_Lobby_DestroyLobbyCallbackInfo* Data)
		{
			FDestroyLobbyContext* Context = static_cast<FDestroyLobbyContext*>(Data->ClientData);
			
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
					
					UE_LOG(LogTemp, Warning, TEXT("EOSKit: ? Lobby destroyed successfully!"));
					UE_LOG(LogTemp, Warning, TEXT("EOSKit: Lobby ID: %s"), *LobbyId);
					UE_LOG(LogTemp, Warning, TEXT("EOSKit: All members have been disconnected"));
					UE_LOG(LogTemp, Warning, TEXT("EOSKit: Lobby data has been permanently deleted"));
					
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
					UE_LOG(LogTemp, Error, TEXT("EOSKit: ? Failed to destroy lobby: %s"), 
						*FString(EOS_EResult_ToString(Data->ResultCode)));
					
					// Common error explanations
					if (Data->ResultCode == EOS_EResult::EOS_InvalidParameters)
					{
						UE_LOG(LogTemp, Error, TEXT("EOSKit: Invalid parameters - check Lobby ID format"));
					}
					else if (Data->ResultCode == EOS_EResult::EOS_InvalidUser)
					{
						UE_LOG(LogTemp, Error, TEXT("EOSKit: Invalid user - Product User ID not valid"));
					}
					else if (Data->ResultCode == EOS_EResult::EOS_NotFound)
					{
						UE_LOG(LogTemp, Error, TEXT("EOSKit: Lobby not found - may have already been destroyed"));
					}
					else if (Data->ResultCode == EOS_EResult::EOS_Lobby_NotOwner)
					{
						UE_LOG(LogTemp, Error, TEXT("EOSKit: Not owner - only the lobby owner can destroy the lobby"));
					}
					else if (Data->ResultCode == EOS_EResult::EOS_InvalidState)
					{
						UE_LOG(LogTemp, Error, TEXT("EOSKit: Invalid state - lobby may not be in destroyable state"));
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
