// Copyright (C) 2024, All Rights Reserved.

#include "EOSLeaveLobbyAsync.h"
#include "EOSKitSubsystem.h"
#include "Kismet/GameplayStatics.h"
#if WITH_EOS_SDK
#include "Windows/AllowWindowsPlatformTypes.h"
#include "eos_sdk.h"
#include "eos_lobby.h"
#include "eos_lobby_types.h"
#include "Windows/HideWindowsPlatformTypes.h"
#endif
#include "Async/Async.h"

UEOSLeaveLobbyAsync* UEOSLeaveLobbyAsync::LeaveLobby(FString LocalUserId, FString LobbyId)
{
	UEOSLeaveLobbyAsync* Node = NewObject<UEOSLeaveLobbyAsync>();
	Node->VarLocalUserId = LocalUserId;
	Node->VarLobbyId = LobbyId;
	return Node;
}

void UEOSLeaveLobbyAsync::Activate()
{
	PerformLeave();
	Super::Activate();
}

void UEOSLeaveLobbyAsync::PerformLeave()
{
	UE_LOG(LogTemp, Warning, TEXT("EOSKit: ========================================"));
	UE_LOG(LogTemp, Warning, TEXT("EOSKit: Leaving Lobby"));
	UE_LOG(LogTemp, Warning, TEXT("EOSKit: Lobby ID: %s"), *VarLobbyId);
	UE_LOG(LogTemp, Warning, TEXT("EOSKit: Local User ID: %s"), *VarLocalUserId);
	UE_LOG(LogTemp, Warning, TEXT("EOSKit: RTC Room will be automatically left if enabled"));
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
	
	// Setup Leave Lobby Options
	EOS_Lobby_LeaveLobbyOptions LeaveOptions = {};
	LeaveOptions.ApiVersion = EOS_LOBBY_LEAVELOBBY_API_LATEST;
	LeaveOptions.LocalUserId = LocalUserId;
	
	FTCHARToUTF8 LobbyIdConverter(*VarLobbyId);
	LeaveOptions.LobbyId = LobbyIdConverter.Get();
	
	// Create callback context
	struct FLeaveLobbyContext
	{
		UEOSLeaveLobbyAsync* AsyncNode;
		FString LobbyId;
		FString UserId;
	};
	
	FLeaveLobbyContext* CallbackContext = new FLeaveLobbyContext();
	CallbackContext->AsyncNode = this;
	CallbackContext->LobbyId = VarLobbyId;
	CallbackContext->UserId = VarLocalUserId;
	
	// Call EOS SDK
	EOS_Lobby_LeaveLobby(LobbyHandle, &LeaveOptions, CallbackContext,
		[](const EOS_Lobby_LeaveLobbyCallbackInfo* Data)
		{
			FLeaveLobbyContext* Context = static_cast<FLeaveLobbyContext*>(Data->ClientData);
			
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
					
					UE_LOG(LogTemp, Warning, TEXT("EOSKit: ? Successfully left lobby!"));
					UE_LOG(LogTemp, Warning, TEXT("EOSKit: Lobby ID: %s"), *LobbyId);
					UE_LOG(LogTemp, Warning, TEXT("EOSKit: User: %s"), *Context->UserId);
					UE_LOG(LogTemp, Warning, TEXT("EOSKit: RTC Room disconnected (if enabled)"));
					UE_LOG(LogTemp, Warning, TEXT("EOSKit: Presence updated"));
					
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
					UE_LOG(LogTemp, Error, TEXT("EOSKit: ? Failed to leave lobby: %s"), 
						*FString(EOS_EResult_ToString(Data->ResultCode)));
					
					// Common error explanations
					if (Data->ResultCode == EOS_EResult::EOS_InvalidParameters)
					{
						UE_LOG(LogTemp, Error, TEXT("EOSKit: Invalid parameters - check lobby ID and user ID"));
					}
					else if (Data->ResultCode == EOS_EResult::EOS_InvalidUser)
					{
						UE_LOG(LogTemp, Error, TEXT("EOSKit: Invalid user - Product User ID not valid"));
					}
					else if (Data->ResultCode == EOS_EResult::EOS_NotFound)
					{
						UE_LOG(LogTemp, Error, TEXT("EOSKit: Lobby not found - may have been destroyed or never joined"));
					}
					else if (Data->ResultCode == EOS_EResult::EOS_Lobby_NotOwner)
					{
						UE_LOG(LogTemp, Warning, TEXT("EOSKit: Not owner - but leaving is still allowed"));
						UE_LOG(LogTemp, Warning, TEXT("EOSKit: This error shouldn't prevent leaving"));
					}
					else if (Data->ResultCode == EOS_EResult::EOS_InvalidState)
					{
						UE_LOG(LogTemp, Error, TEXT("EOSKit: Invalid state - user may not be in the lobby"));
						UE_LOG(LogTemp, Error, TEXT("EOSKit: Check that user is actually a member of this lobby"));
					}
					else if (Data->ResultCode == EOS_EResult::EOS_AlreadyPending)
					{
						UE_LOG(LogTemp, Error, TEXT("EOSKit: Already pending - leave request already in progress"));
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
