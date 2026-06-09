// Copyright (c) 2023 Betide Studio. All Rights Reserved.


#include "EOK_PlayerDataStorage_QueryFile.h"

UEOK_PlayerDataStorage_QueryFile* UEOK_PlayerDataStorage_QueryFile::EOK_PlayerDataStorage_QueryFile(
	FEOK_ProductUserId LocalUserId, FString Filename)
{
	UEOK_PlayerDataStorage_QueryFile* Action = NewObject<UEOK_PlayerDataStorage_QueryFile>();
	Action->Var_LocalUserId = LocalUserId;
	Action->Var_Filename = Filename;
	return Action;
}

void UEOK_PlayerDataStorage_QueryFile::Activate()
{
	Super::Activate();
	if (IOnlineSubsystem* OnlineSub = IOnlineSubsystem::Get("EOK"))
	{
		if (FOnlineSubsystemEOS* EOSRef = static_cast<FOnlineSubsystemEOS*>(OnlineSub))
		{
			EOS_PlayerDataStorage_QueryFileOptions Options = {};
			Options.ApiVersion = EOS_PLAYERDATASTORAGE_QUERYFILE_API_LATEST;
			Options.LocalUserId = Var_LocalUserId.GetValueAsEosType();
			Options.Filename = TCHAR_TO_ANSI(*Var_Filename);
			EOS_PlayerDataStorage_QueryFile(EOSRef->PlayerDataStorageHandle, &Options, this, &UEOK_PlayerDataStorage_QueryFile::EOS_PlayerDataStorage_OnQueryFileComplete);
			return;
		}
	}
	UE_LOG(LogEOK, Error, TEXT("EOK_PlayerDataStorage_QueryFile: OnlineSubsystemEOS is not valid"));
	OnCallback.Broadcast(EEOK_Result::EOS_NotFound, FEOK_ProductUserId());
	SetReadyToDestroy();
#if ENGINE_MAJOR_VERSION == 5
	MarkAsGarbage();
#else
	MarkPendingKill();
#endif
}

void UEOK_PlayerDataStorage_QueryFile::EOS_PlayerDataStorage_OnQueryFileComplete(const EOS_PlayerDataStorage_QueryFileCallbackInfo* Data)
{
	if (UEOK_PlayerDataStorage_QueryFile* Node = static_cast<UEOK_PlayerDataStorage_QueryFile*>(Data->ClientData))
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
