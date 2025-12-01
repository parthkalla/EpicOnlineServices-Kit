// Copyright (C) 2024, All Rights Reserved.

#include "EOSSendSessionInviteAsync.h"
#include "EOSKitSubsystem.h"
#include "EOSKitLoginHelpers.h"
#include "EOS_SessionsSubsystem.h"
#include "Kismet/GameplayStatics.h"
#include "eos_sessions.h"
#include "eos_sessions_types.h"

UEOSSendSessionInviteAsync* UEOSSendSessionInviteAsync::SendSessionInvite(UObject* WorldContextObject, 
	const FEOSKitEpicAccountId& TargetEpicAccountId, 
	const FString& SessionName)
{
	UEOSSendSessionInviteAsync* Node = NewObject<UEOSSendSessionInviteAsync>();
	Node->WorldContextObject = WorldContextObject;
	Node->TargetEpicAccountId = TargetEpicAccountId;
	Node->SessionNameString = SessionName;
	return Node;
}

void UEOSSendSessionInviteAsync::Activate()
{
	if (!WorldContextObject)
	{
		OnFailure.Broadcast(TEXT("Invalid World Context"));
		SetReadyToDestroy();
		return;
	}

	UGameInstance* GameInstance = UGameplayStatics::GetGameInstance(WorldContextObject);
	if (!GameInstance)
	{
		OnFailure.Broadcast(TEXT("Invalid Game Instance"));
		SetReadyToDestroy();
		return;
	}

	UEOSKitSubsystem* EOSSubsystem = GameInstance->GetSubsystem<UEOSKitSubsystem>();
	if (!EOSSubsystem || !EOSSubsystem->GetPlatformHandle())
	{
		OnFailure.Broadcast(TEXT("EOS Platform not initialized"));
		SetReadyToDestroy();
		return;
	}

	// Get Product User ID from Epic Account ID
	// Note: This requires converting Epic Account ID to Product User ID
	// For now, we'll use the Sessions subsystem's SendInvite function
	// which accepts Product User IDs
	
	UEOS_SessionsSubsystem* SessionsSubsystem = GameInstance->GetSubsystem<UEOS_SessionsSubsystem>();
	if (!SessionsSubsystem)
	{
		OnFailure.Broadcast(TEXT("Sessions Subsystem not available"));
		SetReadyToDestroy();
		return;
	}

	// Get local user Product User ID
	FString ProductUserIdString = UEOSKitLoginHelpers::GetProductUserIdString(WorldContextObject);
	if (ProductUserIdString.IsEmpty())
	{
		OnFailure.Broadcast(TEXT("User not logged in"));
		SetReadyToDestroy();
		return;
	}

#if WITH_EOS_SDK
	EOS_ProductUserId LocalPUID = EOS_ProductUserId_FromString(TCHAR_TO_UTF8(*ProductUserIdString));
	FEOSKitProductUserId LocalUserId(LocalPUID);
#else
	FEOSKitProductUserId LocalUserId(ProductUserIdString);
#endif

	// TODO: Convert Epic Account ID to Product User ID
	// For now, we'll need to get the Product User ID from the friend's Epic Account ID
	// This requires using the Connect interface to query external account mappings
	// For simplicity, this implementation assumes the caller provides the Product User ID
	
	// Note: This is a simplified implementation
	// In a full implementation, you would:
	// 1. Use EOS_Connect_QueryExternalAccountMappings to get Product User ID from Epic Account ID
	// 2. Then use the Sessions subsystem to send the invite
	
	OnFailure.Broadcast(TEXT("SendSessionInvite to friend requires Product User ID conversion - use Sessions SendInvite directly with Product User ID"));
	SetReadyToDestroy();
}

