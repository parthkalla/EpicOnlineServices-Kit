// Copyright (C) 2024, All Rights Reserved.

#include "EOSKitSanctionsSubsystem.h"
#include "EOSKitSubsystem.h"
#include "Kismet/GameplayStatics.h"
#if WITH_EOS_SDK
#include "Windows/AllowWindowsPlatformTypes.h"
#include "Windows/PreWindowsApi.h"
#include "eos_platform.h"
#include "eos_sanctions.h"
#include "eos_sanctions_types.h"
#include "Windows/PostWindowsApi.h"
#include "Windows/HideWindowsPlatformTypes.h"
#endif
#include "EOSKitSharedTypes.h"

void UEOSKitSanctionsSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	UE_LOG(LogTemp, Log, TEXT("EOSKitSanctionsSubsystem: Initialized"));
}

void UEOSKitSanctionsSubsystem::Deinitialize()
{
	Super::Deinitialize();
}

UEOSKitSubsystem* UEOSKitSanctionsSubsystem::GetEOSKitSubsystem() const
{
	if (UGameInstance* GameInstance = GetGameInstance())
	{
		return GameInstance->GetSubsystem<UEOSKitSubsystem>();
	}
	return nullptr;
}

EOS_HSanctions UEOSKitSanctionsSubsystem::GetSanctionsHandle() const
{
	UEOSKitSubsystem* EOSKitSubsystem = GetEOSKitSubsystem();
	if (!EOSKitSubsystem || !EOSKitSubsystem->GetPlatformHandle())
	{
		return nullptr;
	}
	return EOS_Platform_GetSanctionsInterface(EOSKitSubsystem->GetPlatformHandle());
}

EEOSResult UEOSKitSanctionsSubsystem::QueryActivePlayerSanctions(const FEOSKitProductUserId& LocalUserId, const FEOSKitProductUserId& TargetUserId)
{
	EOS_HSanctions SanctionsHandle = GetSanctionsHandle();
	if (!SanctionsHandle)
	{
		UE_LOG(LogTemp, Error, TEXT("EOSKitSanctions: Failed to get Sanctions Handle"));
		return EEOSResult::EOS_NotConfigured;
	}

	EOS_Sanctions_QueryActivePlayerSanctionsOptions Options = {};
	Options.ApiVersion = EOS_SANCTIONS_QUERYACTIVEPLAYERSANCTIONS_API_LATEST;
	Options.TargetUserId = TargetUserId.GetValueAsEosType();
	Options.LocalUserId = LocalUserId.IsValid() ? LocalUserId.GetValueAsEosType() : nullptr;

	// Note: This is async, but we return immediately
	// The actual callback should be handled by async nodes if needed
	EOS_Sanctions_QueryActivePlayerSanctions(SanctionsHandle, &Options, this, &UEOSKitSanctionsSubsystem::OnQueryActivePlayerSanctionsCallback);

	return EEOSResult::EOS_Success;
}

int32 UEOSKitSanctionsSubsystem::GetPlayerSanctionCount(const FEOSKitProductUserId& TargetUserId) const
{
	EOS_HSanctions SanctionsHandle = GetSanctionsHandle();
	if (!SanctionsHandle)
	{
		UE_LOG(LogTemp, Error, TEXT("EOSKitSanctions: Failed to get Sanctions Handle"));
		return -1;
	}

	EOS_Sanctions_GetPlayerSanctionCountOptions Options = {};
	Options.ApiVersion = EOS_SANCTIONS_GETPLAYERSANCTIONCOUNT_API_LATEST;
	Options.TargetUserId = TargetUserId.GetValueAsEosType();

	uint32_t Count = EOS_Sanctions_GetPlayerSanctionCount(SanctionsHandle, &Options);
	return static_cast<int32>(Count);
}

