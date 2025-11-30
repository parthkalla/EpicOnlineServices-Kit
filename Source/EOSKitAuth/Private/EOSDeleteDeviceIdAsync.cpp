// Copyright (C) 2024, All Rights Reserved.

#include "EOSDeleteDeviceIdAsync.h"
#include "EOSKitSubsystem.h"
#include "Kismet/GameplayStatics.h"
#include "eos_connect.h"
#include "eos_connect_types.h"

UEOSDeleteDeviceIdAsync* UEOSDeleteDeviceIdAsync::DeleteDeviceId(UObject* WorldContextObject)
{
	UEOSDeleteDeviceIdAsync* Node = NewObject<UEOSDeleteDeviceIdAsync>();
	Node->WorldContextObject = WorldContextObject;
	return Node;
}

void UEOSDeleteDeviceIdAsync::Activate()
{
	if (!WorldContextObject)
	{
		OnFailure.Broadcast();
		SetReadyToDestroy();
		return;
	}

	UGameInstance* GameInstance = UGameplayStatics::GetGameInstance(WorldContextObject);
	if (!GameInstance)
	{
		OnFailure.Broadcast();
		SetReadyToDestroy();
		return;
	}

	UEOSKitSubsystem* EOSSubsystem = GameInstance->GetSubsystem<UEOSKitSubsystem>();
	if (!EOSSubsystem || !EOSSubsystem->GetPlatformHandle())
	{
		OnFailure.Broadcast();
		SetReadyToDestroy();
		return;
	}

	EOS_HConnect ConnectHandle = EOS_Platform_GetConnectInterface(EOSSubsystem->GetPlatformHandle());
	if (!ConnectHandle)
	{
		OnFailure.Broadcast();
		SetReadyToDestroy();
		return;
	}

	// Setup options
	EOS_Connect_DeleteDeviceIdOptions Options = {};
	Options.ApiVersion = EOS_CONNECT_DELETEDEVICEID_API_LATEST;

	// Delete device ID
	EOS_Connect_DeleteDeviceId(ConnectHandle, &Options, this, &UEOSDeleteDeviceIdAsync::OnDeleteDeviceIdComplete);
}

void EOS_CALL UEOSDeleteDeviceIdAsync::OnDeleteDeviceIdComplete(const EOS_Connect_DeleteDeviceIdCallbackInfo* Data)
{
	if (!Data || !Data->ClientData)
	{
		return;
	}

	UEOSDeleteDeviceIdAsync* Self = static_cast<UEOSDeleteDeviceIdAsync*>(Data->ClientData);

	if (Data->ResultCode == EOS_EResult::EOS_Success)
	{
		Self->OnSuccess.Broadcast();
	}
	else
	{
		Self->OnFailure.Broadcast();
	}

	Self->SetReadyToDestroy();
}

