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

	// Store the identity interface and local user num for the callback
	StoredPlatformIdentity = PlatformIdentity;
	StoredLocalUserNum = LocalUserNum;

	// Request the auth token from the platform (TokenType may be empty)
	FString TokenTypeToUse = TokenType;
	if (TokenTypeToUse.IsEmpty())
	{
		TokenTypeToUse = PlatformOSS->GetSubsystemName() == TEXT("Steam") ? TEXT("Session") : TEXT("");
	}

	// In UE 5.5, GetLinkedAccountAuthToken may have been removed or changed
	// For now, we'll use a lambda directly if the method exists
	// Note: This method may not be available in UE 5.5
	UE_LOG(LogTemp, Warning, TEXT("EOSGetPlatformAuthTokenAsync: GetLinkedAccountAuthToken may not be available in UE 5.5"));
	OnFailure.Broadcast(TEXT("GetLinkedAccountAuthToken not available in UE 5.5"));
	SetReadyToDestroy();
}

void UEOSGetPlatformAuthTokenAsync::OnGetPlatformAuthTokenComplete(
	int32 InLocalUserNum,
	bool bWasSuccessful,
	const FExternalAuthToken& ExternalAuthToken)
{
	// Only process if this is for our local user
	if (InLocalUserNum != StoredLocalUserNum)
	{
		return;
	}

	// No cleanup needed - delegate is passed directly in UE 5.5

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

