// Copyright (C) 2024, All Rights Reserved.

#include "EOSJoinEOKSessionAsync.h"
#include "OnlineSubsystem.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerController.h"
#include "Engine/Engine.h"
#include "Engine/World.h"

UEOSJoinEOKSessionAsync* UEOSJoinEOKSessionAsync::JoinEOKSession(
	UObject* WorldContextObject,
	FName SessionName,
	FEOSKitSessionFindResult SessionResult,
	bool bUsePresence)
{
	UEOSJoinEOKSessionAsync* Node = NewObject<UEOSJoinEOKSessionAsync>();
	Node->CachedWorldContextObject = WorldContextObject;
	Node->VSessionName = SessionName;
	Node->SessionResult = SessionResult;
	Node->bUsePresence = bUsePresence;
	return Node;
}

void UEOSJoinEOKSessionAsync::Activate()
{
	JoinSession();
	Super::Activate();
}

void UEOSJoinEOKSessionAsync::JoinSession()
{
	UE_LOG(LogTemp, Warning, TEXT("EOSKit: ========================================"));
	UE_LOG(LogTemp, Warning, TEXT("EOSKit: Joining EOS Session via OnlineSubsystem (like EIK)"));
	UE_LOG(LogTemp, Warning, TEXT("EOSKit: Session Name: %s"), *VSessionName.ToString());
	UE_LOG(LogTemp, Warning, TEXT("EOSKit: ========================================"));
	
	if (!CachedWorldContextObject)
	{
		UE_LOG(LogTemp, Error, TEXT("EOSKit: WorldContextObject is null"));
		OnFail.Broadcast();
		SetReadyToDestroy();
#if ENGINE_MAJOR_VERSION == 5
		MarkAsGarbage();
#else
		MarkPendingKill();
#endif
		return;
	}
	
	// Use OnlineSubsystem interface (like EIK). Force EOSKit explicitly.
	if (const IOnlineSubsystem* Subsystem = IOnlineSubsystem::Get(TEXT("EOSKit")))
	{
		if (const IOnlineSessionPtr SessionPtr = Subsystem->GetSessionInterface())
		{
			// Register callback
			SessionPtr->OnJoinSessionCompleteDelegates.AddUObject(this, &UEOSJoinEOKSessionAsync::OnJoinSessionCompleted);
			
			// Join session using the stored OnlineResult from FindSessions
			UE_LOG(LogTemp, Log, TEXT("EOSKit: Calling OnlineSubsystem(EOSKit)->JoinSession()..."));
			SessionPtr->JoinSession(0, VSessionName, SessionResult.OnlineResult);
			return;
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("EOSKit: SessionInterface is null"));
			OnFail.Broadcast();
			SetReadyToDestroy();
#if ENGINE_MAJOR_VERSION == 5
			MarkAsGarbage();
#else
			MarkPendingKill();
#endif
			return;
		}
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("EOSKit: OnlineSubsystem(EOSKit) is null"));
		OnFail.Broadcast();
		SetReadyToDestroy();
#if ENGINE_MAJOR_VERSION == 5
		MarkAsGarbage();
#else
		MarkPendingKill();
#endif
		return;
	}
}

