// Copyright (C) 2024, All Rights Reserved.

#include "EOSLobbySearchFindAsync.h"
#include "EOSKitSubsystem.h"
#include "Kismet/GameplayStatics.h"
#include "eos_lobby.h"
#include "eos_lobby_types.h"
#include "Async/Async.h"

UEOSLobbySearchFindAsync* UEOSLobbySearchFindAsync::LobbySearchFind(FEOSKitLobbySearchHandle SearchHandle, FString LocalUserId)
{
	UEOSLobbySearchFindAsync* Node = NewObject<UEOSLobbySearchFindAsync>();
	Node->VarSearchHandle = SearchHandle;
	Node->VarLocalUserId = LocalUserId;
	return Node;
}

void UEOSLobbySearchFindAsync::Activate()
{
	PerformSearch();
	Super::Activate();
}

void UEOSLobbySearchFindAsync::PerformSearch()
{
	UE_LOG(LogTemp, Warning, TEXT("EOSKit: ========================================"));
	UE_LOG(LogTemp, Warning, TEXT("EOSKit: Executing Lobby Search"));
	UE_LOG(LogTemp, Warning, TEXT("EOSKit: Local User ID: %s"), *VarLocalUserId);
	UE_LOG(LogTemp, Warning, TEXT("EOSKit: Searching for matching lobbies..."));
	UE_LOG(LogTemp, Warning, TEXT("EOSKit: ========================================"));
	
	// Validate search handle
	if (!VarSearchHandle.IsValid())
	{
		UE_LOG(LogTemp, Error, TEXT("EOSKit: Invalid lobby search handle"));
		UE_LOG(LogTemp, Error, TEXT("EOSKit: Must create handle first via CreateLobbySearch"));
		OnComplete.Broadcast(false);
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
	
	// Setup Find Options
	EOS_LobbySearch_FindOptions FindOptions = {};
	FindOptions.ApiVersion = EOS_LOBBYSEARCH_FIND_API_LATEST;
	FindOptions.LocalUserId = LocalUserId;
	
	// Create callback context
	struct FLobbySearchContext
	{
		UEOSLobbySearchFindAsync* AsyncNode;
		FString UserId;
		void* SearchHandle;
	};
	
	FLobbySearchContext* CallbackContext = new FLobbySearchContext();
	CallbackContext->AsyncNode = this;
	CallbackContext->UserId = VarLocalUserId;
	CallbackContext->SearchHandle = VarSearchHandle.Handle;
	
	// Call EOS SDK
	EOS_LobbySearch_Find(static_cast<EOS_HLobbySearch>(VarSearchHandle.Handle), &FindOptions, CallbackContext,
		[](const EOS_LobbySearch_FindCallbackInfo* Data)
		{
			FLobbySearchContext* Context = static_cast<FLobbySearchContext*>(Data->ClientData);
			
			if (!Context || !Context->AsyncNode)
			{
				// Cleanup search handle
				if (Context && Context->SearchHandle)
				{
					EOS_LobbySearch_Release(static_cast<EOS_HLobbySearch>(Context->SearchHandle));
				}
				delete Context;
				return;
			}
			
			// Execute on game thread
			AsyncTask(ENamedThreads::GameThread, [Context, Data]()
			{
				if (Data->ResultCode == EOS_EResult::EOS_Success)
				{
					UE_LOG(LogTemp, Warning, TEXT("EOSKit: ? Lobby search completed successfully!"));
					UE_LOG(LogTemp, Warning, TEXT("EOSKit: User: %s"), *Context->UserId);
					UE_LOG(LogTemp, Warning, TEXT("EOSKit: ========================================"));
					UE_LOG(LogTemp, Warning, TEXT("EOSKit: Next Steps:"));
					UE_LOG(LogTemp, Warning, TEXT("EOSKit: 1. Use GetSearchResultCount to get number of lobbies found"));
					UE_LOG(LogTemp, Warning, TEXT("EOSKit: 2. Use CopySearchResultByIndex to iterate through results"));
					UE_LOG(LogTemp, Warning, TEXT("EOSKit: 3. Use CopyLobbyDetailsHandle to get lobby info"));
					UE_LOG(LogTemp, Warning, TEXT("EOSKit: 4. Use JoinLobby to join a lobby from results"));
					UE_LOG(LogTemp, Warning, TEXT("EOSKit: 5. Release handle when done: EOS_LobbySearch_Release"));
					UE_LOG(LogTemp, Warning, TEXT("EOSKit: ========================================"));
					UE_LOG(LogTemp, Warning, TEXT("EOSKit: ?? Note: Handle released automatically after this call"));
					
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
					UE_LOG(LogTemp, Error, TEXT("EOSKit: ? Lobby search failed: %s"), 
						*FString(EOS_EResult_ToString(Data->ResultCode)));
					
					// Common error explanations
					if (Data->ResultCode == EOS_EResult::EOS_InvalidParameters)
					{
						UE_LOG(LogTemp, Error, TEXT("EOSKit: Invalid parameters - check search handle configuration"));
					}
					else if (Data->ResultCode == EOS_EResult::EOS_InvalidUser)
					{
						UE_LOG(LogTemp, Error, TEXT("EOSKit: Invalid user - Product User ID not valid"));
					}
					else if (Data->ResultCode == EOS_EResult::EOS_NotFound)
					{
						UE_LOG(LogTemp, Warning, TEXT("EOSKit: No lobbies found matching search criteria"));
						UE_LOG(LogTemp, Warning, TEXT("EOSKit: This is not an error - no results available"));
						UE_LOG(LogTemp, Warning, TEXT("EOSKit: Try adjusting search parameters"));
					}
					else if (Data->ResultCode == EOS_EResult::EOS_TooManyRequests)
					{
						UE_LOG(LogTemp, Error, TEXT("EOSKit: Too many requests - rate limited"));
						UE_LOG(LogTemp, Error, TEXT("EOSKit: Wait before searching again"));
					}
					else if (Data->ResultCode == EOS_EResult::EOS_InvalidState)
					{
						UE_LOG(LogTemp, Error, TEXT("EOSKit: Invalid state - search handle not properly configured"));
						UE_LOG(LogTemp, Error, TEXT("EOSKit: Set search parameters before calling Find"));
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
				
				// Cleanup search handle
				if (Context->SearchHandle)
				{
					EOS_LobbySearch_Release(static_cast<EOS_HLobbySearch>(Context->SearchHandle));
					UE_LOG(LogTemp, Log, TEXT("EOSKit: Lobby search handle released"));
				}
				
				delete Context;
			});
		});
}
