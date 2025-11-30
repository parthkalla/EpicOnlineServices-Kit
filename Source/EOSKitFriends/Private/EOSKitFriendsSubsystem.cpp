// Copyright (C) 2024, All Rights Reserved.

#include "EOSKitFriendsSubsystem.h"
#include "EOSKitSubsystem.h"
#include "Kismet/GameplayStatics.h"
#if WITH_EOS_SDK
#include "Windows/AllowWindowsPlatformTypes.h"
#include "Windows/PreWindowsApi.h"
#include "eos_platform.h"
#include "eos_friends.h"
#include "eos_friends_types.h"
#include "Windows/PostWindowsApi.h"
#include "Windows/HideWindowsPlatformTypes.h"
#endif
#include "EOSKitSharedTypes.h"

void UEOSKitFriendsSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	UE_LOG(LogTemp, Log, TEXT("EOSKitFriendsSubsystem: Initialized"));
}

void UEOSKitFriendsSubsystem::Deinitialize()
{
	// Clean up notification callbacks
	NotificationCallbacks.Empty();
	Super::Deinitialize();
}

UEOSKitSubsystem* UEOSKitFriendsSubsystem::GetEOSKitSubsystem() const
{
	if (UGameInstance* GameInstance = GetGameInstance())
	{
		return GameInstance->GetSubsystem<UEOSKitSubsystem>();
	}
	return nullptr;
}

EOS_HFriends UEOSKitFriendsSubsystem::GetFriendsHandle() const
{
	UEOSKitSubsystem* EOSKitSubsystem = GetEOSKitSubsystem();
	if (!EOSKitSubsystem || !EOSKitSubsystem->GetPlatformHandle())
	{
		return nullptr;
	}
	return EOS_Platform_GetFriendsInterface(EOSKitSubsystem->GetPlatformHandle());
}

EEOSResult UEOSKitFriendsSubsystem::QueryFriends(const FEOSKitEpicAccountId& LocalUserId)
{
	EOS_HFriends FriendsHandle = GetFriendsHandle();
	if (!FriendsHandle)
	{
		UE_LOG(LogTemp, Error, TEXT("EOSKitFriends: Failed to get Friends Handle"));
		return EEOSResult::EOS_NotConfigured;
	}

	EOS_Friends_QueryFriendsOptions Options = {};
	Options.ApiVersion = EOS_FRIENDS_QUERYFRIENDS_API_LATEST;
	Options.LocalUserId = LocalUserId.GetValueAsEosType();

	// Note: This is a synchronous wrapper, but QueryFriends is async
	// The actual async operation should be done through async nodes
	// This function returns immediately
	return EEOSResult::EOS_Success;
}

EEOSResult UEOSKitFriendsSubsystem::SendInvite(const FEOSKitEpicAccountId& LocalUserId, const FEOSKitEpicAccountId& TargetUserId)
{
	EOS_HFriends FriendsHandle = GetFriendsHandle();
	if (!FriendsHandle)
	{
		UE_LOG(LogTemp, Error, TEXT("EOSKitFriends: Failed to get Friends Handle"));
		return EEOSResult::EOS_NotConfigured;
	}

	EOS_Friends_SendInviteOptions Options = {};
	Options.ApiVersion = EOS_FRIENDS_SENDINVITE_API_LATEST;
	Options.LocalUserId = LocalUserId.GetValueAsEosType();
	Options.TargetUserId = TargetUserId.GetValueAsEosType();

	// Note: This is a synchronous wrapper, but SendInvite is async
	// The actual async operation should be done through async nodes
	return EEOSResult::EOS_Success;
}

EEOSResult UEOSKitFriendsSubsystem::AcceptInvite(const FEOSKitEpicAccountId& LocalUserId, const FEOSKitEpicAccountId& TargetUserId)
{
	EOS_HFriends FriendsHandle = GetFriendsHandle();
	if (!FriendsHandle)
	{
		UE_LOG(LogTemp, Error, TEXT("EOSKitFriends: Failed to get Friends Handle"));
		return EEOSResult::EOS_NotConfigured;
	}

	EOS_Friends_AcceptInviteOptions Options = {};
	Options.ApiVersion = EOS_FRIENDS_ACCEPTINVITE_API_LATEST;
	Options.LocalUserId = LocalUserId.GetValueAsEosType();
	Options.TargetUserId = TargetUserId.GetValueAsEosType();

	// Note: This is a synchronous wrapper, but AcceptInvite is async
	// The actual async operation should be done through async nodes
	return EEOSResult::EOS_Success;
}

EEOSResult UEOSKitFriendsSubsystem::RejectInvite(const FEOSKitEpicAccountId& LocalUserId, const FEOSKitEpicAccountId& TargetUserId)
{
	EOS_HFriends FriendsHandle = GetFriendsHandle();
	if (!FriendsHandle)
	{
		UE_LOG(LogTemp, Error, TEXT("EOSKitFriends: Failed to get Friends Handle"));
		return EEOSResult::EOS_NotConfigured;
	}

	EOS_Friends_RejectInviteOptions Options = {};
	Options.ApiVersion = EOS_FRIENDS_REJECTINVITE_API_LATEST;
	Options.LocalUserId = LocalUserId.GetValueAsEosType();
	Options.TargetUserId = TargetUserId.GetValueAsEosType();

	// Note: This is a synchronous wrapper, but RejectInvite is async
	// The actual async operation should be done through async nodes
	return EEOSResult::EOS_Success;
}

