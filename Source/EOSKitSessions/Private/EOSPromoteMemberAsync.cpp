// Copyright (C) 2024, All Rights Reserved.

#include "EOSPromoteMemberAsync.h"
#include "EOSKitSubsystem.h"
#include "Kismet/GameplayStatics.h"
#include "eos_lobby.h"
#include "eos_lobby_types.h"
#include "Async/Async.h"

UEOSPromoteMemberAsync* UEOSPromoteMemberAsync::PromoteMember(FString LobbyId, FString LocalUserId, FString TargetUserId)
{
	UEOSPromoteMemberAsync* Node = NewObject<UEOSPromoteMemberAsync>();
	Node->VarLobbyId = LobbyId;
	Node->VarLocalUserId = LocalUserId;
	Node->VarTargetUserId = TargetUserId;
	return Node;
}

void UEOSPromoteMemberAsync::Activate()
{
	PerformPromote();
	Super::Activate();
}

void UEOSPromoteMemberAsync::PerformPromote()
{
	UE_LOG(LogTemp, Warning, TEXT("EOSKit: ========================================"));
	UE_LOG(LogTemp, Warning, TEXT("EOSKit: Promoting Lobby Member to Owner"));
	UE_LOG(LogTemp, Warning, TEXT("EOSKit: Lobby ID: %s"), *VarLobbyId);
	UE_LOG(LogTemp, Warning, TEXT("EOSKit: Current Owner: %s"), *VarLocalUserId);
	UE_LOG(LogTemp, Warning, TEXT("EOSKit: New Owner: %s"), *VarTargetUserId);
	UE_LOG(LogTemp, Warning, TEXT("EOSKit: WARNING: Ownership will be transferred!"));
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
	
	// Validate Local Product User ID (must be current owner)
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
	
	// Validate Target Product User ID (member to promote)
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
	
	// Check if trying to promote self
	if (VarLocalUserId == VarTargetUserId)
	{
		UE_LOG(LogTemp, Error, TEXT("EOSKit: Cannot promote self - already owner"));
		OnComplete.Broadcast(false, TEXT(""));
		SetReadyToDestroy();
#if ENGINE_MAJOR_VERSION == 5
		MarkAsGarbage();
#else
		MarkPendingKill();
#endif
		return;
	}
	
	// Setup Promote Member Options
	EOS_Lobby_PromoteMemberOptions PromoteOptions = {};
	PromoteOptions.ApiVersion = EOS_LOBBY_PROMOTEMEMBER_API_LATEST;
	
	FTCHARToUTF8 LobbyIdConverter(*VarLobbyId);
	PromoteOptions.LobbyId = LobbyIdConverter.Get();
	PromoteOptions.LocalUserId = LocalUserId;
	PromoteOptions.TargetUserId = TargetUserId;
	
	// Create callback context
	struct FPromoteMemberContext
	{
		UEOSPromoteMemberAsync* AsyncNode;
		FString LobbyId;
		FString OldOwnerId;
		FString NewOwnerId;
	};
	
	FPromoteMemberContext* CallbackContext = new FPromoteMemberContext();
	CallbackContext->AsyncNode = this;
	CallbackContext->LobbyId = VarLobbyId;
	CallbackContext->OldOwnerId = VarLocalUserId;
	CallbackContext->NewOwnerId = VarTargetUserId;
	
	// Call EOS SDK
	EOS_Lobby_PromoteMember(LobbyHandle, &PromoteOptions, CallbackContext,
		[](const EOS_Lobby_PromoteMemberCallbackInfo* Data)
		{
			FPromoteMemberContext* Context = static_cast<FPromoteMemberContext*>(Data->ClientData);
			
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
					
					UE_LOG(LogTemp, Warning, TEXT("EOSKit: ? Member promoted to owner successfully!"));
					UE_LOG(LogTemp, Warning, TEXT("EOSKit: Lobby ID: %s"), *LobbyId);
					UE_LOG(LogTemp, Warning, TEXT("EOSKit: ========================================"));
					UE_LOG(LogTemp, Warning, TEXT("EOSKit: ?? Ownership Transferred!"));
					UE_LOG(LogTemp, Warning, TEXT("EOSKit: Old Owner: %s ? Regular Member"), *Context->OldOwnerId);
					UE_LOG(LogTemp, Warning, TEXT("EOSKit: New Owner: %s ? Lobby Owner"), *Context->NewOwnerId);
					UE_LOG(LogTemp, Warning, TEXT("EOSKit: ========================================"));
					UE_LOG(LogTemp, Warning, TEXT("EOSKit: New owner has full lobby modification rights"));
					UE_LOG(LogTemp, Warning, TEXT("EOSKit: Old owner can no longer modify lobby settings"));
					UE_LOG(LogTemp, Warning, TEXT("EOSKit: All members have been notified of the change"));
					
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
					UE_LOG(LogTemp, Error, TEXT("EOSKit: ? Failed to promote member: %s"), 
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
						UE_LOG(LogTemp, Error, TEXT("EOSKit: Not owner - only the current owner can promote members"));
						UE_LOG(LogTemp, Error, TEXT("EOSKit: Local User: %s is not the owner"), *Context->OldOwnerId);
					}
					else if (Data->ResultCode == EOS_EResult::EOS_InvalidState)
					{
						UE_LOG(LogTemp, Error, TEXT("EOSKit: Invalid state - target user may not be in lobby"));
						UE_LOG(LogTemp, Error, TEXT("EOSKit: Target User: %s must be a member of the lobby"), *Context->NewOwnerId);
					}
					else if (Data->ResultCode == EOS_EResult::EOS_UnexpectedError)
					{
						UE_LOG(LogTemp, Error, TEXT("EOSKit: Unexpected error - operation failed"));
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
