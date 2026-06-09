// Copyright (c) 2023 Betide Studio. All Rights Reserved.


#include "EOK_PlayerDataStorage_DeleteFile.h"



UEOK_PlayerDataStorage_DeleteFile* UEOK_PlayerDataStorage_DeleteFile::EOK_PlayerDataStorage_DeleteFile(
	FEOK_ProductUserId LocalUserId, FString Filename)
{
	UEOK_PlayerDataStorage_DeleteFile* Action = NewObject<UEOK_PlayerDataStorage_DeleteFile>();
	Action->Var_LocalUserId = LocalUserId;
	Action->Var_Filename = Filename;
	return Action;
}

void UEOK_PlayerDataStorage_DeleteFile::Activate()
{
	Super::Activate();
	if (IOnlineSubsystem* OnlineSub = IOnlineSubsystem::Get("EOK"))
	{
		if (FOnlineSubsystemEOS* EOSRef = static_cast<FOnlineSubsystemEOS*>(OnlineSub))
		{
			EOS_PlayerDataStorage_DeleteFileOptions Options = {};
			Options.ApiVersion = EOS_PLAYERDATASTORAGE_DELETEFILE_API_LATEST;
			Options.LocalUserId = Var_LocalUserId.GetValueAsEosType();
			Options.Filename = TCHAR_TO_ANSI(*Var_Filename);
			EOS_PlayerDataStorage_DeleteFile(EOSRef->PlayerDataStorageHandle, &Options, this, &UEOK_PlayerDataStorage_DeleteFile::EOS_PlayerDataStorage_OnDeleteFileComplete);
			return;
		}
	}
	UE_LOG(LogEOK, Error, TEXT("EOK_PlayerDataStorage_DeleteFile: OnlineSubsystemEOS is not valid"));
	OnCallback.Broadcast(EEOK_Result::EOS_NotFound, FEOK_ProductUserId());
	SetReadyToDestroy();
#if ENGINE_MAJOR_VERSION == 5
	MarkAsGarbage();
#else
	MarkPendingKill();
#endif
}

void UEOK_PlayerDataStorage_DeleteFile::EOS_PlayerDataStorage_OnDeleteFileComplete(
	const EOS_PlayerDataStorage_DeleteFileCallbackInfo* Data)
{
	if (UEOK_PlayerDataStorage_DeleteFile* Node = static_cast<UEOK_PlayerDataStorage_DeleteFile*>(Data->ClientData))
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
