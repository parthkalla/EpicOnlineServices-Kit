// Copyright (c) 2023 Betide Studio. All Rights Reserved.


#include "EOK_FriendsSubsystem.h"

#include "OnlineSubsystemEOS.h"
#include "Async/Async.h"
#include "OnlineSubsystemEOK/SdkFunctions/ConnectInterface/EOK_ConnectSubsystem.h"

void UEOK_FriendsSubsystem::OnBlockedUserUpdateCallback(const EOS_Friends_OnBlockedUsersUpdateInfo* Data)
{
	if(UEOK_FriendsSubsystem* Subsystem = static_cast<UEOK_FriendsSubsystem*>(Data->ClientData))
	{
		{
			AsyncTask(ENamedThreads::GameThread, [Data, Subsystem]()
			{
				bool LocalData = false;
				if (Data->bBlocked == EOS_TRUE)
				{
					LocalData = true;
				}
				Subsystem->OnBlockedUserUpdate.ExecuteIfBound(Data->LocalUserId, Data->TargetUserId, LocalData);
			});
		}
	}
}

FEOK_NotificationId UEOK_FriendsSubsystem::EOK_Friends_AddNotifyBlockedUsersUpdate(FEOK_Friends_OnBlockedUsersUpdateCallback Callback)
{
	if (IOnlineSubsystem* OnlineSub = IOnlineSubsystem::Get("EOK"))
	{
		if (FOnlineSubsystemEOS* EOSRef = static_cast<FOnlineSubsystemEOS*>(OnlineSub))
		{
			OnBlockedUserUpdate = Callback;
			EOS_Friends_AddNotifyBlockedUsersUpdateOptions Options = {};
			Options.ApiVersion = EOS_FRIENDS_ADDNOTIFYBLOCKEDUSERSUPDATE_API_LATEST;
			return EOS_Friends_AddNotifyBlockedUsersUpdate(EOSRef->FriendsHandle, &Options, this, &UEOK_FriendsSubsystem::OnBlockedUserUpdateCallback);
		}
	}
	UE_LOG(LogEOK, Error, TEXT("Failed to add notify blocked users update either OnlineSubsystem is not valid or EOSRef is not valid."));
	return FEOK_NotificationId();
}

void UEOK_FriendsSubsystem::OnFriendsUpdateCallback(const EOS_Friends_OnFriendsUpdateInfo* Data)
{
	if(UEOK_FriendsSubsystem* Subsystem = static_cast<UEOK_FriendsSubsystem*>(Data->ClientData))
	{
		{
			AsyncTask(ENamedThreads::GameThread, [Data, Subsystem]()
			{
				Subsystem->OnFriendsUpdate.ExecuteIfBound(Data->LocalUserId, Data->TargetUserId, static_cast<EEOK_EFriendsStatus>(Data->PreviousStatus), static_cast<EEOK_EFriendsStatus>(Data->CurrentStatus));
			});
		}
	}
}

FEOK_NotificationId UEOK_FriendsSubsystem::EOK_Friends_AddNotifyFriendsUpdate(
	FEOK_Friends_OnFriendsUpdateCallback Callback)
{
	if (IOnlineSubsystem* OnlineSub = IOnlineSubsystem::Get("EOK"))
	{
		if (FOnlineSubsystemEOS* EOSRef = static_cast<FOnlineSubsystemEOS*>(OnlineSub))
		{
			OnFriendsUpdate = Callback;
			EOS_Friends_AddNotifyFriendsUpdateOptions Options = {};
			Options.ApiVersion = EOS_FRIENDS_ADDNOTIFYFRIENDSUPDATE_API_LATEST;
			return EOS_Friends_AddNotifyFriendsUpdate(EOSRef->FriendsHandle, &Options, this, &UEOK_FriendsSubsystem::OnFriendsUpdateCallback);
		}
	}
	UE_LOG(LogEOK, Error, TEXT("Failed to add notify friends update either OnlineSubsystem is not valid or EOSRef is not valid."));
	return FEOK_NotificationId();
}

FEOK_EpicAccountId UEOK_FriendsSubsystem::EOK_Friends_GetBlockedUserAtIndex(FEOK_EpicAccountId LocalUserId, int32 Index)
{
	if (IOnlineSubsystem* OnlineSub = IOnlineSubsystem::Get("EOK"))
	{
		if (FOnlineSubsystemEOS* EOSRef = static_cast<FOnlineSubsystemEOS*>(OnlineSub))
		{
			EOS_Friends_GetBlockedUserAtIndexOptions Options = {};
			Options.ApiVersion = EOS_FRIENDS_GETBLOCKEDUSERATINDEX_API_LATEST;
			Options.LocalUserId = LocalUserId.GetValueAsEosType();
			Options.Index = Index;
			return EOS_Friends_GetBlockedUserAtIndex(EOSRef->FriendsHandle, &Options);
		}
	}
	UE_LOG(LogEOK, Error, TEXT("Failed to get blocked user at index either OnlineSubsystem is not valid or EOSRef is not valid."));
	return FEOK_EpicAccountId();
}

