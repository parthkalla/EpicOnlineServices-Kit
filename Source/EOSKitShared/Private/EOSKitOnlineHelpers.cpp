// Copyright (C) 2024, All Rights Reserved.

#include "EOSKitOnlineHelpers.h"

#include "Engine/World.h"
#include "Interfaces/OnlineExternalUIInterface.h"
#include "Interfaces/OnlineIdentityInterface.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "Math/UnrealMathUtility.h"
#include "OnlineSubsystem.h"
#include "OnlineSubsystemUtils.h"

namespace
{
	IOnlineSubsystem* GetSubsystem(UWorld* WorldContext)
	{
		return Online::GetSubsystem(WorldContext);
	}

	IOnlineSubsystem* GetSubsystemFromWorldOrDefault(UWorld* WorldContext)
	{
		if (IOnlineSubsystem* Subsystem = GetSubsystem(WorldContext))
		{
			return Subsystem;
		}

		return Online::GetSubsystem(nullptr);
	}
}

namespace FEOSKitOnlineHelpers
{
	FString GetPlayerNickname(int32 LocalUserNum)
	{
		if (const IOnlineSubsystem* Subsystem = GetSubsystemFromWorldOrDefault(nullptr))
		{
			if (const IOnlineIdentityPtr Identity = Subsystem->GetIdentityInterface())
			{
				return Identity->GetPlayerNickname(LocalUserNum);
			}
		}

		return FString();
	}

	bool IsPlayerLoggedIn(int32 LocalUserNum)
	{
		if (const IOnlineSubsystem* Subsystem = GetSubsystemFromWorldOrDefault(nullptr))
		{
			if (const IOnlineIdentityPtr Identity = Subsystem->GetIdentityInterface())
			{
				return Identity->GetLoginStatus(LocalUserNum) == ELoginStatus::LoggedIn;
			}
		}

		return false;
	}

	FString GetProductUserId(int32 LocalUserNum)
	{
		if (const IOnlineSubsystem* Subsystem = GetSubsystemFromWorldOrDefault(nullptr))
		{
			if (const IOnlineIdentityPtr Identity = Subsystem->GetIdentityInterface())
			{
				if (TSharedPtr<const FUniqueNetId> UniqueId = Identity->GetUniquePlayerId(LocalUserNum))
				{
					const FString UniqueIdString = UniqueId->ToString();
					TArray<FString> Parts;
					UniqueIdString.ParseIntoArray(Parts, TEXT("|"));

					if (Parts.Num() >= 2)
					{
						return Parts[1];
					}

					if (Parts.Num() == 1)
					{
						return Parts[0];
					}
				}
			}
		}

		return FString();
	}

	FString GetEpicAccountId(int32 LocalUserNum)
	{
		if (const IOnlineSubsystem* Subsystem = GetSubsystemFromWorldOrDefault(nullptr))
		{
			if (const IOnlineIdentityPtr Identity = Subsystem->GetIdentityInterface())
			{
				if (TSharedPtr<const FUniqueNetId> UniqueId = Identity->GetUniquePlayerId(LocalUserNum))
				{
					const FString UniqueIdString = UniqueId->ToString();
					TArray<FString> Parts;
					UniqueIdString.ParseIntoArray(Parts, TEXT("|"));

					if (Parts.Num() >= 1)
					{
						return Parts[0];
					}
				}
			}
		}

		return FString();
	}

	FString GenerateSessionCode(int32 CodeLength)
	{
		const FString Characters = TEXT("ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789");
		FString Code;
		Code.Reserve(CodeLength);

		for (int32 Index = 0; Index < CodeLength; ++Index)
		{
			const int32 RandomIndex = FMath::RandRange(0, Characters.Len() - 1);
			Code.AppendChar(Characters[RandomIndex]);
		}

		return Code;
	}

	void RegisterLocalPlayer(UWorld* World, FName SessionName, bool bWasInvited)
	{
		if (!World)
		{
			return;
		}

		if (IOnlineSubsystem* Subsystem = GetSubsystem(World))
		{
			if (const IOnlineSessionPtr SessionInterface = Subsystem->GetSessionInterface())
			{
				if (const IOnlineIdentityPtr Identity = Subsystem->GetIdentityInterface())
				{
					if (TSharedPtr<const FUniqueNetId> UniqueId = Identity->GetUniquePlayerId(0))
					{
						SessionInterface->RegisterPlayer(SessionName, *UniqueId, bWasInvited);
					}
				}
			}
		}
	}

	void UnregisterLocalPlayer(UWorld* World, FName SessionName)
	{
		if (!World)
		{
			return;
		}

		if (IOnlineSubsystem* Subsystem = GetSubsystem(World))
		{
			if (const IOnlineSessionPtr SessionInterface = Subsystem->GetSessionInterface())
			{
				if (const IOnlineIdentityPtr Identity = Subsystem->GetIdentityInterface())
				{
					if (TSharedPtr<const FUniqueNetId> UniqueId = Identity->GetUniquePlayerId(0))
					{
						SessionInterface->UnregisterPlayer(SessionName, *UniqueId);
					}
				}
			}
		}
	}

	bool StartSession(UWorld* World, FName SessionName)
	{
		if (!World)
		{
			return false;
		}

		if (IOnlineSubsystem* Subsystem = GetSubsystem(World))
		{
			if (const IOnlineSessionPtr SessionInterface = Subsystem->GetSessionInterface())
			{
				return SessionInterface->StartSession(SessionName);
			}
		}

		return false;
	}

	bool EndSession(UWorld* World, FName SessionName)
	{
		if (!World)
		{
			return false;
		}

		if (IOnlineSubsystem* Subsystem = GetSubsystem(World))
		{
			if (const IOnlineSessionPtr SessionInterface = Subsystem->GetSessionInterface())
			{
				return SessionInterface->EndSession(SessionName);
			}
		}

		return false;
	}

	bool ShowFriendsInterface()
	{
		if (const IOnlineSubsystem* Subsystem = GetSubsystemFromWorldOrDefault(nullptr))
		{
			if (const IOnlineExternalUIPtr ExternalUI = Subsystem->GetExternalUIInterface())
			{
				return ExternalUI->ShowFriendsUI(0);
			}
		}

		return false;
	}

	bool IsEOSKitInitialized()
	{
		if (const IOnlineSubsystem* Subsystem = GetSubsystemFromWorldOrDefault(nullptr))
		{
			const FName Name = Subsystem->GetSubsystemName();
			if (Name == TEXT("EOS") || Name == TEXT("EOSPlus"))
			{
				return Subsystem->GetIdentityInterface().IsValid();
			}
		}

		return false;
	}
}

