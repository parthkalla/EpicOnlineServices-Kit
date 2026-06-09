// Copyright (c) 2023 Betide Studio. All Rights Reserved.


#include "EOK_Presence_QueryPresence.h"

#include "OnlineSubsystemEOS.h"
#include "Async/Async.h"
#include "OnlineSubsystemEOK/SdkFunctions/ConnectInterface/EOK_ConnectSubsystem.h"

UEOK_Presence_QueryPresence* UEOK_Presence_QueryPresence::EOK_Presence_QueryPresence(FEOK_EpicAccountId LocalUserId,
                                                                                     FEOK_EpicAccountId TargetUserId)
{
	UEOK_Presence_QueryPresence* PresenceQuery = NewObject<UEOK_Presence_QueryPresence>();
	PresenceQuery->Var_LocalUserId = LocalUserId;
	PresenceQuery->Var_TargetUserId = TargetUserId;
	return PresenceQuery;
}

void UEOK_Presence_QueryPresence::Activate()
{
	Super::Activate();
	if (IOnlineSubsystem* OnlineSub = IOnlineSubsystem::Get("EOK"))
	{
		if (FOnlineSubsystemEOS* EOSRef = static_cast<FOnlineSubsystemEOS*>(OnlineSub))
		{
			EOS_Presence_QueryPresenceOptions Options = {};
			Options.LocalUserId = Var_LocalUserId.GetValueAsEosType();
			Options.TargetUserId = Var_TargetUserId.GetValueAsEosType();
			EOS_Presence_QueryPresence(EOSRef->PresenceHandle, &Options, this, &UEOK_Presence_QueryPresence::Internal_OnQueryPresenceComplete);
			return;
		}
	}
	UE_LOG(LogEOK, Error, TEXT("UEOK_Presence_QueryPresence::Activate: Unable to get EOS SDK"));
	OnCallback.Broadcast(Var_LocalUserId, Var_TargetUserId, EEOK_Result::EOS_NotFound);
	SetReadyToDestroy();
#if ENGINE_MAJOR_VERSION == 5
	MarkAsGarbage();
#else
	MarkPendingKill();
#endif
}

void UEOK_Presence_QueryPresence::Internal_OnQueryPresenceComplete(const EOS_Presence_QueryPresenceCallbackInfo* Data)
{
	UEOK_Presence_QueryPresence* PresenceQuery = static_cast<UEOK_Presence_QueryPresence*>(Data->ClientData);
	if (PresenceQuery)
	{
		AsyncTask(ENamedThreads::GameThread, [PresenceQuery, Data]()
		{
			PresenceQuery->OnCallback.Broadcast(PresenceQuery->Var_LocalUserId, PresenceQuery->Var_TargetUserId, static_cast<EEOK_Result>(Data->ResultCode));
			PresenceQuery->SetReadyToDestroy();
#if ENGINE_MAJOR_VERSION == 5
PresenceQuery->MarkAsGarbage();
#else
PresenceQuery->MarkPendingKill();
#endif
		});
	}
}
