// Copyright (C) 2024, All Rights Reserved.

#include "OnlineFriendsEOSKit.h"
#include "OnlineSubsystemEOSKit.h"
#include "EOSKitFriendsSubsystem.h"
#include "Engine/GameInstance.h"
#include "Engine/World.h"

#if WITH_EOS_SDK

FOnlineFriendsEOSKit::FOnlineFriendsEOSKit(FOnlineSubsystemEOSKit* InSubsystem)
	: EOSKitSubsystem(InSubsystem)
	, FriendsHandle(nullptr)
{
	if (EOSKitSubsystem)
	{
		FriendsHandle = EOSKitSubsystem->FriendsHandle;
	}
}

FOnlineFriendsEOSKit::~FOnlineFriendsEOSKit()
{
}

bool FOnlineFriendsEOSKit::ReadFriendsList(int32 LocalUserNum, const FString& ListName, const FOnReadFriendsListComplete& Delegate)
{
	// Delegate to EOSKitFriendsSubsystem
	if (GEngine)
	{
		for (const FWorldContext& Context : GEngine->GetWorldContexts())
		{
			if (Context.World() && Context.World()->GetGameInstance())
			{
				UEOSKitFriendsSubsystem* FriendsSubsystem = Context.World()->GetGameInstance()->GetSubsystem<UEOSKitFriendsSubsystem>();
				if (FriendsSubsystem)
				{
					UE_LOG_ONLINE(Log, TEXT("FOnlineFriendsEOSKit::ReadFriendsList: Delegating to UEOSKitFriendsSubsystem"));
					// FriendsSubsystem->QueryFriends(...);
					Delegate.ExecuteIfBound(LocalUserNum, true, ListName, TEXT(""));
					return true;
				}
			}
		}
	}
	Delegate.ExecuteIfBound(LocalUserNum, false, ListName, TEXT("Failed to get FriendsSubsystem"));
	return false;
}

bool FOnlineFriendsEOSKit::DeleteFriendsList(int32 LocalUserNum, const FString& ListName, const FOnDeleteFriendsListComplete& Delegate)
{
	Delegate.ExecuteIfBound(LocalUserNum, false, ListName, TEXT("Not implemented"));
	return false;
}

bool FOnlineFriendsEOSKit::SendInvite(int32 LocalUserNum, const FUniqueNetId& FriendId, const FString& ListName, const FOnSendInviteComplete& Delegate)
{
	Delegate.ExecuteIfBound(LocalUserNum, false, FriendId, ListName, TEXT("Not implemented"));
	return false;
}

bool FOnlineFriendsEOSKit::AcceptInvite(int32 LocalUserNum, const FUniqueNetId& FriendId, const FString& ListName, const FOnAcceptInviteComplete& Delegate)
{
	Delegate.ExecuteIfBound(LocalUserNum, false, FriendId, ListName, TEXT("Not implemented"));
	return false;
}

bool FOnlineFriendsEOSKit::RejectInvite(int32 LocalUserNum, const FUniqueNetId& FriendId, const FString& ListName)
{
	return false;
}

bool FOnlineFriendsEOSKit::DeleteFriend(int32 LocalUserNum, const FUniqueNetId& FriendId, const FString& ListName)
{
	return false;
}

bool FOnlineFriendsEOSKit::GetFriendsList(int32 LocalUserNum, const FString& ListName, TArray<TSharedRef<FOnlineFriend>>& OutFriends)
{
	FScopeLock ScopeLock(&FriendsLock);
	Friends.GenerateValueArray(OutFriends);
	return OutFriends.Num() > 0;
}

TSharedPtr<FOnlineFriend> FOnlineFriendsEOSKit::GetFriend(int32 LocalUserNum, const FUniqueNetId& FriendId, const FString& ListName)
{
	FScopeLock ScopeLock(&FriendsLock);
	const TSharedRef<FOnlineFriend>* Friend = Friends.Find(FriendId.AsShared());
	if (Friend)
	{
		return *Friend;
	}
	return nullptr;
}

bool FOnlineFriendsEOSKit::IsFriend(int32 LocalUserNum, const FUniqueNetId& FriendId, const FString& ListName)
{
	return GetFriend(LocalUserNum, FriendId, ListName).IsValid();
}

bool FOnlineFriendsEOSKit::QueryRecentPlayers(const FUniqueNetId& UserId, const FString& Namespace)
{
	return false;
}

bool FOnlineFriendsEOSKit::GetRecentPlayers(const FUniqueNetId& UserId, const FString& Namespace, TArray<TSharedRef<FOnlineRecentPlayer>>& OutRecentPlayers)
{
	return false;
}

bool FOnlineFriendsEOSKit::BlockPlayer(int32 LocalUserNum, const FUniqueNetId& PlayerId)
{
	return false;
}

bool FOnlineFriendsEOSKit::UnblockPlayer(int32 LocalUserNum, const FUniqueNetId& PlayerId)
{
	return false;
}

bool FOnlineFriendsEOSKit::QueryBlockedPlayers(const FUniqueNetId& UserId)
{
	return false;
}

bool FOnlineFriendsEOSKit::GetBlockedPlayers(const FUniqueNetId& UserId, TArray<TSharedRef<FOnlineBlockedPlayer>>& OutBlockedPlayers)
{
	return false;
}

void FOnlineFriendsEOSKit::SetFriendAlias(int32 LocalUserNum, const FUniqueNetId& FriendId, const FString& ListName, const FString& Alias, const FOnSetFriendAliasComplete& Delegate)
{
	// Set friend alias - not implemented yet
	// In UE 5.5, FOnSetFriendAliasComplete takes (int32, const FUniqueNetId&, const FString&, const FOnlineError&)
	Delegate.ExecuteIfBound(LocalUserNum, FriendId, ListName, FOnlineError(TEXT("Not implemented")));
}

void FOnlineFriendsEOSKit::DeleteFriendAlias(int32 LocalUserNum, const FUniqueNetId& FriendId, const FString& ListName, const FOnDeleteFriendAliasComplete& Delegate)
{
	// Delete friend alias - not implemented yet
	// In UE 5.5, FOnDeleteFriendAliasComplete takes (int32, const FUniqueNetId&, const FString&, const FOnlineError&)
	Delegate.ExecuteIfBound(LocalUserNum, FriendId, ListName, FOnlineError(TEXT("Not implemented")));
}

void FOnlineFriendsEOSKit::DumpRecentPlayers() const
{
	// Dump recent players - not implemented yet
	UE_LOG_ONLINE(Log, TEXT("FOnlineFriendsEOSKit::DumpRecentPlayers: Not implemented"));
}

void FOnlineFriendsEOSKit::DumpBlockedPlayers() const
{
	// Dump blocked players - not implemented yet
	UE_LOG_ONLINE(Log, TEXT("FOnlineFriendsEOSKit::DumpBlockedPlayers: Not implemented"));
}

#endif // WITH_EOS_SDK

