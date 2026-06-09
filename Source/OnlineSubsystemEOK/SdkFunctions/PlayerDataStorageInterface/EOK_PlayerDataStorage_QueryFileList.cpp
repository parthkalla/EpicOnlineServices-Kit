// Copyright (c) 2023 Betide Studio. All Rights Reserved.


#include "EOK_PlayerDataStorage_QueryFileList.h"

UEOK_PlayerDataStorage_QueryFileList* UEOK_PlayerDataStorage_QueryFileList::EOK_PlayerDataStorage_QueryFileList(
	FEOK_ProductUserId LocalUserId)
{
	UEOK_PlayerDataStorage_QueryFileList* Node = NewObject<UEOK_PlayerDataStorage_QueryFileList>();
	Node->Var_LocalUserId = LocalUserId;
	return Node;
}

void UEOK_PlayerDataStorage_QueryFileList::Activate()
{
	Super::Activate();
	if (IOnlineSubsystem* OnlineSub = IOnlineSubsystem::Get("EOK"))
	{
		if (FOnlineSubsystemEOS* EOSRef = static_cast<FOnlineSubsystemEOS*>(OnlineSub))
		{
			EOS_PlayerDataStorage_QueryFileListOptions Options = {};
			Options.ApiVersion = EOS_PLAYERDATASTORAGE_QUERYFILELIST_API_LATEST;
			Options.LocalUserId = Var_LocalUserId.GetValueAsEosType();
			EOS_PlayerDataStorage_QueryFileList(EOSRef->PlayerDataStorageHandle, &Options, this, &UEOK_PlayerDataStorage_QueryFileList::EOS_PlayerDataStorage_OnQueryFileListComplete);
			return;
		}
	}
	UE_LOG(LogEOK, Error, TEXT("EOK_PlayerDataStorage_QueryFileList: OnlineSubsystemEOS is not valid"));
	OnCallback.Broadcast(EEOK_Result::EOS_NotFound, FEOK_ProductUserId(), -1);
	SetReadyToDestroy();
#if ENGINE_MAJOR_VERSION == 5
	MarkAsGarbage();
#else
	MarkPendingKill();
#endif
}

void UEOK_PlayerDataStorage_QueryFileList::EOS_PlayerDataStorage_OnQueryFileListComplete(const EOS_PlayerDataStorage_QueryFileListCallbackInfo* Data)
{
	if (UEOK_PlayerDataStorage_QueryFileList* Node = static_cast<UEOK_PlayerDataStorage_QueryFileList*>(Data->ClientData))
	{
		AsyncTask(ENamedThreads::GameThread, [Node, Data]()
		{
			Node->OnCallback.Broadcast(static_cast<EEOK_Result>(Data->ResultCode), Data->LocalUserId, Data->FileCount);
			Node->SetReadyToDestroy();
#if ENGINE_MAJOR_VERSION == 5
Node->MarkAsGarbage();
#else
Node->MarkPendingKill();
#endif
		});
	}
}