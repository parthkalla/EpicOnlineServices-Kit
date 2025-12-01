// Copyright (C) 2024, All Rights Reserved.

#include "EOSRejectFriendInviteAsync.h"
#include "EOSKitSubsystem.h"
#include "EOSKitGameInstanceSubsystem.h"
#include "Kismet/GameplayStatics.h"
#if WITH_EOS_SDK
#include "Windows/AllowWindowsPlatformTypes.h"
#include "eos_sdk.h"
#include "eos_friends.h"
#include "eos_friends_types.h"
#include "Windows/HideWindowsPlatformTypes.h"
#endif

UEOSRejectFriendInviteAsync* UEOSRejectFriendInviteAsync::RejectFriendInvite(UObject* WorldContextObject, const FEOSKitEpicAccountId& TargetEpicAccountId)
{
	UEOSRejectFriendInviteAsync* Node = NewObject<UEOSRejectFriendInviteAsync>();
	Node->WorldContextObject = WorldContextObject;
	Node->TargetEpicAccountId = TargetEpicAccountId;
	return Node;
}

void UEOSRejectFriendInviteAsync::Activate()
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

	// Get Epic Account ID from subsystem
	FString EpicAccountIdString = UEOSKitGameInstanceSubsystem::GetEpicAccountId(0);
	if (EpicAccountIdString.IsEmpty())
	{
		OnFailure.Broadcast(TEXT("User not logged in"));
		SetReadyToDestroy();
		return;
	}

	EOS_HFriends FriendsHandle = EOS_Platform_GetFriendsInterface(EOSSubsystem->GetPlatformHandle());
	if (!FriendsHandle)
	{
		OnFailure.Broadcast(TEXT("Failed to get Friends Interface"));
		SetReadyToDestroy();
		return;
	}

	// Convert Epic Account IDs
	EOS_EpicAccountId LocalUserId = EOS_EpicAccountId_FromString(TCHAR_TO_UTF8(*EpicAccountIdString));
	EOS_EpicAccountId TargetUserId = TargetEpicAccountId.GetValueAsEosType();

	if (!EOS_EpicAccountId_IsValid(LocalUserId) || !EOS_EpicAccountId_IsValid(TargetUserId))
	{
		OnFailure.Broadcast(TEXT("Invalid Epic Account ID"));
		SetReadyToDestroy();
		return;
	}

	// Setup options
	EOS_Friends_RejectInviteOptions Options = {};
	Options.ApiVersion = EOS_FRIENDS_REJECTINVITE_API_LATEST;
	Options.LocalUserId = LocalUserId;
	Options.TargetUserId = TargetUserId;

	// Reject invite
	EOS_Friends_RejectInvite(FriendsHandle, &Options, this, &UEOSRejectFriendInviteAsync::OnRejectInviteComplete);
}

void EOS_CALL UEOSRejectFriendInviteAsync::OnRejectInviteComplete(const EOS_Friends_RejectInviteCallbackInfo* Data)
{
	if (!Data || !Data->ClientData)
	{
		return;
	}

	UEOSRejectFriendInviteAsync* Self = static_cast<UEOSRejectFriendInviteAsync*>(Data->ClientData);

	if (Data->ResultCode == EOS_EResult::EOS_Success)
	{
		Self->OnSuccess.Broadcast(TEXT(""));
	}
	else
	{
		FString ErrorMessage = FString::Printf(TEXT("Reject Invite Failed: %s"), 
			UTF8_TO_TCHAR(EOS_EResult_ToString(Data->ResultCode)));
		Self->OnFailure.Broadcast(ErrorMessage);
	}

	Self->SetReadyToDestroy();
}

