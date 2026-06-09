// Copyright (c) 2023 Betide Studio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "OnlineSubsystemEOS.h"
#include "OnlineSubsystemEOK/SdkFunctions/EOK_SharedFunctionFile.h"
THIRD_PARTY_INCLUDES_START
#include "eos_ui.h"
#include "eos_ui_types.h"
THIRD_PARTY_INCLUDES_END
#include "EOK_UiSubsystem.generated.h"

DECLARE_DYNAMIC_DELEGATE_TwoParams(FEOK_OnDisplaySettingsUpdated, bool, bIsVisible, bool, bIsExclusiveInput);
DECLARE_DYNAMIC_DELEGATE(FEOK_OnMemoryMonitor);
UCLASS()
class OnlineSubsystemEOK_API UEOK_UiSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	//Lets the SDK know that the given UI event ID has been acknowledged and should be released. EOS_Success is returned if the UI event ID has been acknowledged. EOS_NotFound is returned if the UI event ID does not exist.
	UFUNCTION(BlueprintCallable, Category = "Epic Online Services-Kit V2 | SDK Functions | UI Interface", DisplayName="EOS_UI_AcknowledgeEventId")
	static TEnumAsByte<EEOK_Result> EOK_UI_AcknowledgeEventId(const FEOK_UI_EventId& UiEventId, const TEnumAsByte<EEOK_Result>& Result);

	FEOK_OnDisplaySettingsUpdated OnDisplaySettingsUpdated;
	//Register to receive notifications when the overlay display settings are updated. Newly registered handlers will always be called the next tick with the current state.
	UFUNCTION(BlueprintCallable, Category = "Epic Online Services-Kit V2 | SDK Functions | UI Interface", DisplayName="EOS_UI_AddNotifyDisplaySettingsUpdated")
	FEOK_NotificationId EOK_UI_AddNotifyDisplaySettingsUpdated(FEOK_OnDisplaySettingsUpdated Callback);

	FEOK_OnMemoryMonitor OnMemoryMonitor;
	//Register to receive notifications from the memory monitor. Newly registered handlers will always be called the next tick with the current state.
	UFUNCTION(BlueprintCallable, Category = "Epic Online Services-Kit V2 | SDK Functions | UI Interface", DisplayName="EOS_UI_AddNotifyMemoryMonitor")
	FEOK_NotificationId EOK_UI_AddNotifyMemoryMonitor(FEOK_OnMemoryMonitor Callback);

	//Gets the friends overlay exclusive input state.
	UFUNCTION(BlueprintCallable, Category = "Epic Online Services-Kit V2 | SDK Functions | UI Interface", DisplayName="EOS_UI_GetFriendsExclusiveInput")
	static bool EOK_UI_GetFriendsExclusiveInput(FEOK_EpicAccountId LocalUserId);

	//Gets the friends overlay visibility.
	UFUNCTION(BlueprintCallable, Category = "Epic Online Services-Kit V2 | SDK Functions | UI Interface", DisplayName="EOS_UI_GetFriendsVisible")
	static bool EOK_UI_GetFriendsVisible(FEOK_EpicAccountId LocalUserId);
	
	//Returns the current notification location display preference.
	UFUNCTION(BlueprintCallable, Category = "Epic Online Services-Kit V2 | SDK Functions | UI Interface", DisplayName="EOS_UI_GetNotificationLocationPreference")
	static TEnumAsByte<EEOK_UI_ENotificationLocation> EOK_UI_GetNotificationLocationPreference();

	//Sets the bIsPaused state of the overlay. While true then all notifications will be delayed until after the bIsPaused is false again. While true then the key and button events will not toggle the overlay. If the Overlay was visible before being paused then it will be hidden. If it is known that the Overlay should now be visible after being paused then it will be shown.
	UFUNCTION(BlueprintCallable, Category = "Epic Online Services-Kit V2 | SDK Functions | UI Interface", DisplayName="EOS_UI_PauseSocialOverlay")
	TEnumAsByte<EEOK_Result> EOK_UI_PauseSocialOverlay(bool bIsPaused);

	//Unregister from receiving notifications when the overlay display settings are updated.
	UFUNCTION(BlueprintCallable, Category = "Epic Online Services-Kit V2 | SDK Functions | UI Interface", DisplayName="EOS_UI_RemoveNotifyDisplaySettingsUpdated")
	void EOK_UI_RemoveNotifyDisplaySettingsUpdated(FEOK_NotificationId NotificationId);

	//Unregister from receiving notifications when the memory monitor posts a notification.
	UFUNCTION(BlueprintCallable, Category = "Epic Online Services-Kit V2 | SDK Functions | UI Interface", DisplayName="EOS_UI_RemoveNotifyMemoryMonitor")
	void EOK_UI_RemoveNotifyMemoryMonitor(FEOK_NotificationId NotificationId);

	
	//Define any preferences for any display settings.
	UFUNCTION(BlueprintCallable, Category = "Epic Online Services-Kit V2 | SDK Functions | UI Interface", DisplayName="EOS_UI_SetDisplayPreference")
	TEnumAsByte<EEOK_Result> EOK_UI_SetDisplayPreference(const EEOK_UI_ENotificationLocation& DisplayPreference);
};
