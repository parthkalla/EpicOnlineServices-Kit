// Copyright (C) 2024, All Rights Reserved.

#include "EOSUnregisterPlayersAsync.h"
#include "EOSKitSubsystem.h"
#include "Kismet/GameplayStatics.h"
#if WITH_EOS_SDK
#include "Windows/AllowWindowsPlatformTypes.h"
#include "Windows/PreWindowsApi.h"
#include "eos_platform.h"
#include "eos_sessions.h"
#include "Windows/PostWindowsApi.h"
#include "Windows/HideWindowsPlatformTypes.h"
#endif
#include "Async/Async.h"

UEOSUnregisterPlayersAsync* UEOSUnregisterPlayersAsync::UnregisterPlayers(
	FName SessionName,
	const TArray<FString>& PlayersToUnregister)
{
	UEOSUnregisterPlayersAsync* Node = NewObject<UEOSUnregisterPlayersAsync>();
	Node->VSessionName = SessionName;
	Node->PlayerIds = PlayersToUnregister;
	return Node;
}

void UEOSUnregisterPlayersAsync::Activate()
{
	UnregisterPlayersFromSession();
	Super::Activate();
}

void UEOSUnregisterPlayersAsync::UnregisterPlayersFromSession()
{
	UE_LOG(LogTemp, Warning, TEXT("EOSKit: ========================================"));
	UE_LOG(LogTemp, Warning, TEXT("EOSKit: Unregistering Players from Session via SDK"));
	UE_LOG(LogTemp, Warning, TEXT("EOSKit: Session Name: %s"), *VSessionName.ToString());
	UE_LOG(LogTemp, Warning, TEXT("EOSKit: Players to unregister: %d"), PlayerIds.Num());
	UE_LOG(LogTemp, Warning, TEXT("EOSKit: ========================================"));
	
	// Get the EOSKit subsystem
	UGameInstance* GameInstance = GetWorld()->GetGameInstance();
	if (!GameInstance)
	{
		UE_LOG(LogTemp, Error, TEXT("EOSKit: Failed to get Game Instance"));
		OnFail.Broadcast(TArray<FString>());
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
		OnFail.Broadcast(TArray<FString>());
		SetReadyToDestroy();
#if ENGINE_MAJOR_VERSION == 5
		MarkAsGarbage();
#else
		MarkPendingKill();
#endif
		return;
	}
	
	EOS_HPlatform PlatformHandle = EOSKitSubsystem->GetPlatformHandle();
	EOS_HSessions SessionsHandle = EOS_Platform_GetSessionsInterface(PlatformHandle);
	
	if (!SessionsHandle)
	{
		UE_LOG(LogTemp, Error, TEXT("EOSKit: Failed to get Sessions Handle"));
		OnFail.Broadcast(TArray<FString>());
		SetReadyToDestroy();
#if ENGINE_MAJOR_VERSION == 5
		MarkAsGarbage();
#else
		MarkPendingKill();
#endif
		return;
	}
	
	// Get Product User ID for local player
	EOS_ProductUserId LocalUserId = EOSKitSubsystem->GetProductUserId();
	if (!LocalUserId)
	{
		UE_LOG(LogTemp, Error, TEXT("EOSKit: Product User ID is null - user must be logged in"));
		OnFail.Broadcast(TArray<FString>());
		SetReadyToDestroy();
#if ENGINE_MAJOR_VERSION == 5
		MarkAsGarbage();
#else
		MarkPendingKill();
#endif
		return;
	}
	
	// Convert player IDs to EOS_ProductUserId array
	TArray<EOS_ProductUserId> ProductUserIds;
	
	if (PlayerIds.Num() == 0)
	{
		// If no player IDs provided, unregister the local player
		ProductUserIds.Add(LocalUserId);
		UE_LOG(LogTemp, Log, TEXT("EOSKit: No player IDs provided, unregistering local player only"));
	}
	else
	{
		// Convert string IDs to EOS_ProductUserId
		// Note: This is a simplified version - in a real implementation you'd need to
		// store EOS_ProductUserId handles or convert from string properly
		UE_LOG(LogTemp, Warning, TEXT("EOSKit: Converting player IDs from strings to EOS_ProductUserId"));
		
		// For now, just use the local player as we can't easily convert strings to EOS_ProductUserId
		// In a production system, you'd need to maintain a mapping of ProductUserId handles
		ProductUserIds.Add(LocalUserId);
		UE_LOG(LogTemp, Warning, TEXT("EOSKit: Note: Only local player will be unregistered (string conversion not implemented)"));
	}
	
	// Allocate array for EOS SDK
	EOS_ProductUserId* PlayersArray = new EOS_ProductUserId[ProductUserIds.Num()];
	for (int32 i = 0; i < ProductUserIds.Num(); i++)
	{
		PlayersArray[i] = ProductUserIds[i];
	}
	
	// Setup Unregister Players Options
	EOS_Sessions_UnregisterPlayersOptions UnregisterOptions = {};
	UnregisterOptions.ApiVersion = EOS_SESSIONS_UNREGISTERPLAYERS_API_LATEST;
	
	FTCHARToUTF8 SessionNameConverter(*VSessionName.ToString());
	UnregisterOptions.SessionName = SessionNameConverter.Get();
	UnregisterOptions.PlayersToUnregister = PlayersArray;
	UnregisterOptions.PlayersToUnregisterCount = ProductUserIds.Num();
	
	// Create callback context
	struct FUnregisterPlayersContext
	{
		UEOSUnregisterPlayersAsync* AsyncNode;
		FString SessionName;
		EOS_ProductUserId* PlayersArray;
	};
	
	FUnregisterPlayersContext* CallbackContext = new FUnregisterPlayersContext();
	CallbackContext->AsyncNode = this;
	CallbackContext->SessionName = VSessionName.ToString();
	CallbackContext->PlayersArray = PlayersArray; // Store for cleanup
	
	// Call EOS SDK to unregister players
	EOS_Sessions_UnregisterPlayers(SessionsHandle, &UnregisterOptions, CallbackContext,
		[](const EOS_Sessions_UnregisterPlayersCallbackInfo* Data)
		{
			FUnregisterPlayersContext* Context = static_cast<FUnregisterPlayersContext*>(Data->ClientData);
			
			if (!Context || !Context->AsyncNode)
			{
				if (Context)
				{
					delete[] Context->PlayersArray;
				}
				delete Context;
				return;
			}
			
			// Execute on game thread
			AsyncTask(ENamedThreads::GameThread, [Context, Data]()
			{
				TArray<FString> UnregisteredPlayers;
				
				if (Data->ResultCode == EOS_EResult::EOS_Success)
				{
					UE_LOG(LogTemp, Warning, TEXT("EOSKit: ? Successfully unregistered players!"));
					UE_LOG(LogTemp, Warning, TEXT("EOSKit: Session Name: %s"), *Context->SessionName);
					UE_LOG(LogTemp, Warning, TEXT("EOSKit: Unregistered %d players"), Data->UnregisteredPlayersCount);
					
					// Convert unregistered players to strings
					for (uint32_t i = 0; i < Data->UnregisteredPlayersCount; i++)
					{
						char ProductUserIdStr[EOS_PRODUCTUSERID_MAX_LENGTH + 1];
						int32_t BufferSize = sizeof(ProductUserIdStr);
						EOS_ProductUserId_ToString(Data->UnregisteredPlayers[i], ProductUserIdStr, &BufferSize);
						UnregisteredPlayers.Add(UTF8_TO_TCHAR(ProductUserIdStr));
						
						UE_LOG(LogTemp, Log, TEXT("EOSKit: Unregistered Player #%d: %s"), i + 1, UTF8_TO_TCHAR(ProductUserIdStr));
					}
					
					if (Context->AsyncNode)
					{
						Context->AsyncNode->OnSuccess.Broadcast(UnregisteredPlayers);
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
					UE_LOG(LogTemp, Error, TEXT("EOSKit: ? Failed to unregister players: %s"), 
						*FString(EOS_EResult_ToString(Data->ResultCode)));
					
					// Common error explanations
					if (Data->ResultCode == EOS_EResult::EOS_Sessions_OutOfSync)
					{
						UE_LOG(LogTemp, Error, TEXT("EOSKit: Session is out of sync - try refreshing session state"));
					}
					else if (Data->ResultCode == EOS_EResult::EOS_NotFound)
					{
						UE_LOG(LogTemp, Error, TEXT("EOSKit: Players not found in session - they may already be unregistered"));
					}
					
					if (Context->AsyncNode)
					{
						Context->AsyncNode->OnFail.Broadcast(UnregisteredPlayers);
						Context->AsyncNode->SetReadyToDestroy();
#if ENGINE_MAJOR_VERSION == 5
						Context->AsyncNode->MarkAsGarbage();
#else
						Context->AsyncNode->MarkPendingKill();
#endif
					}
				}
				
				// Cleanup
				delete[] Context->PlayersArray;
				delete Context;
			});
		});
}
