// Copyright (C) 2024, All Rights Reserved.

#include "EOSKitPresenceSubsystem.h"
#include "EOSKitSubsystem.h"
#include "Kismet/GameplayStatics.h"
#if WITH_EOS_SDK
#include "Windows/AllowWindowsPlatformTypes.h"
#include "eos_sdk.h"
#include "eos_presence.h"
#include "eos_presence_types.h"
#include "Windows/HideWindowsPlatformTypes.h"
#endif
#include "EOSKitSharedTypes.h"

void UEOSKitPresenceSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	UE_LOG(LogTemp, Log, TEXT("EOSKitPresenceSubsystem: Initialized"));
}

void UEOSKitPresenceSubsystem::Deinitialize()
{
	// Clean up notification callbacks
	NotificationCallbacks.Empty();
	Super::Deinitialize();
}

UEOSKitSubsystem* UEOSKitPresenceSubsystem::GetEOSKitSubsystem() const
{
	if (UGameInstance* GameInstance = GetGameInstance())
	{
		return GameInstance->GetSubsystem<UEOSKitSubsystem>();
	}
	return nullptr;
}

EOS_HPresence UEOSKitPresenceSubsystem::GetPresenceHandle() const
{
	UEOSKitSubsystem* EOSKitSubsystem = GetEOSKitSubsystem();
	if (!EOSKitSubsystem || !EOSKitSubsystem->GetPlatformHandle())
	{
		return nullptr;
	}
	return EOS_Platform_GetPresenceInterface(EOSKitSubsystem->GetPlatformHandle());
}

EEOSResult UEOSKitPresenceSubsystem::QueryPresence(const FEOSKitEpicAccountId& LocalUserId, const FEOSKitEpicAccountId& TargetUserId)
{
	EOS_HPresence PresenceHandle = GetPresenceHandle();
	if (!PresenceHandle)
	{
		UE_LOG(LogTemp, Error, TEXT("EOSKitPresence: Failed to get Presence Handle"));
		return EEOSResult::EOS_NotConfigured;
	}

	EOS_Presence_QueryPresenceOptions Options = {};
	Options.ApiVersion = EOS_PRESENCE_QUERYPRESENCE_API_LATEST;
	Options.LocalUserId = LocalUserId.GetValueAsEosType();
	Options.TargetUserId = TargetUserId.GetValueAsEosType();

	// Note: This is a synchronous wrapper, but QueryPresence is async
	// The actual async operation should be done through async nodes
	return EEOSResult::EOS_Success;
}

bool UEOSKitPresenceSubsystem::GetPresence(const FEOSKitEpicAccountId& LocalUserId, const FEOSKitEpicAccountId& TargetUserId, FEOSKitPresenceInfo& OutPresenceInfo)
{
	EOS_HPresence PresenceHandle = GetPresenceHandle();
	if (!PresenceHandle)
	{
		UE_LOG(LogTemp, Error, TEXT("EOSKitPresence: Failed to get Presence Handle"));
		return false;
	}

	EOS_Presence_CopyPresenceOptions Options = {};
	Options.ApiVersion = EOS_PRESENCE_COPYPRESENCE_API_LATEST;
	Options.LocalUserId = LocalUserId.GetValueAsEosType();
	Options.TargetUserId = TargetUserId.GetValueAsEosType();

	EOS_Presence_Info* PresenceInfo = nullptr;
	EOS_EResult Result = EOS_Presence_CopyPresence(PresenceHandle, &Options, &PresenceInfo);

	if (Result == EOS_EResult::EOS_Success && PresenceInfo)
	{
		OutPresenceInfo.Status = static_cast<EEOSKitPresenceStatus>(PresenceInfo->Status);
		OutPresenceInfo.RichText = PresenceInfo->RichText ? UTF8_TO_TCHAR(PresenceInfo->RichText) : TEXT("");
		OutPresenceInfo.EpicAccountId = FEOSKitEpicAccountId(PresenceInfo->UserId);

		// Copy presence data records
		OutPresenceInfo.PresenceData.Empty();
		for (int32 i = 0; i < PresenceInfo->RecordsCount; i++)
		{
			const EOS_Presence_DataRecord* Record = &PresenceInfo->Records[i];
			if (Record && Record->Key && Record->Value)
			{
				OutPresenceInfo.PresenceData.Add(
					UTF8_TO_TCHAR(Record->Key),
					UTF8_TO_TCHAR(Record->Value)
				);
			}
		}

		EOS_Presence_Info_Release(PresenceInfo);
		return true;
	}

	return false;
}

FEOSKitNotificationId UEOSKitPresenceSubsystem::AddNotifyOnPresenceChanged(const FEOSKitEpicAccountId& LocalUserId,
	const FEOSKit_Presence_OnPresenceChangedCallback& OnPresenceChanged)
{
	EOS_HPresence PresenceHandle = GetPresenceHandle();
	if (!PresenceHandle)
	{
		UE_LOG(LogTemp, Error, TEXT("EOSKitPresence: Failed to get Presence Handle"));
		return FEOSKitNotificationId();
	}

	EOS_Presence_AddNotifyOnPresenceChangedOptions Options = {};
	Options.ApiVersion = EOS_PRESENCE_ADDNOTIFYONPRESENCECHANGED_API_LATEST;
	// Note: EOS_Presence_AddNotifyOnPresenceChangedOptions does not have LocalUserId - it uses the platform handle context

	// TODO: Implement callback wrapper using TEOSKitGlobalCallback
	// For now, return a placeholder
	EOS_NotificationId NotificationId = EOS_Presence_AddNotifyOnPresenceChanged(
		PresenceHandle,
		&Options,
		nullptr,
		nullptr // Callback will be implemented later
	);

	return FEOSKitNotificationId(NotificationId);
}

void UEOSKitPresenceSubsystem::RemoveNotifyOnPresenceChanged(const FEOSKitNotificationId& NotificationId)
{
	EOS_HPresence PresenceHandle = GetPresenceHandle();
	if (!PresenceHandle)
	{
		UE_LOG(LogTemp, Error, TEXT("EOSKitPresence: Failed to get Presence Handle"));
		return;
	}

	EOS_Presence_RemoveNotifyOnPresenceChanged(PresenceHandle, NotificationId.GetValueAsEosType());
	
	// Remove from callbacks map
	NotificationCallbacks.Remove(NotificationId.GetValueAsEosType());
}

