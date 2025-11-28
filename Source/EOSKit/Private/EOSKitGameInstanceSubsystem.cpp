// Copyright (C) 2024, All Rights Reserved.

#include "EOSKitGameInstanceSubsystem.h"
#include "EOSKitSubsystem.h"
#include "OnlineSubsystemUtils.h"
#include "Interfaces/OnlineIdentityInterface.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "Interfaces/OnlineExternalUIInterface.h"
#include "Kismet/GameplayStatics.h"

void UEOSKitGameInstanceSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	
	bIsInitialized = true;
	
	UE_LOG(LogTemp, Log, TEXT("EOSKit: Game Instance Subsystem Initialized"));
}

void UEOSKitGameInstanceSubsystem::Deinitialize()
{
	bIsInitialized = false;
	
	UE_LOG(LogTemp, Log, TEXT("EOSKit: Game Instance Subsystem Deinitialized"));
	
	Super::Deinitialize();
}

// ========================================
// User Information
// ========================================

FString UEOSKitGameInstanceSubsystem::GetPlayerNickname(int32 LocalUserNum)
{
	if (const IOnlineSubsystem* Subsystem = Online::GetSubsystem(nullptr))
	{
		if (const IOnlineIdentityPtr Identity = Subsystem->GetIdentityInterface())
		{
			return Identity->GetPlayerNickname(LocalUserNum);
		}
	}
	return FString();
}

bool UEOSKitGameInstanceSubsystem::IsPlayerLoggedIn(int32 LocalUserNum)
{
	if (const IOnlineSubsystem* Subsystem = Online::GetSubsystem(nullptr))
	{
		if (const IOnlineIdentityPtr Identity = Subsystem->GetIdentityInterface())
		{
			return Identity->GetLoginStatus(LocalUserNum) == ELoginStatus::LoggedIn;
		}
	}
	return false;
}

FString UEOSKitGameInstanceSubsystem::GetProductUserId(int32 LocalUserNum)
{
	if (const IOnlineSubsystem* Subsystem = Online::GetSubsystem(nullptr))
	{
		if (const IOnlineIdentityPtr Identity = Subsystem->GetIdentityInterface())
		{
			if (TSharedPtr<const FUniqueNetId> UniqueId = Identity->GetUniquePlayerId(LocalUserNum))
			{
				return ParseProductUserId(UniqueId->ToString());
			}
		}
	}
	return FString();
}

FString UEOSKitGameInstanceSubsystem::GetEpicAccountId(int32 LocalUserNum)
{
	if (const IOnlineSubsystem* Subsystem = Online::GetSubsystem(nullptr))
	{
		if (const IOnlineIdentityPtr Identity = Subsystem->GetIdentityInterface())
		{
			if (TSharedPtr<const FUniqueNetId> UniqueId = Identity->GetUniquePlayerId(LocalUserNum))
			{
				return ParseEpicAccountId(UniqueId->ToString());
			}
		}
	}
	return FString();
}

// ========================================
// Session Utilities
// ========================================

FString UEOSKitGameInstanceSubsystem::GenerateSessionCode(int32 CodeLength)
{
	const FString Characters = TEXT("ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789");
	FString Code;
	Code.Reserve(CodeLength);
	
	for (int32 i = 0; i < CodeLength; i++)
	{
		const int32 RandomIndex = FMath::RandRange(0, Characters.Len() - 1);
		Code.AppendChar(Characters[RandomIndex]);
	}
	
	return Code;
}

void UEOSKitGameInstanceSubsystem::RegisterLocalPlayer(FName SessionName, bool bWasInvited)
{
	if (const IOnlineSubsystem* Subsystem = Online::GetSubsystem(GetWorld()))
	{
		if (const IOnlineSessionPtr Session = Subsystem->GetSessionInterface())
		{
			if (const IOnlineIdentityPtr Identity = Subsystem->GetIdentityInterface())
			{
				if (TSharedPtr<const FUniqueNetId> UniqueId = Identity->GetUniquePlayerId(0))
				{
					if (Session->RegisterPlayer(SessionName, *UniqueId, bWasInvited))
					{
						UE_LOG(LogTemp, Log, TEXT("EOSKit: Successfully registered player in session '%s'"), *SessionName.ToString());
					}
					else
					{
						UE_LOG(LogTemp, Warning, TEXT("EOSKit: Failed to register player in session '%s'"), *SessionName.ToString());
					}
				}
			}
		}
	}
}

