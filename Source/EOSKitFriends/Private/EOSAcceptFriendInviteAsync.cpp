// Copyright (C) 2024, All Rights Reserved.

#include "EOSAcceptFriendInviteAsync.h"
#include "EOSKitSubsystem.h"
#include "EOSKitOnlineHelpers.h"
#include "Kismet/GameplayStatics.h"
#if WITH_EOS_SDK
#include "Windows/AllowWindowsPlatformTypes.h"
#include "eos_sdk.h"
#include "eos_friends.h"
#include "eos_friends_types.h"
#include "Windows/HideWindowsPlatformTypes.h"
#endif

UEOSAcceptFriendInviteAsync* UEOSAcceptFriendInviteAsync::AcceptFriendInvite(UObject* WorldContextObject, const FEOSKitEpicAccountId& TargetEpicAccountId)
{
	UEOSAcceptFriendInviteAsync* Node = NewObject<UEOSAcceptFriendInviteAsync>();
	Node->WorldContextObject = WorldContextObject;
	Node->TargetEpicAccountId = TargetEpicAccountId;
	return Node;
}

void UEOSAcceptFriendInviteAsync::Activate()
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
	FString EpicAccountIdString = FEOSKitOnlineHelpers::GetEpicAccountId(0);
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
	EOS_Friends_AcceptInviteOptions Options = {};
	Options.ApiVersion = EOS_FRIENDS_ACCEPTINVITE_API_LATEST;
	Options.LocalUserId = LocalUserId;
	Options.TargetUserId = TargetUserId;

	// Accept invite
	EOS_Friends_AcceptInvite(FriendsHandle, &Options, this, &UEOSAcceptFriendInviteAsync::OnAcceptInviteComplete);
}

void EOS_CALL UEOSAcceptFriendInviteAsync::OnAcceptInviteComplete(const EOS_Friends_AcceptInviteCallbackInfo* Data)
{
	if (!Data || !Data->ClientData)
	{
		return;
	}

	UEOSAcceptFriendInviteAsync* Self = static_cast<UEOSAcceptFriendInviteAsync*>(Data->ClientData);

	if (Data->ResultCode == EOS_EResult::EOS_Success)
	{
		Self->OnSuccess.Broadcast(TEXT(""));
	}
	else
	{
		FString ErrorMessage = FString::Printf(TEXT("Accept Invite Failed: %s"), 
			UTF8_TO_TCHAR(EOS_EResult_ToString(Data->ResultCode)));
		Self->OnFailure.Broadcast(ErrorMessage);
	}

	Self->SetReadyToDestroy();
}

