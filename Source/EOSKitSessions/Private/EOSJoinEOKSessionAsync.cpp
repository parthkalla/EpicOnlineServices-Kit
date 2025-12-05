// Copyright (C) 2024, All Rights Reserved.

#include "EOSJoinEOKSessionAsync.h"
#include "OnlineSubsystem.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerController.h"

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
	
	// Use OnlineSubsystem interface (like EIK)
	if (const IOnlineSubsystem* Subsystem = IOnlineSubsystem::Get())
	{
		if (const IOnlineSessionPtr SessionPtr = Subsystem->GetSessionInterface())
		{
			// Register callback
			SessionPtr->OnJoinSessionCompleteDelegates.AddUObject(this, &UEOSJoinEOKSessionAsync::OnJoinSessionCompleted);
			
			// Join session using the stored OnlineResult from FindSessions
			UE_LOG(LogTemp, Log, TEXT("EOSKit: Calling OnlineSubsystem->JoinSession()..."));
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
		UE_LOG(LogTemp, Error, TEXT("EOSKit: OnlineSubsystem is null"));
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
		
		// Get connect string and travel
		if (APlayerController* PC = UGameplayStatics::GetPlayerController(CachedWorldContextObject, 0))
		{
			if (const IOnlineSubsystem* Subsystem = IOnlineSubsystem::Get())
			{
				if (const IOnlineSessionPtr SessionPtr = Subsystem->GetSessionInterface())
				{
					FString JoinAddress;
					if (SessionPtr->GetResolvedConnectString(SessionName, JoinAddress))
					{
						// EIK passes the address directly to ClientTravel without modification
						// The EOS NetDriver will handle the EOS:ProductUserId:SocketName:Channel format
						UE_LOG(LogTemp, Warning, TEXT("EOSKit: Traveling to: %s"), *JoinAddress);
						PC->ClientTravel(JoinAddress, ETravelType::TRAVEL_Absolute);
						OnSuccess.Broadcast();
					}
					else
					{
						UE_LOG(LogTemp, Error, TEXT("EOSKit: Failed to get connect string"));
						OnFail.Broadcast();
					}
				}
			}
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
