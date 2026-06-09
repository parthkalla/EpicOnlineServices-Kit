// Copyright (c) 2023 Betide Studio. All Rights Reserved.


#include "EOK_SessionSearch_Find.h"

#include "Async/Async.h"

UEOK_SessionSearch_Find* UEOK_SessionSearch_Find::EOK_SessionSearch_Find(FEOK_HSessionSearch SessionSearchHandle,
                                                                         FEOK_ProductUserId LocalUserId)
{
	UEOK_SessionSearch_Find* Node = NewObject<UEOK_SessionSearch_Find>();
	Node->Var_SessionSearchHandle = SessionSearchHandle;
	Node->Var_LocalUserId = LocalUserId;
	return Node;
}

void UEOK_SessionSearch_Find::Activate()
{
	Super::Activate();
	if (IOnlineSubsystem* OnlineSub = IOnlineSubsystem::Get("EOK"))
	{
		if (FOnlineSubsystemEOS* EOSRef = static_cast<FOnlineSubsystemEOS*>(OnlineSub))
		{
			if(!Var_SessionSearchHandle.Ref)
			{
				UE_LOG(LogEOK, Error, TEXT("Failed to find session because SessionSearchHandle is not valid."));
				OnCallback.Broadcast(EEOK_Result::EOS_NotFound);
				SetReadyToDestroy();
				#if ENGINE_MAJOR_VERSION == 5
	MarkAsGarbage();
#else
	MarkPendingKill();
#endif
				return;
			}
			EOS_SessionSearch_FindOptions SessionSearchFindOptions = { };
			SessionSearchFindOptions.ApiVersion = EOS_SESSIONSEARCH_FIND_API_LATEST;
			SessionSearchFindOptions.LocalUserId = Var_LocalUserId.GetValueAsEosType();
			EOS_SessionSearch_Find(*Var_SessionSearchHandle.Ref, &SessionSearchFindOptions, this, &UEOK_SessionSearch_Find::OnSessionSearch_FindCallback);
			return;
		}
	}
	UE_LOG(LogEOK, Error, TEXT("Failed to find session either OnlineSubsystem is not valid or EOSRef is not valid."));
	OnCallback.Broadcast(EEOK_Result::EOS_NotFound);
	SetReadyToDestroy();
#if ENGINE_MAJOR_VERSION == 5
	MarkAsGarbage();
#else
	MarkPendingKill();
#endif
}
void UEOK_SessionSearch_Find::OnSessionSearch_FindCallback(const EOS_SessionSearch_FindCallbackInfo* Data)
{
	if (UEOK_SessionSearch_Find* Node = static_cast<UEOK_SessionSearch_Find*>(Data->ClientData))
	{
		AsyncTask(ENamedThreads::GameThread, [Node, Data]()
		{
			Node->OnCallback.Broadcast(static_cast<EEOK_Result>(Data->ResultCode));
		});
		Node->SetReadyToDestroy();
#if ENGINE_MAJOR_VERSION == 5
		Node->MarkAsGarbage();
#else
		Node->MarkPendingKill();
#endif
	}
}
