// Copyright (C) 2024, All Rights Reserved.

#include "EOSInviteFriendAsync.h"
#include "EOSKitSubsystem.h"
#include "Kismet/GameplayStatics.h"
#include "eos_friends.h"
#include "eos_friends_types.h"

UEOSInviteFriendAsync* UEOSInviteFriendAsync::InviteFriend(UObject* WorldContextObject, const FEOSKitEpicAccountId& TargetEpicAccountId)
{
	UEOSInviteFriendAsync* Node = NewObject<UEOSInviteFriendAsync>();
	Node->WorldContextObject = WorldContextObject;
	Node->TargetEpicAccountId = TargetEpicAccountId;
	return Node;
}

void UEOSInviteFriendAsync::Activate()
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
	FString EpicAccountIdString = EOSSubsystem->GetEpicAccountIdString();
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
	EOS_Friends_SendInviteOptions Options = {};
	Options.ApiVersion = EOS_FRIENDS_SENDINVITE_API_LATEST;
	Options.LocalUserId = LocalUserId;
	Options.TargetUserId = TargetUserId;

	// Send invite
	EOS_Friends_SendInvite(FriendsHandle, &Options, this, &UEOSInviteFriendAsync::OnSendInviteComplete);
}

void EOS_CALL UEOSInviteFriendAsync::OnSendInviteComplete(const EOS_Friends_SendInviteCallbackInfo* Data)
{
	if (!Data || !Data->ClientData)
	{
		return;
	}

	UEOSInviteFriendAsync* Self = static_cast<UEOSInviteFriendAsync*>(Data->ClientData);

	if (Data->ResultCode == EOS_EResult::EOS_Success)
	{
		Self->OnSuccess.Broadcast(TEXT(""));
	}
	else
	{
		FString ErrorMessage = FString::Printf(TEXT("Send Invite Failed: %s"), 
			UTF8_TO_TCHAR(EOS_EResult_ToString(Data->ResultCode)));
		Self->OnFailure.Broadcast(ErrorMessage);
	}

	Self->SetReadyToDestroy();
}