int32 UEOK_FriendsSubsystem::EOK_Friends_GetBlockedUsersCount(FEOK_EpicAccountId LocalUserId)
{
	if (IOnlineSubsystem* OnlineSub = IOnlineSubsystem::Get("EOK"))
	{
		if (FOnlineSubsystemEOS* EOSRef = static_cast<FOnlineSubsystemEOS*>(OnlineSub))
		{
			EOS_Friends_GetBlockedUsersCountOptions Options = {};
			Options.ApiVersion = EOS_FRIENDS_GETBLOCKEDUSERSCOUNT_API_LATEST;
			Options.LocalUserId = LocalUserId.GetValueAsEosType();
			return EOS_Friends_GetBlockedUsersCount(EOSRef->FriendsHandle, &Options);
		}
	}
	UE_LOG(LogEOK, Error, TEXT("Failed to get blocked users count either OnlineSubsystem is not valid or EOSRef is not valid."));
	return 0;
}

FEOK_EpicAccountId UEOK_FriendsSubsystem::EOK_Friends_GetFriendAtIndex(FEOK_EpicAccountId LocalUserId, int32 Index)
{
	if (IOnlineSubsystem* OnlineSub = IOnlineSubsystem::Get("EOK"))
	{
		if (FOnlineSubsystemEOS* EOSRef = static_cast<FOnlineSubsystemEOS*>(OnlineSub))
		{
			EOS_Friends_GetFriendAtIndexOptions Options = {};
			Options.ApiVersion = EOS_FRIENDS_GETFRIENDATINDEX_API_LATEST;
			Options.LocalUserId = LocalUserId.GetValueAsEosType();
			Options.Index = Index;
			return EOS_Friends_GetFriendAtIndex(EOSRef->FriendsHandle, &Options);
		}
	}
	UE_LOG(LogEOK, Error, TEXT("Failed to get friend at index either OnlineSubsystem is not valid or EOSRef is not valid."));
	return FEOK_EpicAccountId();
}

int32 UEOK_FriendsSubsystem::EOK_Friends_GetFriendsCount(FEOK_EpicAccountId LocalUserId)
{
	if (IOnlineSubsystem* OnlineSub = IOnlineSubsystem::Get("EOK"))
	{
		if (FOnlineSubsystemEOS* EOSRef = static_cast<FOnlineSubsystemEOS*>(OnlineSub))
		{
			EOS_Friends_GetFriendsCountOptions Options = {};
			Options.ApiVersion = EOS_FRIENDS_GETFRIENDSCOUNT_API_LATEST;
			Options.LocalUserId = LocalUserId.GetValueAsEosType();
			return EOS_Friends_GetFriendsCount(EOSRef->FriendsHandle, &Options);
		}
	}
	UE_LOG(LogEOK, Error, TEXT("Failed to get friends count either OnlineSubsystem is not valid or EOSRef is not valid."));
	return 0;
}

TEnumAsByte<EEOK_EFriendsStatus> UEOK_FriendsSubsystem::EOK_Friends_GetStatus(FEOK_EpicAccountId LocalUserId,
	FEOK_EpicAccountId TargetUserId)
{
	if (IOnlineSubsystem* OnlineSub = IOnlineSubsystem::Get("EOK"))
	{
		if (FOnlineSubsystemEOS* EOSRef = static_cast<FOnlineSubsystemEOS*>(OnlineSub))
		{
			EOS_Friends_GetStatusOptions Options = {};
			Options.ApiVersion = EOS_FRIENDS_GETSTATUS_API_LATEST;
			Options.LocalUserId = LocalUserId.GetValueAsEosType();
			Options.TargetUserId = TargetUserId.GetValueAsEosType();
			return static_cast<EEOK_EFriendsStatus>(EOS_Friends_GetStatus(EOSRef->FriendsHandle, &Options));
		}
	}
	UE_LOG(LogEOK, Error, TEXT("Failed to get friends status either OnlineSubsystem is not valid or EOSRef is not valid."));
	return EEOK_EFriendsStatus::EOK_FS_NotFriends;
}

void UEOK_FriendsSubsystem::EOK_Friends_RemoveNotifyBlockedUsersUpdate(FEOK_NotificationId NotificationId)
{
	if (IOnlineSubsystem* OnlineSub = IOnlineSubsystem::Get("EOK"))
	{
		if (FOnlineSubsystemEOS* EOSRef = static_cast<FOnlineSubsystemEOS*>(OnlineSub))
		{
			EOS_Friends_RemoveNotifyBlockedUsersUpdate(EOSRef->FriendsHandle, NotificationId.NotificationId);
			return;
		}
	}
	UE_LOG(LogEOK, Error, TEXT("Failed to remove notify blocked users update either OnlineSubsystem is not valid or EOSRef is not valid."));
}

void UEOK_FriendsSubsystem::EOK_Friends_RemoveNotifyFriendsUpdate(FEOK_NotificationId NotificationId)
{
	if (IOnlineSubsystem* OnlineSub = IOnlineSubsystem::Get("EOK"))
	{
		if (FOnlineSubsystemEOS* EOSRef = static_cast<FOnlineSubsystemEOS*>(OnlineSub))
		{
			EOS_Friends_RemoveNotifyFriendsUpdate(EOSRef->FriendsHandle, NotificationId.NotificationId);
			return;
		}
	}
	UE_LOG(LogEOK, Error, TEXT("Failed to remove notify friends update either OnlineSubsystem is not valid or EOSRef is not valid."));
}