EEOSResult UEOSKitSanctionsSubsystem::CopyPlayerSanctionByIndex(const FEOSKitProductUserId& TargetUserId, int32 SanctionIndex, FEOSKitPlayerSanction& OutSanction) const
{
	EOS_HSanctions SanctionsHandle = GetSanctionsHandle();
	if (!SanctionsHandle)
	{
		UE_LOG(LogTemp, Error, TEXT("EOSKitSanctions: Failed to get Sanctions Handle"));
		return EEOSResult::EOS_NotConfigured;
	}

	EOS_Sanctions_CopyPlayerSanctionByIndexOptions Options = {};
	Options.ApiVersion = EOS_SANCTIONS_COPYPLAYERSANCTIONBYINDEX_API_LATEST;
	Options.TargetUserId = TargetUserId.GetValueAsEosType();
	Options.SanctionIndex = static_cast<uint32_t>(SanctionIndex);

	EOS_Sanctions_PlayerSanction* OutSanctionPtr = nullptr;
	EOS_EResult Result = EOS_Sanctions_CopyPlayerSanctionByIndex(SanctionsHandle, &Options, &OutSanctionPtr);

	if (Result == EOS_EResult::EOS_Success && OutSanctionPtr)
	{
		// Copy the data
		OutSanction.TimePlaced = OutSanctionPtr->TimePlaced;
		OutSanction.Action = OutSanctionPtr->Action ? UTF8_TO_TCHAR(OutSanctionPtr->Action) : TEXT("");
		OutSanction.TimeExpires = OutSanctionPtr->TimeExpires;
		OutSanction.ReferenceId = OutSanctionPtr->ReferenceId ? UTF8_TO_TCHAR(OutSanctionPtr->ReferenceId) : TEXT("");
		OutSanction.bIsPermanent = (OutSanctionPtr->TimeExpires == 0);

		// Release the memory
		EOS_Sanctions_PlayerSanction_Release(OutSanctionPtr);
	}

	return ConvertEOSResultToEEOSResult(Result);
}

EEOSResult UEOSKitSanctionsSubsystem::CreatePlayerSanctionAppeal(const FEOSKitProductUserId& LocalUserId, 
	const FString& ReferenceId, 
	EEOSKitSanctionAppealReason Reason)
{
	EOS_HSanctions SanctionsHandle = GetSanctionsHandle();
	if (!SanctionsHandle)
	{
		UE_LOG(LogTemp, Error, TEXT("EOSKitSanctions: Failed to get Sanctions Handle"));
		return EEOSResult::EOS_NotConfigured;
	}

	EOS_Sanctions_CreatePlayerSanctionAppealOptions Options = {};
	Options.ApiVersion = EOS_SANCTIONS_CREATEPLAYERSANCTIONAPPEAL_API_LATEST;
	Options.LocalUserId = LocalUserId.GetValueAsEosType();
	Options.ReferenceId = TCHAR_TO_UTF8(*ReferenceId);
	Options.Reason = static_cast<EOS_ESanctionAppealReason>(Reason);

	// Note: This is async, but we return immediately
	// The actual callback should be handled by async nodes if needed
	EOS_Sanctions_CreatePlayerSanctionAppeal(SanctionsHandle, &Options, this, &UEOSKitSanctionsSubsystem::OnCreatePlayerSanctionAppealCallback);

	return EEOSResult::EOS_Success;
}

void EOS_CALL UEOSKitSanctionsSubsystem::OnQueryActivePlayerSanctionsCallback(const EOS_Sanctions_QueryActivePlayerSanctionsCallbackInfo* Data)
{
	if (!Data || !Data->ClientData)
	{
		return;
	}

	UEOSKitSanctionsSubsystem* Self = static_cast<UEOSKitSanctionsSubsystem*>(Data->ClientData);
	EEOSResult Result = ConvertEOSResultToEEOSResult(Data->ResultCode);

	FEOSKitProductUserId LocalUserId(Data->LocalUserId);
	FEOSKitProductUserId TargetUserId(Data->TargetUserId);

	// Broadcast to delegates
	Self->OnQueryActivePlayerSanctionsComplete.Broadcast(LocalUserId, TargetUserId, Result);

	if (Result != EEOSResult::EOS_Success)
	{
		UE_LOG(LogTemp, Warning, TEXT("EOSKitSanctions: QueryActivePlayerSanctions failed with result: %d"), static_cast<int32>(Result));
	}
}

void EOS_CALL UEOSKitSanctionsSubsystem::OnCreatePlayerSanctionAppealCallback(const EOS_Sanctions_CreatePlayerSanctionAppealCallbackInfo* Data)
{
	if (!Data || !Data->ClientData)
	{
		return;
	}

	UEOSKitSanctionsSubsystem* Self = static_cast<UEOSKitSanctionsSubsystem*>(Data->ClientData);
	EEOSResult Result = ConvertEOSResultToEEOSResult(Data->ResultCode);

	if (Result == EEOSResult::EOS_Success)
	{
		UE_LOG(LogTemp, Log, TEXT("EOSKitSanctions: CreatePlayerSanctionAppeal succeeded for ReferenceId: %s"), 
			Data->ReferenceId ? UTF8_TO_TCHAR(Data->ReferenceId) : TEXT(""));
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("EOSKitSanctions: CreatePlayerSanctionAppeal failed with result: %d"), static_cast<int32>(Result));
	}
}

