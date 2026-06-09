// Copyright (c) 2023 Betide Studio. All Rights Reserved.


#include "EOK_Auth_DeletePersistentAuth.h"

#include "OnlineSubsystemEOS.h"
#include "Async/Async.h"
#include "OnlineSubsystemEOK/SdkFunctions/ConnectInterface/EOK_ConnectSubsystem.h"

UEOK_Auth_DeletePersistentAuth* UEOK_Auth_DeletePersistentAuth::DeletePersistentAuth(FString RefreshToken)
{
	UEOK_Auth_DeletePersistentAuth* Node = NewObject<UEOK_Auth_DeletePersistentAuth>();
	Node->Var_RefreshToken = RefreshToken;
	return Node;
}

void UEOK_Auth_DeletePersistentAuth::Activate()
{
	Super::Activate();
	if(	IOnlineSubsystem* OnlineSub = IOnlineSubsystem::Get("EOK"))
	{
		if (FOnlineSubsystemEOS* EOSRef = static_cast<FOnlineSubsystemEOS*>(OnlineSub))
		{
			EOS_Auth_DeletePersistentAuthOptions Options = {};
			Options.ApiVersion = EOS_AUTH_DELETEPERSISTENTAUTH_API_LATEST;
			if(Var_RefreshToken.IsEmpty())
			{
				Options.RefreshToken = nullptr;
			}
			else
			{
				Options.RefreshToken = TCHAR_TO_ANSI(*Var_RefreshToken);
			}
			EOS_Auth_DeletePersistentAuth(EOSRef->AuthHandle, &Options, this, &UEOK_Auth_DeletePersistentAuth::OnDeletePersistentAuthCallback);
			return;
		}
	}
	UE_LOG(LogEOK, Error, TEXT("Failed to delete persistent auth either OnlineSubsystem is not valid or EOSRef is not valid."));
	OnCallback.Broadcast(EEOK_Result::EOS_NotFound);
	SetReadyToDestroy();
	#if ENGINE_MAJOR_VERSION == 5
	MarkAsGarbage();
#else
	MarkPendingKill();
#endif
}

void UEOK_Auth_DeletePersistentAuth::OnDeletePersistentAuthCallback(const EOS_Auth_DeletePersistentAuthCallbackInfo* Data)
{
	UEOK_Auth_DeletePersistentAuth* Node = static_cast<UEOK_Auth_DeletePersistentAuth*>(Data->ClientData);
	if(Node)
	{
		AsyncTask(ENamedThreads::GameThread, [Node, Data]()
		{
			Node->OnCallback.Broadcast(static_cast<EEOK_Result>(Data->ResultCode));
		});
		Node->SetReadyToDestroy();
#if ENGINE_MAJOR_VERSION == 5
		Node->MarkAsGarbage();
#else
		Node->MarkPendingKill();
#endif
	}
}
