// Copyright (c) 2023 Betide Studio. All Rights Reserved.


#include "EOK_Presence_SetPresence.h"

#include "Async/Async.h"
#include "OnlineSubsystemEOK/SdkFunctions/ConnectInterface/EOK_ConnectSubsystem.h"

UEOK_Presence_SetPresence* UEOK_Presence_SetPresence::EOK_Presence_SetPresence(FEOK_EpicAccountId LocalUserId,
                                                                               FEOK_HPresenceModification PresenceModificationHandle)
{
	UEOK_Presence_SetPresence* Node = NewObject<UEOK_Presence_SetPresence>();
	Node->Var_LocalUserId = LocalUserId;
	Node->Var_PresenceModificationHandle = PresenceModificationHandle;
	return Node;
}

void UEOK_Presence_SetPresence::Activate()
{
	Super::Activate();
	if (IOnlineSubsystem* OnlineSub = IOnlineSubsystem::Get("EOK"))
	{
		if (FOnlineSubsystemEOS* EOSRef = static_cast<FOnlineSubsystemEOS*>(OnlineSub))
		{
			EOS_Presence_SetPresenceOptions Options = {};
			Options.LocalUserId = Var_LocalUserId.GetValueAsEosType();
			Options.PresenceModificationHandle = Var_PresenceModificationHandle.Ref;
			EOS_Presence_SetPresence(EOSRef->PresenceHandle, &Options, this, &UEOK_Presence_SetPresence::Internal_OnSetPresenceComplete);
			return;
		}
	}
	UE_LOG(LogEOK, Error, TEXT("UEOK_Presence_SetPresence::Activate: Unable to get EOS SDK"));
	OnCallback.Broadcast(Var_LocalUserId, EEOK_Result::EOS_NotFound);
	SetReadyToDestroy();
	#if ENGINE_MAJOR_VERSION == 5
	MarkAsGarbage();
#else
	MarkPendingKill();
#endif
}

void UEOK_Presence_SetPresence::Internal_OnSetPresenceComplete(const EOS_Presence_SetPresenceCallbackInfo* Data)
{
	if (UEOK_Presence_SetPresence* Node = static_cast<UEOK_Presence_SetPresence*>(Data->ClientData))
	{
		AsyncTask(ENamedThreads::GameThread, [Node, Data]()
		{
			Node->OnCallback.Broadcast(Node->Var_LocalUserId, static_cast<EEOK_Result>(Data->ResultCode));
			Node->SetReadyToDestroy();
#if ENGINE_MAJOR_VERSION == 5
Node->MarkAsGarbage();
#else
Node->MarkPendingKill();
#endif
		});
	}
}
