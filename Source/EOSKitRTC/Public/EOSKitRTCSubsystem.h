// Copyright (C) 2024, All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "EOSKitRTCTypes.h"
#include "EOSKitSharedTypes.h"
#include "EOSKitSubsystem.h"

THIRD_PARTY_INCLUDES_START
#include "eos_rtc.h"
#include "eos_rtc_types.h"
#include "eos_rtc_audio.h"
#include "eos_rtc_audio_types.h"
THIRD_PARTY_INCLUDES_END

// .generated.h must always be the last include
#include "EOSKitRTCSubsystem.generated.h"

/**
 * Delegate for participant status changed (multicast for Blueprint assignment)
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnEOSParticipantStatusChanged, const FEOSKitRTCParticipantStatusChangedInfo&, StatusInfo);

/**
 * Delegate for participant status changed (regular for function parameters)
 */
DECLARE_DYNAMIC_DELEGATE_OneParam(FOnEOSParticipantStatusChangedDelegate, const FEOSKitRTCParticipantStatusChangedInfo&, StatusInfo);

/**
 * Delegate for audio before render (receiving audio) (multicast for Blueprint assignment)
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_FourParams(FOnEOSAudioBeforeRender,
	const FEOSKitProductUserId&, LocalUserId,
	const FEOSKitProductUserId&, ParticipantId,
	const TArray<uint8>&, AudioBuffer,
	int32, SampleRate);

/**
 * Delegate for audio before render (receiving audio) (regular for function parameters)
 */
DECLARE_DYNAMIC_DELEGATE_FourParams(FOnEOSAudioBeforeRenderDelegate,
	const FEOSKitProductUserId&, LocalUserId,
	const FEOSKitProductUserId&, ParticipantId,
	const TArray<uint8>&, AudioBuffer,
	int32, SampleRate);

/**
 * Delegate for audio before send (sending audio) (multicast for Blueprint assignment)
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnEOSAudioBeforeSend,
	const FEOSKitProductUserId&, LocalUserId,
	const TArray<uint8>&, AudioBuffer,
	int32, SampleRate);

/**
 * Delegate for audio before send (sending audio) (regular for function parameters)
 */
DECLARE_DYNAMIC_DELEGATE_ThreeParams(FOnEOSAudioBeforeSendDelegate,
	const FEOSKitProductUserId&, LocalUserId,
	const TArray<uint8>&, AudioBuffer,
	int32, SampleRate);

/**
 * RTC subsystem for managing EOS RTC interface
 */
