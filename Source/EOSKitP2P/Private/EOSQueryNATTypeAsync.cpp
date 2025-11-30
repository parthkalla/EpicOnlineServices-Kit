// Copyright (C) 2024, All Rights Reserved.

#include "EOSQueryNATTypeAsync.h"
#include "EOSKitSubsystem.h"
#include "Kismet/GameplayStatics.h"
#if WITH_EOS_SDK
#include "Windows/AllowWindowsPlatformTypes.h"
#include "Windows/PreWindowsApi.h"
#include "eos_platform.h"
#include "eos_p2p.h"
#include "eos_p2p_types.h"
#include "Windows/PostWindowsApi.h"
#include "Windows/HideWindowsPlatformTypes.h"
#endif

UEOSQueryNATTypeAsync* UEOSQueryNATTypeAsync::QueryNATType(UObject* WorldContextObject)
{
	UEOSQueryNATTypeAsync* Node = NewObject<UEOSQueryNATTypeAsync>();
	Node->WorldContextObject = WorldContextObject;
	return Node;
}

void UEOSQueryNATTypeAsync::Activate()
{
	if (!WorldContextObject)
	{
		OnFailure.Broadcast(EEOSKitNATType::Unknown);
		SetReadyToDestroy();
		return;
	}

	UGameInstance* GameInstance = UGameplayStatics::GetGameInstance(WorldContextObject);
	if (!GameInstance)
	{
		OnFailure.Broadcast(EEOSKitNATType::Unknown);
		SetReadyToDestroy();
		return;
	}

	UEOSKitSubsystem* EOSSubsystem = GameInstance->GetSubsystem<UEOSKitSubsystem>();
	if (!EOSSubsystem || !EOSSubsystem->GetPlatformHandle())
	{
		OnFailure.Broadcast(EEOSKitNATType::Unknown);
		SetReadyToDestroy();
		return;
	}

	EOS_HP2P P2PHandle = EOS_Platform_GetP2PInterface(EOSSubsystem->GetPlatformHandle());
	if (!P2PHandle)
	{
		OnFailure.Broadcast(EEOSKitNATType::Unknown);
		SetReadyToDestroy();
		return;
	}

	// Setup options
	EOS_P2P_QueryNATTypeOptions Options = {};
	Options.ApiVersion = EOS_P2P_QUERYNATTYPE_API_LATEST;

	// Query NAT type
	EOS_P2P_QueryNATType(P2PHandle, &Options, this, &UEOSQueryNATTypeAsync::OnQueryNATTypeComplete);
}

void EOS_CALL UEOSQueryNATTypeAsync::OnQueryNATTypeComplete(const EOS_P2P_OnQueryNATTypeCompleteInfo* Data)
{
	if (!Data || !Data->ClientData)
	{
		return;
	}

	UEOSQueryNATTypeAsync* Self = static_cast<UEOSQueryNATTypeAsync*>(Data->ClientData);

	if (Data->ResultCode == EOS_EResult::EOS_Success)
	{
		EEOSKitNATType NATType = static_cast<EEOSKitNATType>(Data->NATType);
		Self->OnSuccess.Broadcast(NATType);
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("EOSQueryNATTypeAsync: Query failed with result: %d"), (int32)Data->ResultCode);
		Self->OnFailure.Broadcast(EEOSKitNATType::Unknown);
	}

	Self->SetReadyToDestroy();
}