void UEOSKitGameInstanceSubsystem::UnregisterLocalPlayer(FName SessionName)
{
	if (const IOnlineSubsystem* Subsystem = Online::GetSubsystem(GetWorld()))
	{
		if (const IOnlineSessionPtr Session = Subsystem->GetSessionInterface())
		{
			if (const IOnlineIdentityPtr Identity = Subsystem->GetIdentityInterface())
			{
				if (TSharedPtr<const FUniqueNetId> UniqueId = Identity->GetUniquePlayerId(0))
				{
					if (Session->UnregisterPlayer(SessionName, *UniqueId))
					{
						UE_LOG(LogTemp, Log, TEXT("EOSKit: Successfully unregistered player from session '%s'"), *SessionName.ToString());
					}
					else
					{
						UE_LOG(LogTemp, Warning, TEXT("EOSKit: Failed to unregister player from session '%s'"), *SessionName.ToString());
					}
				}
			}
		}
	}
}

bool UEOSKitGameInstanceSubsystem::StartSession(FName SessionName)
{
	if (const IOnlineSubsystem* Subsystem = Online::GetSubsystem(GetWorld()))
	{
		if (const IOnlineSessionPtr Session = Subsystem->GetSessionInterface())
		{
			if (Session->StartSession(SessionName))
			{
				UE_LOG(LogTemp, Log, TEXT("EOSKit: Successfully started session '%s'"), *SessionName.ToString());
				return true;
			}
			else
			{
				UE_LOG(LogTemp, Warning, TEXT("EOSKit: Failed to start session '%s'"), *SessionName.ToString());
			}
		}
	}
	return false;
}

bool UEOSKitGameInstanceSubsystem::EndSession(FName SessionName)
{
	if (const IOnlineSubsystem* Subsystem = Online::GetSubsystem(GetWorld()))
	{
		if (const IOnlineSessionPtr Session = Subsystem->GetSessionInterface())
		{
			if (Session->EndSession(SessionName))
			{
				UE_LOG(LogTemp, Log, TEXT("EOSKit: Successfully ended session '%s'"), *SessionName.ToString());
				return true;
			}
			else
			{
				UE_LOG(LogTemp, Warning, TEXT("EOSKit: Failed to end session '%s'"), *SessionName.ToString());
			}
		}
	}
	return false;
}

// ========================================
// Social Features
// ========================================

bool UEOSKitGameInstanceSubsystem::ShowFriendsInterface()
{
	if (const IOnlineSubsystem* Subsystem = Online::GetSubsystem(nullptr))
	{
		if (const IOnlineExternalUIPtr ExternalUI = Subsystem->GetExternalUIInterface())
		{
			if (ExternalUI->ShowFriendsUI(0))
			{
				UE_LOG(LogTemp, Log, TEXT("EOSKit: Successfully opened friends interface"));
				return true;
			}
			else
			{
				UE_LOG(LogTemp, Warning, TEXT("EOSKit: Failed to open friends interface"));
			}
		}
	}
	return false;
}

// ========================================
// Helper Functions
// ========================================

FString UEOSKitGameInstanceSubsystem::ParseProductUserId(const FString& UniqueNetIdString)
{
	// EOS format: "EpicAccountId|ProductUserId"
	TArray<FString> Parts;
	UniqueNetIdString.ParseIntoArray(Parts, TEXT("|"));
	
	if (Parts.Num() == 2)
	{
		// Return the ProductUserId (second part)
		return Parts[1];
	}
	else if (Parts.Num() == 1)
	{
		// If there's no pipe, assume it's just the ProductUserId
		return Parts[0];
	}
	
	return FString();
}

FString UEOSKitGameInstanceSubsystem::ParseEpicAccountId(const FString& UniqueNetIdString)
{
	// EOS format: "EpicAccountId|ProductUserId"
	TArray<FString> Parts;
	UniqueNetIdString.ParseIntoArray(Parts, TEXT("|"));
	
	if (Parts.Num() >= 2)
	{
		// Return the EpicAccountId (first part)
		return Parts[0];
	}
	
	return FString();
}

bool UEOSKitGameInstanceSubsystem::IsEOSKitInitialized()
{
	// Check if the main EOSKit subsystem is initialized
	if (const IOnlineSubsystem* Subsystem = Online::GetSubsystem(nullptr))
	{
		// Check if it's the EOS subsystem
		if (Subsystem->GetSubsystemName() == TEXT("EOS") || 
			Subsystem->GetSubsystemName() == TEXT("EOSPlus"))
		{
			// Verify we can get the identity interface
			if (Subsystem->GetIdentityInterface().IsValid())
			{
				return true;
			}
		}
	}
	
	return false;
}
