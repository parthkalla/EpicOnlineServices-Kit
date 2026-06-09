// Copyright (c) 2023 Betide Studio. All Rights Reserved.


#include "EOK_Auth_VerifyIdToken.h"

#include "OnlineSubsystemEOS.h"
#include "Async/Async.h"
#include "OnlineSubsystemEOK/SdkFunctions/ConnectInterface/EOK_ConnectSubsystem.h"

UEOK_Auth_VerifyIdToken* UEOK_Auth_VerifyIdToken::VerifyIdToken(FEOK_Auth_IdToken IdToken)
{
	UEOK_Auth_VerifyIdToken* Node = NewObject<UEOK_Auth_VerifyIdToken>();
	Node->Var_IdToken = IdToken;
	return Node;
}

void UEOK_Auth_VerifyIdToken::OnVerifyIdTokenCallback(const EOS_Auth_VerifyIdTokenCallbackInfo* Data)
{
	UEOK_Auth_VerifyIdToken* Node = static_cast<UEOK_Auth_VerifyIdToken*>(Data->ClientData);
	if(Node)
	{
		AsyncTask(ENamedThreads::GameThread, [Node, Data]()
		{
			Node->OnCallback.Broadcast(*Data);
		});
		Node->SetReadyToDestroy();
#if ENGINE_MAJOR_VERSION == 5
		Node->MarkAsGarbage();
#else
		Node->MarkPendingKill();
#endif
	}
}

void UEOK_Auth_VerifyIdToken::Activate()
{
	Super::Activate();
	if(	IOnlineSubsystem* OnlineSub = IOnlineSubsystem::Get("EOK"))
	{
		if (FOnlineSubsystemEOS* EOSRef = static_cast<FOnlineSubsystemEOS*>(OnlineSub))
		{
			EOS_Auth_VerifyIdTokenOptions Options = {};
			Options.ApiVersion = EOS_AUTH_VERIFYIDTOKEN_API_LATEST;
			EOS_Auth_IdToken TokenRef = Var_IdToken.GetValueAsEosType();
			Options.IdToken = &TokenRef;
			EOS_Auth_VerifyIdToken(EOSRef->AuthHandle, &Options, this, &UEOK_Auth_VerifyIdToken::OnVerifyIdTokenCallback);
			return;
		}
	}
	UE_LOG(LogEOK, Error, TEXT("Failed to verify id token either OnlineSubsystem is not valid or EOSRef is not valid."));
	OnCallback.Broadcast(FEOK_Auth_VerifyIdTokenCallbackInfo());
	SetReadyToDestroy();
	#if ENGINE_MAJOR_VERSION == 5
	MarkAsGarbage();
#else
	MarkPendingKill();
#endif
}
