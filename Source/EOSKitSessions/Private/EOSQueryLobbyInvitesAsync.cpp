// Copyright (C) 2024, All Rights Reserved.

#include "EOSQueryLobbyInvitesAsync.h"
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

UEOSQueryLobbyInvitesAsync* UEOSQueryLobbyInvitesAsync::QueryLobbyInvites(FString LocalUserId)
{
	UEOSQueryLobbyInvitesAsync* Node = NewObject<UEOSQueryLobbyInvitesAsync>();
	Node->VarLocalUserId = LocalUserId;
	return Node;
}

void UEOSQueryLobbyInvitesAsync::Activate()
{
	PerformQuery();
	Super::Activate();
}

void UEOSQueryLobbyInvitesAsync::PerformQuery()
{
	UE_LOG(LogTemp, Warning, TEXT("EOSKit: ========================================"));
	UE_LOG(LogTemp, Warning, TEXT("EOSKit: Querying Lobby Invites"));
	UE_LOG(LogTemp, Warning, TEXT("EOSKit: Local User ID: %s"), *VarLocalUserId);
	UE_LOG(LogTemp, Warning, TEXT("EOSKit: Refreshing invite list from backend..."));
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
	
	// Setup Query Invites Options
	EOS_Lobby_QueryInvitesOptions QueryOptions = {};
	QueryOptions.ApiVersion = EOS_LOBBY_QUERYINVITES_API_LATEST;
	QueryOptions.LocalUserId = LocalUserId;
	
	// Create callback context
	struct FQueryInvitesContext
	{
		UEOSQueryLobbyInvitesAsync* AsyncNode;
		FString UserId;
	};
	
	FQueryInvitesContext* CallbackContext = new FQueryInvitesContext();
	CallbackContext->AsyncNode = this;
	CallbackContext->UserId = VarLocalUserId;
	
	// Call EOS SDK
	EOS_Lobby_QueryInvites(LobbyHandle, &QueryOptions, CallbackContext,
		[](const EOS_Lobby_QueryInvitesCallbackInfo* Data)
		{
			FQueryInvitesContext* Context = static_cast<FQueryInvitesContext*>(Data->ClientData);
			
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
					UE_LOG(LogTemp, Warning, TEXT("EOSKit: ? Successfully queried lobby invites!"));
					UE_LOG(LogTemp, Warning, TEXT("EOSKit: User: %s"), *Context->UserId);
					UE_LOG(LogTemp, Warning, TEXT("EOSKit: Invite list updated from backend"));
					UE_LOG(LogTemp, Warning, TEXT("EOSKit: ========================================"));
					UE_LOG(LogTemp, Warning, TEXT("EOSKit: Next Steps:"));
					UE_LOG(LogTemp, Warning, TEXT("EOSKit: 1. Use GetInviteCount to get number of invites"));
					UE_LOG(LogTemp, Warning, TEXT("EOSKit: 2. Use GetInviteIdByIndex to iterate through invites"));
					UE_LOG(LogTemp, Warning, TEXT("EOSKit: 3. Use CopyLobbyDetailsHandleByInviteId to get lobby info"));
					UE_LOG(LogTemp, Warning, TEXT("EOSKit: 4. Use JoinLobby to accept an invite"));
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
					UE_LOG(LogTemp, Error, TEXT("EOSKit: ? Failed to query lobby invites: %s"), 
						*FString(EOS_EResult_ToString(Data->ResultCode)));
					
					// Common error explanations
					if (Data->ResultCode == EOS_EResult::EOS_InvalidParameters)
					{
						UE_LOG(LogTemp, Error, TEXT("EOSKit: Invalid parameters - check user ID"));
					}
					else if (Data->ResultCode == EOS_EResult::EOS_InvalidUser)
					{
						UE_LOG(LogTemp, Error, TEXT("EOSKit: Invalid user - Product User ID not valid"));
					}
					else if (Data->ResultCode == EOS_EResult::EOS_NotFound)
					{
						UE_LOG(LogTemp, Warning, TEXT("EOSKit: No invites found - user has no pending invites"));
						UE_LOG(LogTemp, Warning, TEXT("EOSKit: This is not necessarily an error"));
					}
					else if (Data->ResultCode == EOS_EResult::EOS_TooManyRequests)
					{
						UE_LOG(LogTemp, Error, TEXT("EOSKit: Too many requests - rate limited"));
						UE_LOG(LogTemp, Error, TEXT("EOSKit: Wait before querying again"));
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
