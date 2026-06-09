// Copyright (c) 2023 Betide Studio. All Rights Reserved.


#include "EOK_Connect_CreateUser.h"

#include "EOK_ConnectSubsystem.h"
#include "OnlineSubsystemEOS.h"
#include "Async/Async.h"
#include "OnlineSubsystemEOK/SdkFunctions/EOK_SharedFunctionFile.h"


UEOK_Connect_CreateUser* UEOK_Connect_CreateUser::CreateUser(FEOK_ContinuanceToken ContinuanceToken)
{
	UEOK_Connect_CreateUser* Node = NewObject<UEOK_Connect_CreateUser>();
	Node->Var_ContinuanceToken = ContinuanceToken;
	return Node;
}

void UEOK_Connect_CreateUser::Activate()
{
	Super::Activate();
	if(	IOnlineSubsystem* OnlineSub = IOnlineSubsystem::Get("EOK"))
	{
		if (FOnlineSubsystemEOS* EOSRef = static_cast<FOnlineSubsystemEOS*>(OnlineSub))
		{
			EOS_Connect_CreateUserOptions CreateUserOptions = { };
			CreateUserOptions.ApiVersion = EOS_CONNECT_CREATEUSER_API_LATEST;
			CreateUserOptions.ContinuanceToken = Var_ContinuanceToken.GetValueAsEosType();
			EOS_Connect_CreateUser(EOSRef->ConnectHandle, &CreateUserOptions, this, &UEOK_Connect_CreateUser::OnCreateUserCallback);
			return;
		}
	}
	UE_LOG(LogEOK, Error, TEXT("Failed to create user either OnlineSubsystem is not valid or EOSRef is not valid."));
	OnCallback.Broadcast(EEOK_Result::EOS_NotFound, FEOK_ProductUserId());
	SetReadyToDestroy();
	#if ENGINE_MAJOR_VERSION == 5
	MarkAsGarbage();
#else
	MarkPendingKill();
#endif
}

void UEOK_Connect_CreateUser::OnCreateUserCallback(const EOS_Connect_CreateUserCallbackInfo* Data)
{
	UEOK_Connect_CreateUser* Node = static_cast<UEOK_Connect_CreateUser*>(Data->ClientData);
	if(Node)
	{
		AsyncTask(ENamedThreads::GameThread, [Node, Data]()
		{
			Node->OnCallback.Broadcast(static_cast<EEOK_Result>(Data->ResultCode), Data->LocalUserId);
		});
		Node->SetReadyToDestroy();
#if ENGINE_MAJOR_VERSION == 5
		Node->MarkAsGarbage();
#else
		Node->MarkPendingKill();
#endif
	}
}