UCLASS()
class EOSKITRTC_API UEOSKitRTCSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	// USubsystem interface
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	// ========================================
	// SDK Function Wrappers
	// ========================================

	/**
	 * Join room
	 * @param LocalUserId - Product User ID of the local user
	 * @param RoomName - Room name
	 * @param ClientBaseUrl - Client base URL
	 * @param ParticipantToken - Authorization credential token
	 * @param ParticipantId - Participant ID (empty to use LocalUserId)
	 * @param bEnabledEcho - Enable echo mode
	 * @param bManualAudioInputEnabled - Enable manual audio input
	 * @param bManualAudioOutputEnabled - Enable manual audio output
	 * @return Result code (Note: This is async, returns immediately)
	 */
	UFUNCTION(BlueprintCallable, Category = "EOSKit|RTC")
	EEOSResult JoinRoom(const FEOSKitProductUserId& LocalUserId, 
		const FString& RoomName, 
		const FString& ClientBaseUrl, 
		const FString& ParticipantToken, 
		const FEOSKitProductUserId& ParticipantId, 
		bool bEnabledEcho = false, 
		bool bManualAudioInputEnabled = false, 
		bool bManualAudioOutputEnabled = false);

	/**
	 * Leave room
	 * @param LocalUserId - Product User ID of the local user
	 * @param RoomName - Room name
	 * @return Result code (Note: This is async, returns immediately)
	 */
	UFUNCTION(BlueprintCallable, Category = "EOSKit|RTC")
	EEOSResult LeaveRoom(const FEOSKitProductUserId& LocalUserId, const FString& RoomName);

	/**
	 * Block participant
	 * @param LocalUserId - Product User ID of the local user
	 * @param RoomName - Room name
	 * @param ParticipantId - Participant ID to block/unblock
	 * @param bBlocked - True to block, false to unblock
	 * @return Result code (Note: This is async, returns immediately)
	 */
	UFUNCTION(BlueprintCallable, Category = "EOSKit|RTC")
	EEOSResult BlockParticipant(const FEOSKitProductUserId& LocalUserId, 
		const FString& RoomName, 
		const FEOSKitProductUserId& ParticipantId, 
		bool bBlocked);

	/**
	 * Set setting (global)
	 * @param SettingName - Setting name (e.g., "DisableEchoCancelation", "DisableNoiseSupression", "DisableAutoGainControl", "DisableDtx")
	 * @param SettingValue - Setting value (e.g., "True", "False")
	 * @return Result code
	 */
	UFUNCTION(BlueprintCallable, Category = "EOSKit|RTC")
	EEOSResult SetSetting(const FString& SettingName, const FString& SettingValue);

	/**
	 * Set room setting
	 * @param LocalUserId - Product User ID of the local user
	 * @param RoomName - Room name
	 * @param SettingName - Setting name (e.g., "DisableEchoCancelation", "DisableNoiseSupression", "DisableAutoGainControl", "DisableDtx")
	 * @param SettingValue - Setting value (e.g., "True", "False")
	 * @return Result code
	 */
	UFUNCTION(BlueprintCallable, Category = "EOSKit|RTC")
	EEOSResult SetRoomSetting(const FEOSKitProductUserId& LocalUserId, 
		const FString& RoomName, 
		const FString& SettingName, 
		const FString& SettingValue);

	/**
	 * Add notification for participant status changed
	 * @param LocalUserId - Product User ID of the local user
	 * @param RoomName - Room name
	 * @param Callback - Callback delegate
	 * @return Notification ID (use with RemoveNotifyParticipantStatusChanged)
	 */
	UFUNCTION(BlueprintCallable, Category = "EOSKit|RTC")
	int32 AddNotifyParticipantStatusChanged(const FEOSKitProductUserId& LocalUserId, 
		const FString& RoomName, 
		const FOnEOSParticipantStatusChangedDelegate& Callback);

	/**
	 * Remove notification for participant status changed
	 * @param NotificationId - Notification ID returned from AddNotifyParticipantStatusChanged
	 */
	UFUNCTION(BlueprintCallable, Category = "EOSKit|RTC")
	void RemoveNotifyParticipantStatusChanged(int32 NotificationId);

	/**
	 * Add notification for audio before render (receiving audio)
	 * @param LocalUserId - Product User ID of the local user
	 * @param RoomName - Room name
	 * @param Callback - Callback delegate
	 * @return Notification ID (use with RemoveNotifyAudioBeforeRender)
	 */
	UFUNCTION(BlueprintCallable, Category = "EOSKit|RTC")
	int32 AddNotifyAudioBeforeRender(const FEOSKitProductUserId& LocalUserId, 
		const FString& RoomName, 
		const FOnEOSAudioBeforeRenderDelegate& Callback);

	/**
	 * Remove notification for audio before render
	 * @param NotificationId - Notification ID returned from AddNotifyAudioBeforeRender
	 */
	UFUNCTION(BlueprintCallable, Category = "EOSKit|RTC")
	void RemoveNotifyAudioBeforeRender(int32 NotificationId);

	/**
	 * Add notification for audio before send (sending audio)
	 * @param LocalUserId - Product User ID of the local user
	 * @param RoomName - Room name
	 * @param Callback - Callback delegate
	 * @return Notification ID (use with RemoveNotifyAudioBeforeSend)
	 */
	UFUNCTION(BlueprintCallable, Category = "EOSKit|RTC")
	int32 AddNotifyAudioBeforeSend(const FEOSKitProductUserId& LocalUserId, 
		const FString& RoomName, 
		const FOnEOSAudioBeforeSendDelegate& Callback);

	/**
	 * Remove notification for audio before send
	 * @param NotificationId - Notification ID returned from AddNotifyAudioBeforeSend
	 */
	UFUNCTION(BlueprintCallable, Category = "EOSKit|RTC")
	void RemoveNotifyAudioBeforeSend(int32 NotificationId);

	UPROPERTY(BlueprintAssignable, Category = "EOSKit|RTC")
	FOnEOSParticipantStatusChanged OnParticipantStatusChanged;

	UPROPERTY(BlueprintAssignable, Category = "EOSKit|RTC")
	FOnEOSAudioBeforeRender OnAudioBeforeRender;

	UPROPERTY(BlueprintAssignable, Category = "EOSKit|RTC")
	FOnEOSAudioBeforeSend OnAudioBeforeSend;

private:
	UEOSKitSubsystem* GetEOSKitSubsystem() const;
	EOS_HRTC GetRTCHandle() const;
	EOS_HRTCAudio GetRTCAudioHandle() const;

	static void EOS_CALL OnParticipantStatusChangedCallback(const EOS_RTC_ParticipantStatusChangedCallbackInfo* Data);
	static void EOS_CALL OnAudioBeforeRenderCallback(const EOS_RTCAudio_AudioBeforeRenderCallbackInfo* Data);
	static void EOS_CALL OnAudioBeforeSendCallback(const EOS_RTCAudio_AudioBeforeSendCallbackInfo* Data);

	TMap<int32, EOS_NotificationId> ParticipantStatusNotificationIdMap;
	TMap<EOS_NotificationId, FOnEOSParticipantStatusChangedDelegate> ParticipantStatusChangedCallbacks;
	TMap<int32, EOS_NotificationId> AudioBeforeRenderNotificationIdMap;
	TMap<EOS_NotificationId, FOnEOSAudioBeforeRenderDelegate> AudioBeforeRenderCallbacks;
	TMap<int32, EOS_NotificationId> AudioBeforeSendNotificationIdMap;
	TMap<EOS_NotificationId, FOnEOSAudioBeforeSendDelegate> AudioBeforeSendCallbacks;
	int32 NextNotificationId;
};

