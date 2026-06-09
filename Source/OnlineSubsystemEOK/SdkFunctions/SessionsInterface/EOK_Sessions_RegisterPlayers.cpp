// Copyright (c) 2023 Betide Studio. All Rights Reserved.


#include "EOK_Sessions_RegisterPlayers.h"

#include "Async/Async.h"

UEOK_Sessions_RegisterPlayers* UEOK_Sessions_RegisterPlayers::EOK_Sessions_RegisterPlayers(FString SessionName,
                                                                                           TArray<FEOK_ProductUserId> PlayersToRegister)
{
	UEOK_Sessions_RegisterPlayers* Node = NewObject<UEOK_Sessions_RegisterPlayers>();
	Node->Var_SessionName = SessionName;
	Node->Var_PlayersToRegister = PlayersToRegister;
	return Node;
}

void UEOK_Sessions_RegisterPlayers::Activate()
{
	Super::Activate();
	if (IOnlineSubsystem* OnlineSub = IOnlineSubsystem::Get("EOK"))
	{
		if (FOnlineSubsystemEOS* EOSRef = static_cast<FOnlineSubsystemEOS*>(OnlineSub))
		{
			EOS_Sessions_RegisterPlayersOptions RegisterPlayersOptions = { };
			RegisterPlayersOptions.ApiVersion = EOS_SESSIONS_REGISTERPLAYERS_API_LATEST;
			RegisterPlayersOptions.SessionName = TCHAR_TO_ANSI(*Var_SessionName);
			RegisterPlayersOptions.PlayersToRegisterCount = Var_PlayersToRegister.Num();
			RegisterPlayersOptions.PlayersToRegister = new EOS_ProductUserId[Var_PlayersToRegister.Num()];
			for (int i = 0; i < Var_PlayersToRegister.Num(); i++)
			{
				RegisterPlayersOptions.PlayersToRegister[i] = Var_PlayersToRegister[i].GetValueAsEosType();
			}
			EOS_Sessions_RegisterPlayers(EOSRef->SessionsHandle, &RegisterPlayersOptions, this, &UEOK_Sessions_RegisterPlayers::OnRegisterPlayersCallback);
			delete[] RegisterPlayersOptions.PlayersToRegister;
			return;
		}
	}
	UE_LOG(LogEOK, Error, TEXT("Failed to register players either OnlineSubsystem is not valid or EOSRef is not valid."));
	OnCallback.Broadcast(EEOK_Result::EOS_NotFound, TArray<FEOK_ProductUserId>(), TArray<FEOK_ProductUserId>());
	SetReadyToDestroy();
	#if ENGINE_MAJOR_VERSION == 5
	MarkAsGarbage();
#else
	MarkPendingKill();
#endif
}

void UEOK_Sessions_RegisterPlayers::OnRegisterPlayersCallback(const EOS_Sessions_RegisterPlayersCallbackInfo* Data)
{
	if (UEOK_Sessions_RegisterPlayers* Node = static_cast<UEOK_Sessions_RegisterPlayers*>(Data->ClientData))
	{
		TArray<FEOK_ProductUserId> RegisteredPlayers;
		for (int i = 0; i < (int32)Data->RegisteredPlayersCount; i++)
		{
			RegisteredPlayers.Add(FEOK_ProductUserId(Data->RegisteredPlayers[i]));
		}
		TArray<FEOK_ProductUserId> SanctionedPlayers;
		for (int i = 0; i < (int32)(Data->SanctionedPlayersCount); i++)
		{
			SanctionedPlayers.Add(FEOK_ProductUserId(Data->SanctionedPlayers[i]));
		}
		AsyncTask(ENamedThreads::GameThread, [Node, Data, RegisteredPlayers, SanctionedPlayers]()
		{
			Node->OnCallback.Broadcast(static_cast<EEOK_Result>(Data->ResultCode), RegisteredPlayers, SanctionedPlayers);
		});
		Node->SetReadyToDestroy();
#if ENGINE_MAJOR_VERSION == 5
		Node->MarkAsGarbage();
#else
		Node->MarkPendingKill();
#endif
	}
}
