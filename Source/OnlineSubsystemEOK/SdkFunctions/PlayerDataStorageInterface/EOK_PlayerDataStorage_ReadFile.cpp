// Copyright (c) 2023 Betide Studio. All Rights Reserved.


#include "EOK_PlayerDataStorage_ReadFile.h"




void UEOK_PlayerDataStorage_ReadFile::EOS_PlayerDataStorage_OnReadFileComplete(
	const EOS_PlayerDataStorage_ReadFileCallbackInfo* Data)
{
	if (UEOK_PlayerDataStorage_ReadFile* Node = static_cast<UEOK_PlayerDataStorage_ReadFile*>(Data->ClientData))
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

EOS_PlayerDataStorage_EReadResult UEOK_PlayerDataStorage_ReadFile::EOS_PlayerDataStorage_OnReadFileData(
	const EOS_PlayerDataStorage_ReadFileDataCallbackInfo* Data)
{
	EOS_PlayerDataStorage_EReadResult Result = EOS_PlayerDataStorage_EReadResult::EOS_RR_ContinueReading;
	return Result;
}


void UEOK_PlayerDataStorage_ReadFile::EOS_PlayerDataStorage_OnFileTransferProgress(
	const EOS_PlayerDataStorage_FileTransferProgressCallbackInfo* Data)
{
	if (UEOK_PlayerDataStorage_ReadFile* Node = static_cast<UEOK_PlayerDataStorage_ReadFile*>(Data->ClientData))
	{
		AsyncTask(ENamedThreads::GameThread, [Node, Data]()
		{
			Node->OnFileTransferProgressCallback.Broadcast(Data->LocalUserId, Data->Filename, Data->BytesTransferred, Data->TotalFileSizeBytes);
		});
	}
}

UEOK_PlayerDataStorage_ReadFile* UEOK_PlayerDataStorage_ReadFile::EOK_PlayerDataStorage_ReadFile(
	FEOK_ProductUserId LocalUserId, FString Filename, int32 ReadChunkLengthBytes)
{
	UEOK_PlayerDataStorage_ReadFile* Node = NewObject<UEOK_PlayerDataStorage_ReadFile>();
	Node->Var_LocalUserId = LocalUserId;
	Node->Var_Filename = Filename;
	Node->Var_ReadChunkLengthBytes = ReadChunkLengthBytes;
	return Node;
}

void UEOK_PlayerDataStorage_ReadFile::Activate()
{
	Super::Activate();
	if (IOnlineSubsystem* OnlineSub = IOnlineSubsystem::Get("EOK"))
	{
		if (FOnlineSubsystemEOS* EOSRef = static_cast<FOnlineSubsystemEOS*>(OnlineSub))
		{
			EOS_PlayerDataStorage_ReadFileOptions Options = {};
			Options.ApiVersion = EOS_PLAYERDATASTORAGE_READFILE_API_LATEST;
			Options.LocalUserId = Var_LocalUserId.GetValueAsEosType();
			Options.Filename = TCHAR_TO_ANSI(*Var_Filename);
			Options.ReadChunkLengthBytes = Var_ReadChunkLengthBytes;
			Options.ReadFileDataCallback = &UEOK_PlayerDataStorage_ReadFile::EOS_PlayerDataStorage_OnReadFileData;
			Options.FileTransferProgressCallback = &UEOK_PlayerDataStorage_ReadFile::EOS_PlayerDataStorage_OnFileTransferProgress;
			EOS_PlayerDataStorage_ReadFile(EOSRef->PlayerDataStorageHandle, &Options, this, &UEOK_PlayerDataStorage_ReadFile::EOS_PlayerDataStorage_OnReadFileComplete);
			return;
		}
	}
	UE_LOG(LogEOK, Error, TEXT("EOK_PlayerDataStorage_ReadFile: OnlineSubsystemEOS is not valid"));
	OnCallback.Broadcast(EEOK_Result::EOS_NotFound, FEOK_ProductUserId());
	SetReadyToDestroy();
#if ENGINE_MAJOR_VERSION == 5
	MarkAsGarbage();
#else
	MarkPendingKill();
#endif
}
