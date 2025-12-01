// Copyright (C) 2024, All Rights Reserved.

#include "EOSRegisterPlayersAsync.h"
#include "EOSKitSubsystem.h"
#include "Kismet/GameplayStatics.h"
#if WITH_EOS_SDK
#include "Windows/AllowWindowsPlatformTypes.h"
#include "eos_sdk.h"
#include "eos_sessions.h"
#include "Windows/HideWindowsPlatformTypes.h"
#endif
#include "Async/Async.h"

UEOSRegisterPlayersAsync* UEOSRegisterPlayersAsync::RegisterPlayers(
	FName SessionName,
	const TArray<FString>& PlayersToRegister)
{
	UEOSRegisterPlayersAsync* Node = NewObject<UEOSRegisterPlayersAsync>();
	Node->VSessionName = SessionName;
	Node->PlayerIds = PlayersToRegister;
	return Node;
}

void UEOSRegisterPlayersAsync::Activate()
{
	RegisterPlayersInSession();
	Super::Activate();
}

void UEOSRegisterPlayersAsync::RegisterPlayersInSession()
{
	UE_LOG(LogTemp, Warning, TEXT("EOSKit: ========================================"));
	UE_LOG(LogTemp, Warning, TEXT("EOSKit: Registering Players in Session via SDK"));
	UE_LOG(LogTemp, Warning, TEXT("EOSKit: Session Name: %s"), *VSessionName.ToString());
	UE_LOG(LogTemp, Warning, TEXT("EOSKit: Players to register: %d"), PlayerIds.Num());
	UE_LOG(LogTemp, Warning, TEXT("EOSKit: ========================================"));
	
	// Get the EOSKit subsystem
	UGameInstance* GameInstance = GetWorld()->GetGameInstance();
	if (!GameInstance)
	{
		UE_LOG(LogTemp, Error, TEXT("EOSKit: Failed to get Game Instance"));
		OnFail.Broadcast(TArray<FString>(), TArray<FString>());
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
		OnFail.Broadcast(TArray<FString>(), TArray<FString>());
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
		OnFail.Broadcast(TArray<FString>(), TArray<FString>());
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
		OnFail.Broadcast(TArray<FString>(), TArray<FString>());
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
		// If no player IDs provided, register the local player
		ProductUserIds.Add(LocalUserId);
		UE_LOG(LogTemp, Log, TEXT("EOSKit: No player IDs provided, registering local player only"));
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
		UE_LOG(LogTemp, Warning, TEXT("EOSKit: Note: Only local player will be registered (string conversion not implemented)"));
	}
	
	// Allocate array for EOS SDK
	EOS_ProductUserId* PlayersArray = new EOS_ProductUserId[ProductUserIds.Num()];
	for (int32 i = 0; i < ProductUserIds.Num(); i++)
	{
		PlayersArray[i] = ProductUserIds[i];
	}
	
	// Setup Register Players Options
	EOS_Sessions_RegisterPlayersOptions RegisterOptions = {};
	RegisterOptions.ApiVersion = EOS_SESSIONS_REGISTERPLAYERS_API_LATEST;
	
	FTCHARToUTF8 SessionNameConverter(*VSessionName.ToString());
	RegisterOptions.SessionName = SessionNameConverter.Get();
	RegisterOptions.PlayersToRegister = PlayersArray;
	RegisterOptions.PlayersToRegisterCount = ProductUserIds.Num();
	
	// Create callback context
	struct FRegisterPlayersContext
	{
		UEOSRegisterPlayersAsync* AsyncNode;
		FString SessionName;
		EOS_ProductUserId* PlayersArray;
	};
	
	FRegisterPlayersContext* CallbackContext = new FRegisterPlayersContext();
	CallbackContext->AsyncNode = this;
	CallbackContext->SessionName = VSessionName.ToString();
	CallbackContext->PlayersArray = PlayersArray; // Store for cleanup
	
	// Call EOS SDK to register players
	EOS_Sessions_RegisterPlayers(SessionsHandle, &RegisterOptions, CallbackContext,
		[](const EOS_Sessions_RegisterPlayersCallbackInfo* Data)
		{
			FRegisterPlayersContext* Context = static_cast<FRegisterPlayersContext*>(Data->ClientData);
			
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
				TArray<FString> RegisteredPlayers;
				TArray<FString> SanctionedPlayers;
				
				if (Data->ResultCode == EOS_EResult::EOS_Success)
				{
					UE_LOG(LogTemp, Warning, TEXT("EOSKit: ? Successfully registered players!"));
					UE_LOG(LogTemp, Warning, TEXT("EOSKit: Session Name: %s"), *Context->SessionName);
					UE_LOG(LogTemp, Warning, TEXT("EOSKit: Registered %d players"), Data->RegisteredPlayersCount);
					UE_LOG(LogTemp, Warning, TEXT("EOSKit: Sanctioned %d players"), Data->SanctionedPlayersCount);
					
					// Convert registered players to strings
					for (uint32_t i = 0; i < Data->RegisteredPlayersCount; i++)
					{
						char ProductUserIdStr[EOS_PRODUCTUSERID_MAX_LENGTH + 1];
						int32_t BufferSize = sizeof(ProductUserIdStr);
						EOS_ProductUserId_ToString(Data->RegisteredPlayers[i], ProductUserIdStr, &BufferSize);
						RegisteredPlayers.Add(UTF8_TO_TCHAR(ProductUserIdStr));
						
						UE_LOG(LogTemp, Log, TEXT("EOSKit: Registered Player #%d: %s"), i + 1, UTF8_TO_TCHAR(ProductUserIdStr));
					}
					
					// Convert sanctioned players to strings
					for (uint32_t i = 0; i < Data->SanctionedPlayersCount; i++)
					{
						char ProductUserIdStr[EOS_PRODUCTUSERID_MAX_LENGTH + 1];
						int32_t BufferSize = sizeof(ProductUserIdStr);
						EOS_ProductUserId_ToString(Data->SanctionedPlayers[i], ProductUserIdStr, &BufferSize);
						SanctionedPlayers.Add(UTF8_TO_TCHAR(ProductUserIdStr));
						
						UE_LOG(LogTemp, Warning, TEXT("EOSKit: ?? Sanctioned Player #%d: %s"), i + 1, UTF8_TO_TCHAR(ProductUserIdStr));
					}
					
					if (Context->AsyncNode)
					{
						Context->AsyncNode->OnSuccess.Broadcast(RegisteredPlayers, SanctionedPlayers);
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
					UE_LOG(LogTemp, Error, TEXT("EOSKit: ? Failed to register players: %s"), 
						*FString(EOS_EResult_ToString(Data->ResultCode)));
					
					if (Context->AsyncNode)
					{
						Context->AsyncNode->OnFail.Broadcast(RegisteredPlayers, SanctionedPlayers);
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
