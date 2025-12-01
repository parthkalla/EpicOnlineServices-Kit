// Copyright (C) 2024, All Rights Reserved.

#include "EOSGetAppleAuthTokenAsync.h"
#include "OnlineSubsystem.h"
#include "Interfaces/OnlineIdentityInterface.h"
#include "Engine/World.h"

UEOSGetAppleAuthTokenAsync* UEOSGetAppleAuthTokenAsync::GetAppleAuthToken(
	UObject* WorldContextObject,
	int32 LocalUserNum)
{
	UEOSGetAppleAuthTokenAsync* BlueprintNode = NewObject<UEOSGetAppleAuthTokenAsync>();
	BlueprintNode->WorldContextObject = WorldContextObject;
	BlueprintNode->LocalUserNum = LocalUserNum;
	return BlueprintNode;
}

void UEOSGetAppleAuthTokenAsync::Activate()
{
	Super::Activate();

	if (!WorldContextObject)
	{
		UE_LOG(LogTemp, Error, TEXT("EOSGetAppleAuthTokenAsync: WorldContextObject is null"));
		OnFailure.Broadcast(TEXT(""));
		SetReadyToDestroy();
		return;
	}

	// Get the Apple OnlineSubsystem
	IOnlineSubsystem* AppleOSS = IOnlineSubsystem::Get(TEXT("Apple"));
	if (!AppleOSS)
	{
		UE_LOG(LogTemp, Error, TEXT("EOSGetAppleAuthTokenAsync: Failed to get Apple OnlineSubsystem"));
		OnFailure.Broadcast(TEXT(""));
		SetReadyToDestroy();
		return;
	}

	IOnlineIdentityPtr AppleIdentity = AppleOSS->GetIdentityInterface();
	if (!AppleIdentity.IsValid())
	{
		UE_LOG(LogTemp, Error, TEXT("EOSGetAppleAuthTokenAsync: Failed to get Apple Identity interface"));
		OnFailure.Broadcast(TEXT(""));
		SetReadyToDestroy();
		return;
	}

	// Check if auth token already exists
	FString AuthToken = AppleIdentity->GetAuthToken(LocalUserNum);
	if (!AuthToken.IsEmpty())
	{
		UE_LOG(LogTemp, Log, TEXT("EOSGetAppleAuthTokenAsync: Apple auth token found"));
		OnSuccess.Broadcast(AuthToken);
		SetReadyToDestroy();
		return;
	}

	// No token found, attempt to login first
	UE_LOG(LogTemp, Log, TEXT("EOSGetAppleAuthTokenAsync: No Apple auth token found, attempting to login"));

	// Add login complete delegate
	LoginDelegateHandle = AppleIdentity->AddOnLoginCompleteDelegate_Handle(
		LocalUserNum,
		FOnLoginCompleteDelegate::CreateUObject(
			this,
			&UEOSGetAppleAuthTokenAsync::OnLoginComplete
		)
	);

	// Attempt auto-login
	AppleIdentity->AutoLogin(LocalUserNum);
}

void UEOSGetAppleAuthTokenAsync::OnLoginComplete(
	int32 LocalUserNum1,
	bool bWasSuccessful,
	const FUniqueNetId& UniqueNetId,
	const FString& Error)
{
	// Remove the delegate
	if (IOnlineSubsystem* AppleOSS = IOnlineSubsystem::Get(TEXT("Apple")))
	{
		if (IOnlineIdentityPtr AppleIdentity = AppleOSS->GetIdentityInterface())
		{
			AppleIdentity->ClearOnLoginCompleteDelegate_Handle(LocalUserNum1, LoginDelegateHandle);
		}
	}

	if (bWasSuccessful)
	{
		// Try to get the auth token again
		if (IOnlineSubsystem* AppleOSS = IOnlineSubsystem::Get(TEXT("Apple")))
		{
			if (IOnlineIdentityPtr AppleIdentity = AppleOSS->GetIdentityInterface())
			{
				FString AuthToken = AppleIdentity->GetAuthToken(LocalUserNum1);
				if (!AuthToken.IsEmpty())
				{
					UE_LOG(LogTemp, Log, TEXT("EOSGetAppleAuthTokenAsync: Apple auth token retrieved after login"));
					OnSuccess.Broadcast(AuthToken);
					SetReadyToDestroy();
					return;
				}
			}
		}
	}

	UE_LOG(LogTemp, Error, TEXT("EOSGetAppleAuthTokenAsync: Failed to get Apple auth token after login: %s"), *Error);
	OnFailure.Broadcast(TEXT(""));
	SetReadyToDestroy();
}

