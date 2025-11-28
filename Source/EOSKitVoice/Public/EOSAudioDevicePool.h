// Copyright (C) 2024, All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "VoiceChat.h"

#if WITH_EOS_RTC

#include "eos_rtc_audio_types.h"

/**
 * Manages EOS audio input and output devices
 */
class EOSKITVOICE_API FEOSAudioDevicePool
{
public:
	FEOSAudioDevicePool(EOS_HRTCAudio InRTCAudioInterface);
	~FEOSAudioDevicePool();

	DECLARE_DELEGATE_OneParam(FOnAudioDevicePoolRefreshAudioDevicesCompleteDelegate, const FVoiceChatResult& /* Result */);

	/**
	 * Refresh the list of available audio devices
	 */
	void RefreshAudioDevices(const FOnAudioDevicePoolRefreshAudioDevicesCompleteDelegate& Delegate);

	/**
	 * Get available input devices
	 */
	TArray<FVoiceChatDeviceInfo> GetCachedInputDeviceInfos() const;

	/**
	 * Get available output devices
	 */
	TArray<FVoiceChatDeviceInfo> GetCachedOutputDeviceInfos() const;

	/**
	 * Get default input device
	 */
	FVoiceChatDeviceInfo GetDefaultInputDeviceInfo() const;

	/**
	 * Get default output device
	 */
	FVoiceChatDeviceInfo GetDefaultOutputDeviceInfo() const;

	/**
	 * Get input device by ID
	 */
	FVoiceChatDeviceInfo GetInputDeviceInfoFromId(const FString& DeviceId) const;

	/**
	 * Get output device by ID
	 */
	FVoiceChatDeviceInfo GetOutputDeviceInfoFromId(const FString& DeviceId) const;

protected:
	EOS_HRTCAudio RTCAudioInterface;

	TArray<FVoiceChatDeviceInfo> CachedInputDeviceInfos;
	TArray<FVoiceChatDeviceInfo> CachedOutputDeviceInfos;

	FVoiceChatDeviceInfo DefaultInputDeviceInfo;
	FVoiceChatDeviceInfo DefaultOutputDeviceInfo;

	mutable FCriticalSection DevicesCriticalSection;
};

#endif // WITH_EOS_RTC
