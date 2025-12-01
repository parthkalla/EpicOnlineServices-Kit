// Copyright (C) 2024, All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#if WITH_EOS_RTC

#include "eos_rtc_types.h"

class UEOSKitVoiceSubsystem;
class UEOSVoiceChatSynthComponent;
class FEOSVoiceChatUser;
class FEOSAudioDevicePool;

/**
 * EOS Voice Chat manager - handles RTC callbacks and audio routing
 */
class EOSKITRTC_API FEOSVoiceChat : public TSharedFromThis<FEOSVoiceChat, ESPMode::ThreadSafe>
{
public:
	FEOSVoiceChat();
	virtual ~FEOSVoiceChat();

	// Internal subsystem methods
	void Initialize(UEOSKitVoiceSubsystem* InSubsystem);
	void Shutdown();
	void OnRoomJoined(const FString& RoomName);
	void OnRoomLeft(const FString& RoomName);
	void SetLocalMuted(bool bMuted);
	UEOSVoiceChatSynthComponent* GetOrCreateSynthComponent(const FString& UserId);
	void RemoveSynthComponent(const FString& UserId);
	void HandleParticipantJoined(const FString& RoomName, const FString& UserId);
	void HandleParticipantLeft(const FString& RoomName, const FString& UserId);
	void HandleAudioData(const FString& RoomName, const FString& UserId, const float* AudioBuffer, int32 NumSamples, int32 SampleRate, int32 NumChannels);

	// Audio device pool access
	TSharedPtr<FEOSAudioDevicePool> AudioDevicePool;

protected:
	friend class FEOSVoiceChatUser;

	void RegisterNotifications();
	void UnregisterNotifications();

	UEOSKitVoiceSubsystem* OwningSubsystem = nullptr;
	TMap<FString, UEOSVoiceChatSynthComponent*> UserSynthComponents;

	void* ParticipantStatusNotificationId = nullptr;
	void* AudioBeforeRenderNotificationId = nullptr;
	void* AudioInputStateNotificationId = nullptr;

	FString CurrentRoomName;
	bool bIsInitialized = false;
	bool bIsLocalMuted = false;
};

#endif // WITH_EOS_RTC