void UEOSJoinEOKSessionAsync::OnJoinSessionCompleted(FName SessionName, EOnJoinSessionCompleteResult::Type Result)
{
	if (Result == EOnJoinSessionCompleteResult::Success)
	{
		UE_LOG(LogTemp, Warning, TEXT("EOSKit: ✅ Successfully joined session!"));
		
		// Official EOS approach: Get connect string and call ClientTravel directly (matches EIK)
		if (const IOnlineSubsystem* Subsystem = IOnlineSubsystem::Get())
		{
			if (const IOnlineSessionPtr SessionPtr = Subsystem->GetSessionInterface())
			{
				FString JoinAddress;
				if (SessionPtr->GetResolvedConnectString(SessionName, JoinAddress))
				{
					// CRITICAL: For EOS P2P addresses, ensure no map path is appended
					// The EOS address format is: EOS:ProductUserId:SocketName:Channel
					// Unreal Engine may append the current map path, so we need to clean it
					FString CleanAddress = JoinAddress;
					
					// Remove any trailing map paths that might have been added
					// Check for common map path patterns
					int32 MapPathIndex = CleanAddress.Find(TEXT("/Game/"), ESearchCase::CaseSensitive);
					if (MapPathIndex == INDEX_NONE)
					{
						MapPathIndex = CleanAddress.Find(TEXT("/"), ESearchCase::CaseSensitive);
						// Only remove if it's after the EOS address (not part of the EOS: prefix)
						if (MapPathIndex != INDEX_NONE && MapPathIndex > 0 && CleanAddress[MapPathIndex - 1] != TEXT(':'))
						{
							// This might be a map path, but be careful not to remove valid parts
							// For EOS URLs, anything after the channel number that starts with / is likely a map path
							int32 ChannelIndex = CleanAddress.Find(TEXT(":"), ESearchCase::CaseSensitive, ESearchDir::FromEnd);
							if (ChannelIndex != INDEX_NONE && MapPathIndex > ChannelIndex)
							{
								CleanAddress = CleanAddress.Left(MapPathIndex);
								UE_LOG(LogTemp, Warning, TEXT("EOSKit: Removed map path from EOS address, using: %s"), *CleanAddress);
							}
						}
					}
					else
					{
						CleanAddress = CleanAddress.Left(MapPathIndex);
						UE_LOG(LogTemp, Warning, TEXT("EOSKit: Removed /Game/ map path from EOS address, using: %s"), *CleanAddress);
					}
					
					// Log the address we're about to use
					UE_LOG(LogTemp, Warning, TEXT("EOSKit: Original address: %s"), *JoinAddress);
					UE_LOG(LogTemp, Warning, TEXT("EOSKit: Clean address: %s"), *CleanAddress);
					
					// Get world and PlayerController
					UWorld* World = GEngine ? GEngine->GetWorldFromContextObject(CachedWorldContextObject, EGetWorldErrorMode::LogAndReturnNull) : nullptr;
					if (!World && CachedWorldContextObject)
					{
						World = CachedWorldContextObject->GetWorld();
					}
					
					if (World)
					{
						if (APlayerController* PC = World->GetFirstPlayerController())
						{
							// CRITICAL: For EOS URLs, Unreal Engine may append the current map path during Browse
							// To prevent this, we add "?" to the end of the URL, which tells Unreal Engine
							// that this is a connection-only URL and no map should be loaded
							// The server will tell the client which map to load after connection
							FString FinalAddress = CleanAddress;
							if (!FinalAddress.EndsWith(TEXT("?")) && !FinalAddress.Contains(TEXT("?")))
							{
								FinalAddress += TEXT("?");
							}
							
							// CRITICAL: Use TRAVEL_Absolute and ensure the URL doesn't trigger map path appending
							// For EOS URLs, we must pass them exactly as-is without any modifications
							UE_LOG(LogTemp, Warning, TEXT("EOSKit: ✅ Calling ClientTravel with: %s (TRAVEL_Absolute, with ? suffix to prevent map loading)"), *FinalAddress);
							PC->ClientTravel(FinalAddress, ETravelType::TRAVEL_Absolute);
							OnSuccess.Broadcast();
						}
						else
						{
							UE_LOG(LogTemp, Error, TEXT("EOSKit: Failed to get PlayerController from world"));
							OnFail.Broadcast();
						}
					}
					else
					{
						UE_LOG(LogTemp, Error, TEXT("EOSKit: Failed to get World from context object"));
						OnFail.Broadcast();
					}
				}
				else
				{
					UE_LOG(LogTemp, Error, TEXT("EOSKit: Failed to get connect string"));
					OnFail.Broadcast();
				}
			}
			else
			{
				UE_LOG(LogTemp, Error, TEXT("EOSKit: SessionInterface is null"));
				OnFail.Broadcast();
			}
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("EOSKit: OnlineSubsystem is null"));
			OnFail.Broadcast();
		}
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("EOSKit: ❌ Failed to join session: %d"), static_cast<int32>(Result));
		OnFail.Broadcast();
	}
	
	SetReadyToDestroy();
#if ENGINE_MAJOR_VERSION == 5
	MarkAsGarbage();
#else
	MarkPendingKill();
#endif
}
