// Copyright (c) 2023 Betide Studio. All Rights Reserved.


#include "EOK_PlayerDataStorage_DeleteCache.h"

#include "OnlineSubsystemEOS.h"
#include "Async/Async.h"
#include "OnlineSubsystemEOK/SdkFunctions/ConnectInterface/EOK_ConnectSubsystem.h"

UEOK_PlayerDataStorage_DeleteCache* UEOK_PlayerDataStorage_DeleteCache::EOK_PlayerDataStorage_DeleteCache(
	FEOK_ProductUserId LocalUserId)
{
	UEOK_PlayerDataStorage_DeleteCache* Node = NewObject<UEOK_PlayerDataStorage_DeleteCache>();
	Node->Var_LocalUserId = LocalUserId;
	return Node;
}
void UEOK_PlayerDataStorage_DeleteCache::Activate()
{
	Super::Activate();
	if (IOnlineSubsystem* OnlineSub = IOnlineSubsystem::Get("EOK"))
	{
		if (FOnlineSubsystemEOS* EOSRef = static_cast<FOnlineSubsystemEOS*>(OnlineSub))
		{
			EOS_PlayerDataStorage_DeleteCacheOptions Options = {};
			Options.ApiVersion = EOS_PLAYERDATASTORAGE_DELETECACHE_API_LATEST;
			Options.LocalUserId = Var_LocalUserId.GetValueAsEosType();
			EOS_PlayerDataStorage_DeleteCache(EOSRef->PlayerDataStorageHandle, &Options, this, &UEOK_PlayerDataStorage_DeleteCache::EOS_PlayerDataStorage_OnDeleteCacheComplete);
			return;
		}
	}
	UE_LOG(LogEOK, Error, TEXT("EOK_PlayerDataStorage_DeleteCache: OnlineSubsystemEOS is not valid"));
	OnCallback.Broadcast(EEOK_Result::EOS_NotFound, FEOK_ProductUserId());
	SetReadyToDestroy();
#if ENGINE_MAJOR_VERSION == 5
	MarkAsGarbage();
#else
	MarkPendingKill();
#endif
}

void UEOK_PlayerDataStorage_DeleteCache::EOS_PlayerDataStorage_OnDeleteCacheComplete(
	const EOS_PlayerDataStorage_DeleteCacheCallbackInfo* Data)
{
	if (UEOK_PlayerDataStorage_DeleteCache* Node = static_cast<UEOK_PlayerDataStorage_DeleteCache*>(Data->ClientData))
	{
		AsyncTask(ENamedThreads::GameThread, [Node, Data]()
		{
			Node->OnCallback.Broadcast(static_cast<EEOK_Result>(Data->ResultCode), Data->LocalUserId);
			Node->SetReadyToDestroy();
#if ENGINE_MAJOR_VERSION == 5
Node->MarkAsGarbage();
#else
Node->MarkPendingKill();
#endif
		});
	}
}
