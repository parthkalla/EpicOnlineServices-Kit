// Copyright (C) 2024, All Rights Reserved.

#include "EOSKitUiSubsystem.h"
#include "EOSKitSubsystem.h"
#include "Kismet/GameplayStatics.h"
#if WITH_EOS_SDK
#include "Windows/AllowWindowsPlatformTypes.h"
#include "eos_sdk.h"
#include "eos_ui.h"
#include "eos_ui_types.h"
#include "Windows/HideWindowsPlatformTypes.h"
#endif
#include "EOSKitSharedTypes.h"
#include "Async/Async.h"

void UEOSKitUiSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	NextNotificationId = 1;
	UE_LOG(LogTemp, Log, TEXT("EOSKitUiSubsystem: Initialized"));
}

void UEOSKitUiSubsystem::Deinitialize()
{
	// Remove all notifications
	EOS_HUI UIHandle = GetUIHandle();

	if (UIHandle)
	{
		for (const auto& Pair : DisplaySettingsNotificationIdMap)
		{
			EOS_UI_RemoveNotifyDisplaySettingsUpdated(UIHandle, Pair.Value);
		}
	}

	DisplaySettingsNotificationIdMap.Empty();

	Super::Deinitialize();
}

UEOSKitSubsystem* UEOSKitUiSubsystem::GetEOSKitSubsystem() const
{
	if (UGameInstance* GameInstance = GetGameInstance())
	{
		return GameInstance->GetSubsystem<UEOSKitSubsystem>();
	}
	return nullptr;
}

EOS_HUI UEOSKitUiSubsystem::GetUIHandle() const
{
	UEOSKitSubsystem* EOSKitSubsystem = GetEOSKitSubsystem();
	if (!EOSKitSubsystem || !EOSKitSubsystem->GetPlatformHandle())
	{
		return nullptr;
	}
	return EOS_Platform_GetUIInterface(EOSKitSubsystem->GetPlatformHandle());
}

EEOSResult UEOSKitUiSubsystem::ShowFriends(const FEOSKitEpicAccountId& LocalUserId)
{
	EOS_HUI UIHandle = GetUIHandle();
	if (!UIHandle)
	{
		UE_LOG(LogTemp, Error, TEXT("EOSKitUi: Failed to get UI Handle"));
		return EEOSResult::EOS_NotConfigured;
	}

	EOS_UI_ShowFriendsOptions Options = {};
	Options.ApiVersion = EOS_UI_SHOWFRIENDS_API_LATEST;
	Options.LocalUserId = LocalUserId.GetValueAsEosType();

	// Note: This is async, but we return immediately
	// The actual callback should be handled by async nodes if needed
	EOS_UI_ShowFriends(UIHandle, &Options, this, &UEOSKitUiSubsystem::OnShowFriendsCallback);

	return EEOSResult::EOS_Success;
}

EEOSResult UEOSKitUiSubsystem::HideFriends(const FEOSKitEpicAccountId& LocalUserId)
{
	EOS_HUI UIHandle = GetUIHandle();
	if (!UIHandle)
	{
		UE_LOG(LogTemp, Error, TEXT("EOSKitUi: Failed to get UI Handle"));
		return EEOSResult::EOS_NotConfigured;
	}

	EOS_UI_HideFriendsOptions Options = {};
	Options.ApiVersion = EOS_UI_HIDEFRIENDS_API_LATEST;
	Options.LocalUserId = LocalUserId.GetValueAsEosType();

	// Note: This is async, but we return immediately
	// The actual callback should be handled by async nodes if needed
	EOS_UI_HideFriends(UIHandle, &Options, this, &UEOSKitUiSubsystem::OnHideFriendsCallback);

	return EEOSResult::EOS_Success;
}

bool UEOSKitUiSubsystem::GetFriendsVisible(const FEOSKitEpicAccountId& LocalUserId) const
{
	EOS_HUI UIHandle = GetUIHandle();
	if (!UIHandle)
	{
		UE_LOG(LogTemp, Error, TEXT("EOSKitUi: Failed to get UI Handle"));
		return false;
	}

	EOS_UI_GetFriendsVisibleOptions Options = {};
	Options.ApiVersion = EOS_UI_GETFRIENDSVISIBLE_API_LATEST;
	Options.LocalUserId = LocalUserId.GetValueAsEosType();

	EOS_Bool bIsVisible = EOS_UI_GetFriendsVisible(UIHandle, &Options);
	return bIsVisible == EOS_TRUE;
}

EEOSResult UEOSKitUiSubsystem::PauseSocialOverlay(bool bIsPaused)
{
	EOS_HUI UIHandle = GetUIHandle();
	if (!UIHandle)
	{
		UE_LOG(LogTemp, Error, TEXT("EOSKitUi: Failed to get UI Handle"));
		return EEOSResult::EOS_NotConfigured;
	}

	EOS_UI_PauseSocialOverlayOptions Options = {};
	Options.ApiVersion = EOS_UI_PAUSESOCIALOVERLAY_API_LATEST;
	Options.bIsPaused = bIsPaused ? EOS_TRUE : EOS_FALSE;

	EOS_EResult Result = EOS_UI_PauseSocialOverlay(UIHandle, &Options);
	return ConvertEOSResultToEEOSResult(Result);
}

EEOSResult UEOSKitUiSubsystem::ResumeSocialOverlay()
{
	// Resume is just pausing with false
	return PauseSocialOverlay(false);
}

