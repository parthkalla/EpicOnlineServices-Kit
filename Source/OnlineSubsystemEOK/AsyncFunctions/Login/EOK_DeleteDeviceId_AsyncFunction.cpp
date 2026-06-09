// Copyright (c) 2023 Betide Studio. All Rights Reserved.


#include "EOK_DeleteDeviceId_AsyncFunction.h"


UEOK_DeleteDeviceId_AsyncFunction* UEOK_DeleteDeviceId_AsyncFunction::DeleteDeviceId()
{
	UEOK_DeleteDeviceId_AsyncFunction* UEOK_DeleteDeviceIdObject= NewObject<UEOK_DeleteDeviceId_AsyncFunction>();
	return UEOK_DeleteDeviceIdObject;
}

void UEOK_DeleteDeviceId_AsyncFunction::OnDeleteDeviceIdComplete(const EOS_Connect_DeleteDeviceIdCallbackInfo* Data)
{
	UEOK_DeleteDeviceId_AsyncFunction *This = static_cast<UEOK_DeleteDeviceId_AsyncFunction*>(Data->ClientData);
	if(This == nullptr)
	{
		UE_LOG(LogTemp, Error, TEXT("OnDeleteDeviceIdComplete: This is null"));
		return;
	}
	if(Data->ResultCode == EOS_EResult::EOS_Success)
	{
		This->OnSuccess.Broadcast();
	}
	else
	{
		This->OnFailure.Broadcast();	
	}
	This->SetReadyToDestroy();
#if ENGINE_MAJOR_VERSION == 5
	This->MarkAsGarbage();
#else
	This->MarkPendingKill();
#endif
}

void UEOK_DeleteDeviceId_AsyncFunction::Activate()
{
	if(	IOnlineSubsystem* OnlineSub = IOnlineSubsystem::Get())
	{
		if (FOnlineSubsystemEOS* EOSRef = static_cast<FOnlineSubsystemEOS*>(OnlineSub))
		{
			EOS_Connect_DeleteDeviceIdOptions Options = {};
			Options.ApiVersion = EOS_CONNECT_DELETEDEVICEID_API_LATEST;
			EOS_Connect_DeleteDeviceId(EOSRef->ConnectHandle, &Options, this, &UEOK_DeleteDeviceId_AsyncFunction::OnDeleteDeviceIdComplete);
		}
		else
		{
			OnFailure.Broadcast();
			SetReadyToDestroy();
#if ENGINE_MAJOR_VERSION == 5
			MarkAsGarbage();
#else
			MarkPendingKill();
#endif
		}
	}
	else
	{
		OnFailure.Broadcast();
		SetReadyToDestroy();
#if ENGINE_MAJOR_VERSION == 5
		MarkAsGarbage();
#else
		MarkPendingKill();
#endif
	}
}
