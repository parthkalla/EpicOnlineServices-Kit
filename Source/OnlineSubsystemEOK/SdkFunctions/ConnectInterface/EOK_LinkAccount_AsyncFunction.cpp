// Copyright (c) 2023 Betide Studio. All Rights Reserved.


#include "EOK_LinkAccount_AsyncFunction.h"

#include "Async/Async.h"


UEOK_LinkAccount_AsyncFunction* UEOK_LinkAccount_AsyncFunction::LinkAccount(const FString& LocalProductUserId,
                                                                            const FEOK_ContinuanceToken& ContinuanceToken)
{
	UEOK_LinkAccount_AsyncFunction* BlueprintNode = NewObject<UEOK_LinkAccount_AsyncFunction>();
	BlueprintNode->Var_LocalProductUserId = LocalProductUserId;
	BlueprintNode->Var_ContinuanceToken = ContinuanceToken;
	return BlueprintNode;
}

void UEOK_LinkAccount_AsyncFunction::OnLinkAccountCallback(const EOS_Connect_LinkAccountCallbackInfo* Data)
{
	TEnumAsByte<EEOK_Result> Result = static_cast<EEOK_Result>(Data->ResultCode);
	UEOK_LinkAccount_AsyncFunction* LinkAccountFunction = static_cast<UEOK_LinkAccount_AsyncFunction*>(Data->ClientData);
	if(LinkAccountFunction)
	{
		AsyncTask(ENamedThreads::GameThread, [LinkAccountFunction, Data]()
		{
			LinkAccountFunction->OnCallback.Broadcast(static_cast<EEOK_Result>(Data->ResultCode));
		});
		LinkAccountFunction->SetReadyToDestroy();
#if ENGINE_MAJOR_VERSION == 5
		LinkAccountFunction->MarkAsGarbage();
#else
		LinkAccountFunction->MarkPendingKill();
#endif
	}
}

void UEOK_LinkAccount_AsyncFunction::Activate()
{
	Super::Activate();
	if(	IOnlineSubsystem* OnlineSub = IOnlineSubsystem::Get("EOK"))
	{
		if (FOnlineSubsystemEOS* EOSRef = static_cast<FOnlineSubsystemEOS*>(OnlineSub))
		{
			EOS_Connect_LinkAccountOptions LinkAccountOptions = { };
			LinkAccountOptions.ApiVersion = EOS_CONNECT_LINKACCOUNT_API_LATEST;
			LinkAccountOptions.LocalUserId = EOS_ProductUserId_FromString(TCHAR_TO_ANSI(*Var_LocalProductUserId));
			LinkAccountOptions.ContinuanceToken = Var_ContinuanceToken.GetValueAsEosType();
			EOS_Connect_LinkAccount(EOSRef->ConnectHandle, &LinkAccountOptions, this, &UEOK_LinkAccount_AsyncFunction::OnLinkAccountCallback);
			return;
		}
	}
	OnCallback.Broadcast(EEOK_Result::EOS_ServiceFailure);
	SetReadyToDestroy();
	#if ENGINE_MAJOR_VERSION == 5
	MarkAsGarbage();
#else
	MarkPendingKill();
#endif
}