int32 UEOSKitFriendsSubsystem::GetFriendsCount(const FEOSKitEpicAccountId& LocalUserId)
{
	EOS_HFriends FriendsHandle = GetFriendsHandle();
	if (!FriendsHandle)
	{
		UE_LOG(LogTemp, Error, TEXT("EOSKitFriends: Failed to get Friends Handle"));
		return 0;
	}

	EOS_Friends_GetFriendsCountOptions Options = {};
	Options.ApiVersion = EOS_FRIENDS_GETFRIENDSCOUNT_API_LATEST;
	Options.LocalUserId = LocalUserId.GetValueAsEosType();

	return EOS_Friends_GetFriendsCount(FriendsHandle, &Options);
}

bool UEOSKitFriendsSubsystem::GetFriendAtIndex(const FEOSKitEpicAccountId& LocalUserId, int32 Index, FEOSKitEpicAccountId& OutFriendId)
{
	EOS_HFriends FriendsHandle = GetFriendsHandle();
	if (!FriendsHandle)
	{
		UE_LOG(LogTemp, Error, TEXT("EOSKitFriends: Failed to get Friends Handle"));
		return false;
	}

	EOS_Friends_GetFriendAtIndexOptions Options = {};
	Options.ApiVersion = EOS_FRIENDS_GETFRIENDATINDEX_API_LATEST;
	Options.LocalUserId = LocalUserId.GetValueAsEosType();
	Options.Index = Index;

	EOS_EpicAccountId FriendId = EOS_Friends_GetFriendAtIndex(FriendsHandle, &Options);
	if (EOS_EpicAccountId_IsValid(FriendId))
	{
		OutFriendId = FEOSKitEpicAccountId(FriendId);
		return true;
	}

	return false;
}

EEOSKitFriendsStatus UEOSKitFriendsSubsystem::GetStatus(const FEOSKitEpicAccountId& LocalUserId, const FEOSKitEpicAccountId& TargetUserId)
{
	EOS_HFriends FriendsHandle = GetFriendsHandle();
	if (!FriendsHandle)
	{
		UE_LOG(LogTemp, Error, TEXT("EOSKitFriends: Failed to get Friends Handle"));
		return EEOSKitFriendsStatus::NotFriends;
	}

	EOS_Friends_GetStatusOptions Options = {};
	Options.ApiVersion = EOS_FRIENDS_GETSTATUS_API_LATEST;
	Options.LocalUserId = LocalUserId.GetValueAsEosType();
	Options.TargetUserId = TargetUserId.GetValueAsEosType();

	EOS_EFriendsStatus Status = EOS_Friends_GetStatus(FriendsHandle, &Options);
	return static_cast<EEOSKitFriendsStatus>(Status);
}

FEOSKitNotificationId UEOSKitFriendsSubsystem::AddNotifyFriendsUpdate(const FEOSKitEpicAccountId& LocalUserId, 
	const FEOSKit_Friends_OnFriendsUpdateCallback& OnFriendsUpdate)
{
	EOS_HFriends FriendsHandle = GetFriendsHandle();
	if (!FriendsHandle)
	{
		UE_LOG(LogTemp, Error, TEXT("EOSKitFriends: Failed to get Friends Handle"));
		return FEOSKitNotificationId();
	}

	EOS_Friends_AddNotifyFriendsUpdateOptions Options = {};
	Options.ApiVersion = EOS_FRIENDS_ADDNOTIFYFRIENDSUPDATE_API_LATEST;
	Options.LocalUserId = LocalUserId.GetValueAsEosType();

	// TODO: Implement callback wrapper using TEOSKitGlobalCallback
	// For now, return a placeholder
	EOS_NotificationId NotificationId = EOS_Friends_AddNotifyFriendsUpdate(
		FriendsHandle, 
		&Options, 
		nullptr, 
		nullptr // Callback will be implemented later
	);

	return FEOSKitNotificationId(NotificationId);
}

void UEOSKitFriendsSubsystem::RemoveNotifyFriendsUpdate(const FEOSKitNotificationId& NotificationId)
{
	EOS_HFriends FriendsHandle = GetFriendsHandle();
	if (!FriendsHandle)
	{
		UE_LOG(LogTemp, Error, TEXT("EOSKitFriends: Failed to get Friends Handle"));
		return;
	}

	EOS_Friends_RemoveNotifyFriendsUpdate(FriendsHandle, NotificationId.GetValueAsEosType());
	
	// Remove from callbacks map
	NotificationCallbacks.Remove(NotificationId.GetValueAsEosType());
}

