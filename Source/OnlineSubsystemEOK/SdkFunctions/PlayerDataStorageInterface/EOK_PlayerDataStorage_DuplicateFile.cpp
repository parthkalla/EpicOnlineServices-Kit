// Copyright (c) 2023 Betide Studio. All Rights Reserved.


#include "EOK_PlayerDataStorage_DuplicateFile.h"

UEOK_PlayerDataStorage_DuplicateFile* UEOK_PlayerDataStorage_DuplicateFile::EOK_PlayerDataStorage_DuplicateFile(
	FEOK_ProductUserId LocalUserId, FString SourceFilename, FString DestinationFilename)
{
	UEOK_PlayerDataStorage_DuplicateFile* Node = NewObject<UEOK_PlayerDataStorage_DuplicateFile>();
	Node->Var_LocalUserId = LocalUserId;
	Node->Var_SourceFilename = SourceFilename;
	Node->Var_DestinationFilename = DestinationFilename;
	return Node;
}

void UEOK_PlayerDataStorage_DuplicateFile::Activate()
{
	Super::Activate();
	if (IOnlineSubsystem* OnlineSub = IOnlineSubsystem::Get("EOK"))
	{
		if (FOnlineSubsystemEOS* EOSRef = static_cast<FOnlineSubsystemEOS*>(OnlineSub))
		{
			EOS_PlayerDataStorage_DuplicateFileOptions Options = {};
			Options.ApiVersion = EOS_PLAYERDATASTORAGE_DUPLICATEFILE_API_LATEST;
			Options.LocalUserId = Var_LocalUserId.GetValueAsEosType();
			Options.SourceFilename = TCHAR_TO_ANSI(*Var_SourceFilename);
			Options.DestinationFilename = TCHAR_TO_ANSI(*Var_DestinationFilename);
			EOS_PlayerDataStorage_DuplicateFile(EOSRef->PlayerDataStorageHandle, &Options, this, &UEOK_PlayerDataStorage_DuplicateFile::EOS_PlayerDataStorage_OnDuplicateFileComplete);
			return;
		}
	}
	UE_LOG(LogEOK, Error, TEXT("EOK_PlayerDataStorage_DuplicateFile: OnlineSubsystemEOS is not valid"));
	OnCallback.Broadcast(EEOK_Result::EOS_NotFound, FEOK_ProductUserId());
	SetReadyToDestroy();
#if ENGINE_MAJOR_VERSION == 5
	MarkAsGarbage();
#else
	MarkPendingKill();
#endif
}

void UEOK_PlayerDataStorage_DuplicateFile::EOS_PlayerDataStorage_OnDuplicateFileComplete(const EOS_PlayerDataStorage_DuplicateFileCallbackInfo* Data)
{
	if (UEOK_PlayerDataStorage_DuplicateFile* Node = static_cast<UEOK_PlayerDataStorage_DuplicateFile*>(Data->ClientData))
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
