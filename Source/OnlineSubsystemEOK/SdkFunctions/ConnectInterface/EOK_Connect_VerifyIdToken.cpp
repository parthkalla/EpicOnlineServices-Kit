// Copyright (c) 2023 Betide Studio. All Rights Reserved.


#include "EOK_Connect_VerifyIdToken.h"

#include "EOK_ConnectSubsystem.h"
#include "OnlineSubsystemEOS.h"
#include "Async/Async.h"

UEOK_Connect_VerifyIdToken* UEOK_Connect_VerifyIdToken::EOK_Connect_VerifyIdToken(FEOK_Connect_IdToken IdToken)
{
	UEOK_Connect_VerifyIdToken* Node = NewObject<UEOK_Connect_VerifyIdToken>();
	Node->Var_IdToken = IdToken;
	return Node;
}

void UEOK_Connect_VerifyIdToken::OnVerifyIdTokenCallback(const EOS_Connect_VerifyIdTokenCallbackInfo* Data)
{
	if (UEOK_Connect_VerifyIdToken* Proxy = static_cast<UEOK_Connect_VerifyIdToken*>(Data->ClientData))
	{
		AsyncTask(ENamedThreads::GameThread, [Proxy, Data]()
		{
			Proxy->OnCallback.Broadcast(Data);
		});
		Proxy->SetReadyToDestroy();
#if ENGINE_MAJOR_VERSION == 5
		Proxy->MarkAsGarbage();
#else
		Proxy->MarkPendingKill();
#endif
	}
}	

void UEOK_Connect_VerifyIdToken::Activate()
{
	Super::Activate();
	if (IOnlineSubsystem* OnlineSub = IOnlineSubsystem::Get("EOK"))
	{
		if (FOnlineSubsystemEOS* EOSRef = static_cast<FOnlineSubsystemEOS*>(OnlineSub))
		{
			EOS_Connect_VerifyIdTokenOptions VerifyIdTokenOptions = {};
			VerifyIdTokenOptions.ApiVersion = EOS_CONNECT_VERIFYIDTOKEN_API_LATEST;
			EOS_Connect_IdToken Temp = Var_IdToken.GetValueAsEosType();
			VerifyIdTokenOptions.IdToken = &Temp;
			EOS_Connect_VerifyIdToken(EOSRef->ConnectHandle, &VerifyIdTokenOptions, this, &UEOK_Connect_VerifyIdToken::OnVerifyIdTokenCallback);
			return;
		}
	}
	UE_LOG(LogEOK, Error, TEXT("Failed to verify id token either OnlineSubsystem is not valid or EOSRef is not valid."));
	OnCallback.Broadcast(FEOK_Connect_VerifyIdTokenCallbackInfo());
	SetReadyToDestroy();
#if ENGINE_MAJOR_VERSION == 5
	MarkAsGarbage();
#else
	MarkPendingKill();
#endif
}
