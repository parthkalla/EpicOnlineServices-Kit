// Copyright (C) 2024, All Rights Reserved.

#include "EOSGetPlatformAuthTokenAsync.h"
#include "OnlineSubsystem.h"
#include "Interfaces/OnlineIdentityInterface.h"
#include "Engine/World.h"

UEOSGetPlatformAuthTokenAsync* UEOSGetPlatformAuthTokenAsync::GetPlatformAuthToken(
	UObject* WorldContextObject,
	int32 LocalUserNum,
	const FString& TokenType)
{
	UEOSGetPlatformAuthTokenAsync* BlueprintNode = NewObject<UEOSGetPlatformAuthTokenAsync>();
	BlueprintNode->WorldContextObject = WorldContextObject;
	BlueprintNode->LocalUserNum = LocalUserNum;
	BlueprintNode->TokenType = TokenType;
	return BlueprintNode;
}

void UEOSGetPlatformAuthTokenAsync::Activate()
{
	Super::Activate();

	if (!WorldContextObject)
	{
		UE_LOG(LogTemp, Error, TEXT("EOSGetPlatformAuthTokenAsync: WorldContextObject is null"));
		OnFailure.Broadcast(TEXT(""));
		SetReadyToDestroy();
		return;
	}

	// Get the platform OnlineSubsystem (Steam, etc.)
	IOnlineSubsystem* PlatformOSS = IOnlineSubsystem::GetByPlatform();
	if (!PlatformOSS)
	{
		// On desktop, try to load Steam
		#if PLATFORM_DESKTOP
		PlatformOSS = IOnlineSubsystem::Get(TEXT("Steam"));
		#endif
	}

	if (!PlatformOSS)
	{
		UE_LOG(LogTemp, Error, TEXT("EOSGetPlatformAuthTokenAsync: Failed to get platform OnlineSubsystem"));
		OnFailure.Broadcast(TEXT(""));
		SetReadyToDestroy();
		return;
	}

	IOnlineIdentityPtr PlatformIdentity = PlatformOSS->GetIdentityInterface();
	if (!PlatformIdentity.IsValid())
	{
		UE_LOG(LogTemp, Error, TEXT("EOSGetPlatformAuthTokenAsync: Failed to get platform Identity interface"));
		OnFailure.Broadcast(TEXT(""));
		SetReadyToDestroy();
		return;
	}

	// Create delegate for callback
	FOnGetLinkedAccountAuthTokenCompleteDelegate Delegate = 
		FOnGetLinkedAccountAuthTokenCompleteDelegate::CreateUObject(
			this, 
			&UEOSGetPlatformAuthTokenAsync::OnGetPlatformAuthTokenComplete
		);

	// Request the auth token from the platform
	#if ENGINE_MAJOR_VERSION == 5 && ENGINE_MINOR_VERSION >= 2
	// UE 5.2+ requires TokenType parameter
	if (!TokenType.IsEmpty())
	{
		PlatformIdentity->GetLinkedAccountAuthToken(LocalUserNum, TokenType, Delegate);
	}
	else
	{
		// Default to "Session" for Steam
		FString DefaultTokenType = PlatformOSS->GetSubsystemName() == TEXT("Steam") ? TEXT("Session") : TEXT("");
		if (!DefaultTokenType.IsEmpty())
		{
			PlatformIdentity->GetLinkedAccountAuthToken(LocalUserNum, DefaultTokenType, Delegate);
		}
		else
		{
			PlatformIdentity->GetLinkedAccountAuthToken(LocalUserNum, Delegate);
		}
	}
	#else
	// UE 5.1 and earlier
	PlatformIdentity->GetLinkedAccountAuthToken(LocalUserNum, Delegate);
	#endif
}

void UEOSGetPlatformAuthTokenAsync::OnGetPlatformAuthTokenComplete(
	int32 LocalUserNum,
	bool bWasSuccessful,
	const FExternalAuthToken& ExternalAuthToken)
{
	if (bWasSuccessful && !ExternalAuthToken.TokenString.IsEmpty())
	{
		UE_LOG(LogTemp, Log, TEXT("EOSGetPlatformAuthTokenAsync: Successfully retrieved platform auth token"));
		OnSuccess.Broadcast(ExternalAuthToken.TokenString);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("EOSGetPlatformAuthTokenAsync: Failed to retrieve platform auth token"));
		OnFailure.Broadcast(ExternalAuthToken.TokenString);
	}
	SetReadyToDestroy();
}

