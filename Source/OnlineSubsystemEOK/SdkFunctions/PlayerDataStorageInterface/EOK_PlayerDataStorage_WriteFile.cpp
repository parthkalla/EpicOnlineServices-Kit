// Copyright (c) 2023 Betide Studio. All Rights Reserved.


#include "EOK_PlayerDataStorage_WriteFile.h"


UEOK_PlayerDataStorage_WriteFile* UEOK_PlayerDataStorage_WriteFile::EOK_PlayerDataStorage_WriteFile(
	FEOK_ProductUserId LocalUserId, FString Filename, const TArray<uint8>& Data, int32 DataLengthBytes,
	int32 ChunkLengthBytes)
{
	UEOK_PlayerDataStorage_WriteFile* Node = NewObject<UEOK_PlayerDataStorage_WriteFile>();
	Node->Var_LocalUserId = LocalUserId;
	Node->Var_Filename = Filename;
	Node->Var_Data = Data;
	Node->Var_DataLengthBytes = DataLengthBytes;
	Node->Var_ChunkLengthBytes = ChunkLengthBytes;
	return Node;
}

void UEOK_PlayerDataStorage_WriteFile::EOS_PlayerDataStorage_OnFileTransferProgress(
	const EOS_PlayerDataStorage_FileTransferProgressCallbackInfo* Data)
{
	if (UEOK_PlayerDataStorage_WriteFile* Node = static_cast<UEOK_PlayerDataStorage_WriteFile*>(Data->ClientData))
	{
		AsyncTask(ENamedThreads::GameThread, [Node, Data]()
		{
			Node->OnFileTransferProgressCallback.Broadcast(Data->LocalUserId, Data->Filename, Data->BytesTransferred,
				Data->TotalFileSizeBytes);
		});
	}
}

EOS_PlayerDataStorage_EWriteResult UEOK_PlayerDataStorage_WriteFile::EOS_PlayerDataStorage_OnWriteFileData(
	const EOS_PlayerDataStorage_WriteFileDataCallbackInfo* Data, void* OutDataBuffer, uint32_t* OutDataWritten)
{
	if (UEOK_PlayerDataStorage_WriteFile* Node = static_cast<UEOK_PlayerDataStorage_WriteFile*>(Data->ClientData))
	{
		uint32_t BytesRemaining = Node->Var_DataLengthBytes - Node->BytesWritten;
		uint32_t BytesToWrite = FMath::Min(BytesRemaining, static_cast<uint32_t>(Node->Var_ChunkLengthBytes));
		FMemory::Memcpy(OutDataBuffer, Node->Var_Data.GetData() + Node->BytesWritten, BytesToWrite);
		*OutDataWritten = BytesToWrite;

		Node->BytesWritten += BytesToWrite;
		if (Node->BytesWritten >= Node->Var_DataLengthBytes)
		{
			return EOS_PlayerDataStorage_EWriteResult::EOS_WR_CompleteRequest;
		}
		return EOS_PlayerDataStorage_EWriteResult::EOS_WR_ContinueWriting;
	}
	return EOS_PlayerDataStorage_EWriteResult::EOS_WR_FailRequest;
}

void UEOK_PlayerDataStorage_WriteFile::Activate()
{
	Super::Activate();
	if (IOnlineSubsystem* OnlineSub = IOnlineSubsystem::Get("EOK"))
	{
		if (FOnlineSubsystemEOS* EOSRef = static_cast<FOnlineSubsystemEOS*>(OnlineSub))
		{
			EOS_PlayerDataStorage_WriteFileOptions Options = {};
			Options.ApiVersion = EOS_PLAYERDATASTORAGE_WRITEFILE_API_LATEST;
			Options.LocalUserId = Var_LocalUserId.GetValueAsEosType();
			Options.Filename = TCHAR_TO_ANSI(*Var_Filename);
			Options.WriteFileDataCallback = &UEOK_PlayerDataStorage_WriteFile::EOS_PlayerDataStorage_OnWriteFileData;
			Options.FileTransferProgressCallback = &UEOK_PlayerDataStorage_WriteFile::EOS_PlayerDataStorage_OnFileTransferProgress;
			EOS_PlayerDataStorage_WriteFile(EOSRef->PlayerDataStorageHandle, &Options, this, &UEOK_PlayerDataStorage_WriteFile::EOS_PlayerDataStorage_OnWriteFileComplete);
			return;
		}
	}
	UE_LOG(LogEOK, Error, TEXT("EOK_PlayerDataStorage_WriteFile: OnlineSubsystemEOS is not valid"));
}

void UEOK_PlayerDataStorage_WriteFile::EOS_PlayerDataStorage_OnWriteFileComplete(
	const EOS_PlayerDataStorage_WriteFileCallbackInfo* Data)
{
	if (UEOK_PlayerDataStorage_WriteFile* Node = static_cast<UEOK_PlayerDataStorage_WriteFile*>(Data->ClientData))
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