bool UEOSKitUiSubsystem::IsSocialOverlayPaused() const
{
	EOS_HUI UIHandle = GetUIHandle();
	if (!UIHandle)
	{
		UE_LOG(LogTemp, Error, TEXT("EOSKitUi: Failed to get UI Handle"));
		return false;
	}

	EOS_UI_IsSocialOverlayPausedOptions Options = {};
	Options.ApiVersion = EOS_UI_ISSOCIALOVERLAYPAUSED_API_LATEST;

	EOS_Bool bIsPaused = EOS_UI_IsSocialOverlayPaused(UIHandle, &Options);
	return bIsPaused == EOS_TRUE;
}

int32 UEOSKitUiSubsystem::AddNotifyDisplaySettingsUpdated(const FOnEOSDisplaySettingsUpdatedDelegate& Callback)
{
	EOS_HUI UIHandle = GetUIHandle();
	if (!UIHandle)
	{
		UE_LOG(LogTemp, Error, TEXT("EOSKitUi: Failed to get UI Handle"));
		return 0;
	}

	EOS_UI_AddNotifyDisplaySettingsUpdatedOptions Options = {};
	Options.ApiVersion = EOS_UI_ADDNOTIFYDISPLAYSETTINGSUPDATED_API_LATEST;

	EOS_NotificationId NotificationId = EOS_UI_AddNotifyDisplaySettingsUpdated(
		UIHandle,
		&Options,
		this,
		&UEOSKitUiSubsystem::OnDisplaySettingsUpdatedCallback);

	if (NotificationId != EOS_INVALID_NOTIFICATIONID)
	{
		int32 OurNotificationId = NextNotificationId++;
		DisplaySettingsNotificationIdMap.Add(OurNotificationId, NotificationId);
		
		// Store the callback delegate with EOS notification ID as key
		DisplaySettingsUpdatedCallbacks.Add(NotificationId, Callback);
		
		return OurNotificationId;
	}

	return 0;
}

void UEOSKitUiSubsystem::RemoveNotifyDisplaySettingsUpdated(int32 NotificationId)
{
	EOS_NotificationId* EOSNotificationId = DisplaySettingsNotificationIdMap.Find(NotificationId);
	if (EOSNotificationId)
	{
		EOS_HUI UIHandle = GetUIHandle();
		if (UIHandle)
		{
			EOS_UI_RemoveNotifyDisplaySettingsUpdated(UIHandle, *EOSNotificationId);
		}
		DisplaySettingsUpdatedCallbacks.Remove(*EOSNotificationId);
		DisplaySettingsNotificationIdMap.Remove(NotificationId);
	}
}

void EOS_CALL UEOSKitUiSubsystem::OnShowFriendsCallback(const EOS_UI_ShowFriendsCallbackInfo* Data)
{
	if (!Data || !Data->ClientData)
	{
		return;
	}

	UEOSKitUiSubsystem* Self = static_cast<UEOSKitUiSubsystem*>(Data->ClientData);
	EEOSResult Result = ConvertEOSResultToEEOSResult(Data->ResultCode);

	if (Result != EEOSResult::EOS_Success)
	{
		UE_LOG(LogTemp, Warning, TEXT("EOSKitUi: ShowFriends failed with result: %d"), static_cast<int32>(Result));
	}
}

void EOS_CALL UEOSKitUiSubsystem::OnHideFriendsCallback(const EOS_UI_HideFriendsCallbackInfo* Data)
{
	if (!Data || !Data->ClientData)
	{
		return;
	}

	UEOSKitUiSubsystem* Self = static_cast<UEOSKitUiSubsystem*>(Data->ClientData);
	EEOSResult Result = ConvertEOSResultToEEOSResult(Data->ResultCode);

	if (Result != EEOSResult::EOS_Success)
	{
		UE_LOG(LogTemp, Warning, TEXT("EOSKitUi: HideFriends failed with result: %d"), static_cast<int32>(Result));
	}
}

void EOS_CALL UEOSKitUiSubsystem::OnDisplaySettingsUpdatedCallback(const EOS_UI_OnDisplaySettingsUpdatedCallbackInfo* Data)
{
	if (!Data || !Data->ClientData)
	{
		return;
	}

	UEOSKitUiSubsystem* Self = static_cast<UEOSKitUiSubsystem*>(Data->ClientData);

	bool bIsVisible = Data->bIsVisible == EOS_TRUE;
	bool bIsExclusiveInput = Data->bIsExclusiveInput == EOS_TRUE;

	// Find and call the stored callback delegate
	// Note: EOS_UI_OnDisplaySettingsUpdatedCallbackInfo does not have NotificationId
	// We need to call all registered callbacks since we can't identify which specific one to call
	for (auto& Pair : Self->DisplaySettingsUpdatedCallbacks)
	{
		FOnEOSDisplaySettingsUpdatedDelegate CallbackCopy = Pair.Value;
		AsyncTask(ENamedThreads::GameThread, [CallbackCopy, bIsVisible, bIsExclusiveInput]()
		{
			CallbackCopy.ExecuteIfBound(bIsVisible, bIsExclusiveInput);
		});
	}

	// Broadcast to multicast delegate
	AsyncTask(ENamedThreads::GameThread, [Self, bIsVisible, bIsExclusiveInput]()
	{
		Self->OnDisplaySettingsUpdated.Broadcast(bIsVisible, bIsExclusiveInput);
	});
}

