// Copyright (c) 2023 Betide Studio. All Rights Reserved.


#include "EOK_AcceptFriendInvite_AsyncFunction.h"
#include "OnlineSubsystemUtils.h"

UEOK_AcceptFriendInvite_AsyncFunction* UEOK_AcceptFriendInvite_AsyncFunction::AcceptFriendInvite(
	FEOKUniqueNetId FriendId)
{
	UEOK_AcceptFriendInvite_AsyncFunction* UEOK_AcceptFriendInviteObject= NewObject<UEOK_AcceptFriendInvite_AsyncFunction>();
	UEOK_AcceptFriendInviteObject->Var_FriendId = FriendId;
	return UEOK_AcceptFriendInviteObject;
}

void UEOK_AcceptFriendInvite_AsyncFunction::OnAcceptComplete(int32 LocalUserNum, bool bWasSuccessful,
	const FUniqueNetId& FriendId, const FString& ListName, const FString& ErrorStr)
{
	if(bWasSuccessful)
	{
		OnSuccess.Broadcast("");
		SetReadyToDestroy();
#if ENGINE_MAJOR_VERSION == 5
		MarkAsGarbage();
#else
		MarkPendingKill();
#endif
	}
	else
	{
		OnFailure.Broadcast(ErrorStr);
		SetReadyToDestroy();
#if ENGINE_MAJOR_VERSION == 5
		MarkAsGarbage();
#else
		MarkPendingKill();
#endif
	}
}

void UEOK_AcceptFriendInvite_AsyncFunction::Activate()
{
	Super::Activate();
	if(const IOnlineSubsystem *SubsystemRef = Online::GetSubsystem(GetWorld()))
	{
		if(const IOnlineFriendsPtr FriendsInterface = SubsystemRef->GetFriendsInterface())
		{
			FriendsInterface->AcceptInvite(0,*Var_FriendId.GetUniqueNetId(),"",FOnAcceptInviteComplete::CreateUObject(this, &UEOK_AcceptFriendInvite_AsyncFunction::OnAcceptComplete));
			SetReadyToDestroy();
#if ENGINE_MAJOR_VERSION == 5
			MarkAsGarbage();
#else
			MarkPendingKill();
#endif
		}
		else
		{
			// Failed to get Friends Interface
			SetReadyToDestroy();
#if ENGINE_MAJOR_VERSION == 5
			MarkAsGarbage();
#else
			MarkPendingKill();
#endif
			OnFailure.Broadcast("Failed to get Friends Interface");
		}
	}
	else
	{
		// Failed to get Subsystem
		SetReadyToDestroy();
#if ENGINE_MAJOR_VERSION == 5
		MarkAsGarbage();
#else
		MarkPendingKill();
#endif
		OnFailure.Broadcast("Failed to get Subsystem");
	}
}
