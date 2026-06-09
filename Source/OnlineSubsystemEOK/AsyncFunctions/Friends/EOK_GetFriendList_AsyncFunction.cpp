// Copyright (c) 2023 Betide Studio. All Rights Reserved.


#include "EOK_GetFriendList_AsyncFunction.h"
#include "OnlineSubsystemUtils.h"
#include "OnlineSubsystem.h"
#include "Interfaces/OnlinePresenceInterface.h"

UEOK_GetFriendList_AsyncFunction* UEOK_GetFriendList_AsyncFunction::GetFriendList()
{
	UEOK_GetFriendList_AsyncFunction* UEOK_GetFriendListObject= NewObject<UEOK_GetFriendList_AsyncFunction>();
	return UEOK_GetFriendListObject;
}

void UEOK_GetFriendList_AsyncFunction::OnFriendsReadComplete(int32 LocalUserNum, bool bWasSuccessful,
	const FString& ListName, const FString& ErrorStr)
{
	if(bWasSuccessful)
	{
		TArray<FEOK_FriendData> FriendList = {};
		if(const IOnlineSubsystem *SubsystemRef = Online::GetSubsystem(GetWorld()))
		{
			if(const IOnlineFriendsPtr FriendsInterface = SubsystemRef->GetFriendsInterface())
			{
				TArray<TSharedRef<FOnlineFriend>> Friends;
				FriendsInterface->GetFriendsList(LocalUserNum, ListName, Friends);
				for(const TSharedRef<FOnlineFriend>& Friend : Friends)
				{
					FEOK_FriendData FriendData;
					FriendData.DisplayName = Friend->GetDisplayName();
					FriendData.bIsOnline = Friend->GetPresence().bIsOnline;
					FriendData.InviteStatus = ToString(Friend->GetInviteStatus());
					FEOKUniqueNetId FriendId;
					FriendId.SetUniqueNetId(Friend->GetUserId());
					FriendData.UserId = FriendId;
					FriendList.Add(FriendData);
				}
				OnSuccess.Broadcast(FriendList, "");
				SetReadyToDestroy();
#if ENGINE_MAJOR_VERSION == 5
				MarkAsGarbage();
#else
				MarkPendingKill();
#endif		
			}
			else
			{
				OnFailure.Broadcast(TArray<FEOK_FriendData>(), "Failed to get Friends Interface");
				SetReadyToDestroy();
#if ENGINE_MAJOR_VERSION == 5
				MarkAsGarbage();
#else
				MarkPendingKill();
#endif
			}
		}
		else
		{
			OnFailure.Broadcast(TArray<FEOK_FriendData>(), "Failed to get Subsystem");
			SetReadyToDestroy();
#if ENGINE_MAJOR_VERSION == 5
			MarkAsGarbage();
#else
			MarkPendingKill();
#endif
		}
	}
	else
	{
		OnFailure.Broadcast(TArray<FEOK_FriendData>(), ErrorStr);
		SetReadyToDestroy();
#if ENGINE_MAJOR_VERSION == 5
		MarkAsGarbage();
#else
		MarkPendingKill();
#endif
	}
	}

void UEOK_GetFriendList_AsyncFunction::Activate()
{
	Super::Activate();
	if(const IOnlineSubsystem *SubsystemRef = Online::GetSubsystem(GetWorld()))
	{
		if(const IOnlineFriendsPtr FriendsInterface = SubsystemRef->GetFriendsInterface())
		{
			FriendsInterface->ReadFriendsList(0, "",FOnReadFriendsListComplete::CreateUObject(this, &UEOK_GetFriendList_AsyncFunction::OnFriendsReadComplete));
		}
		else
		{
			OnFailure.Broadcast(TArray<FEOK_FriendData>(), "Failed to get Friends Interface");
			SetReadyToDestroy();
#if ENGINE_MAJOR_VERSION == 5
			MarkAsGarbage();
#else
			MarkPendingKill();
#endif
		}
	}
	else
	{
		OnFailure.Broadcast(TArray<FEOK_FriendData>(), "Failed to get Subsystem");
		SetReadyToDestroy();
#if ENGINE_MAJOR_VERSION == 5
		MarkAsGarbage();
#else
		MarkPendingKill();
#endif
	}
}
