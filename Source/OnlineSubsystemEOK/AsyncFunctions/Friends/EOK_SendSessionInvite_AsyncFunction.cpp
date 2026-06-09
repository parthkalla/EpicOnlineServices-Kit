// Copyright (c) 2023 Betide Studio. All Rights Reserved.


#include "EOK_SendSessionInvite_AsyncFunction.h"
#include "OnlineSubsystemUtils.h"

UEOK_SendSessionInvite_AsyncFunction* UEOK_SendSessionInvite_AsyncFunction::SendSessionInvite(FName SessionName,FEOKUniqueNetId FriendId)
{
	UEOK_SendSessionInvite_AsyncFunction* BlueprintNode = NewObject<UEOK_SendSessionInvite_AsyncFunction>();
	BlueprintNode->Var_FriendId = FriendId;
	BlueprintNode->Var_SessionName = SessionName;
	return BlueprintNode;
}

void UEOK_SendSessionInvite_AsyncFunction::Activate()
{
	Super::Activate();
	if(const IOnlineSubsystem *SubsystemRef = Online::GetSubsystem(GetWorld()))
	{
		if(const IOnlineSessionPtr SessionInterface = SubsystemRef->GetSessionInterface())
		{
			if(SessionInterface->SendSessionInviteToFriend(0,Var_SessionName,*Var_FriendId.GetUniqueNetId()))
			{
				SetReadyToDestroy();
#if ENGINE_MAJOR_VERSION == 5
				MarkAsGarbage();
#else
				MarkPendingKill();
#endif
				OnSuccess.Broadcast("");
			}
			else
			{
				// Failed to send session invite
				SetReadyToDestroy();
#if ENGINE_MAJOR_VERSION == 5
				MarkAsGarbage();
#else
				MarkPendingKill();
#endif
				OnFailure.Broadcast("Failed to send session invite");
			}
		}
		else
		{
			// Failed to get Session Interface
			SetReadyToDestroy();
#if ENGINE_MAJOR_VERSION == 5
			MarkAsGarbage();
#else
			MarkPendingKill();
#endif
			OnFailure.Broadcast("Failed to get Session Interface");
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
