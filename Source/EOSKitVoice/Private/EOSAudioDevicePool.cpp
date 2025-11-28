// Copyright (C) 2024, All Rights Reserved.

#include "EOSAudioDevicePool.h"

#if WITH_EOS_RTC

#include "eos_rtc.h"
#include "eos_rtc_audio.h"
#include "VoiceChatErrors.h"
#include "VoiceChatResult.h"

FEOSAudioDevicePool::FEOSAudioDevicePool(EOS_HRTCAudio InRTCAudioInterface)
	: RTCAudioInterface(InRTCAudioInterface)
{
}

FEOSAudioDevicePool::~FEOSAudioDevicePool()
{
}

void FEOSAudioDevicePool::RefreshAudioDevices(const FOnAudioDevicePoolRefreshAudioDevicesCompleteDelegate& Delegate)
{
	if (!RTCAudioInterface)
	{
		Delegate.ExecuteIfBound(VoiceChat::Errors::NotInitialized());
		return;
	}

	FScopeLock Lock(&DevicesCriticalSection);

	// Query input devices
	EOS_RTCAudio_GetAudioInputDevicesCountOptions InputCountOptions = {};
	InputCountOptions.ApiVersion = EOS_RTCAUDIO_GETAUDIOINPUTDEVICESCOUNT_API_LATEST;
	uint32_t InputDeviceCount = EOS_RTCAudio_GetAudioInputDevicesCount(RTCAudioInterface, &InputCountOptions);

	CachedInputDeviceInfos.Empty(InputDeviceCount);
	for (uint32_t i = 0; i < InputDeviceCount; ++i)
	{
		EOS_RTCAudio_GetAudioInputDeviceByIndexOptions GetInputOptions = {};
		GetInputOptions.ApiVersion = EOS_RTCAUDIO_GETAUDIOINPUTDEVICEBYINDEX_API_LATEST;
		GetInputOptions.DeviceInfoIndex = i;

		// SDK 1.18: This function returns the device info directly, not through a pointer
		const EOS_RTCAudio_AudioInputDeviceInfo* DeviceInfo = 
			EOS_RTCAudio_GetAudioInputDeviceByIndex(RTCAudioInterface, &GetInputOptions);

		if (DeviceInfo)
		{
			FVoiceChatDeviceInfo Info;
			Info.DisplayName = FString(UTF8_TO_TCHAR(DeviceInfo->DeviceName));
			Info.Id = FString(UTF8_TO_TCHAR(DeviceInfo->DeviceId));
			
			if (DeviceInfo->bDefaultDevice == EOS_TRUE)
			{
				DefaultInputDeviceInfo = Info;
			}

			CachedInputDeviceInfos.Add(Info);
		}
	}

	// Query output devices
	EOS_RTCAudio_GetAudioOutputDevicesCountOptions OutputCountOptions = {};
	OutputCountOptions.ApiVersion = EOS_RTCAUDIO_GETAUDIOOUTPUTDEVICESCOUNT_API_LATEST;
	uint32_t OutputDeviceCount = EOS_RTCAudio_GetAudioOutputDevicesCount(RTCAudioInterface, &OutputCountOptions);

	CachedOutputDeviceInfos.Empty(OutputDeviceCount);
	for (uint32_t i = 0; i < OutputDeviceCount; ++i)
	{
		EOS_RTCAudio_GetAudioOutputDeviceByIndexOptions GetOutputOptions = {};
		GetOutputOptions.ApiVersion = EOS_RTCAUDIO_GETAUDIOOUTPUTDEVICEBYINDEX_API_LATEST;
		GetOutputOptions.DeviceInfoIndex = i;

		// SDK 1.18: This function returns the device info directly, not through a pointer
		const EOS_RTCAudio_AudioOutputDeviceInfo* DeviceInfo = 
			EOS_RTCAudio_GetAudioOutputDeviceByIndex(RTCAudioInterface, &GetOutputOptions);

		if (DeviceInfo)
		{
			FVoiceChatDeviceInfo Info;
			Info.DisplayName = FString(UTF8_TO_TCHAR(DeviceInfo->DeviceName));
			Info.Id = FString(UTF8_TO_TCHAR(DeviceInfo->DeviceId));
			
			if (DeviceInfo->bDefaultDevice == EOS_TRUE)
			{
				DefaultOutputDeviceInfo = Info;
			}

			CachedOutputDeviceInfos.Add(Info);
		}
	}

	Delegate.ExecuteIfBound(FVoiceChatResult::CreateSuccess());
}

TArray<FVoiceChatDeviceInfo> FEOSAudioDevicePool::GetCachedInputDeviceInfos() const
{
	FScopeLock Lock(&DevicesCriticalSection);
	return CachedInputDeviceInfos;
}

TArray<FVoiceChatDeviceInfo> FEOSAudioDevicePool::GetCachedOutputDeviceInfos() const
{
	FScopeLock Lock(&DevicesCriticalSection);
	return CachedOutputDeviceInfos;
}

FVoiceChatDeviceInfo FEOSAudioDevicePool::GetDefaultInputDeviceInfo() const
{
	FScopeLock Lock(&DevicesCriticalSection);
	return DefaultInputDeviceInfo;
}

FVoiceChatDeviceInfo FEOSAudioDevicePool::GetDefaultOutputDeviceInfo() const
{
	FScopeLock Lock(&DevicesCriticalSection);
	return DefaultOutputDeviceInfo;
}

FVoiceChatDeviceInfo FEOSAudioDevicePool::GetInputDeviceInfoFromId(const FString& DeviceId) const
{
	FScopeLock Lock(&DevicesCriticalSection);
	for (const FVoiceChatDeviceInfo& Info : CachedInputDeviceInfos)
	{
		if (Info.Id == DeviceId)
		{
			return Info;
		}
	}
	return FVoiceChatDeviceInfo();
}

FVoiceChatDeviceInfo FEOSAudioDevicePool::GetOutputDeviceInfoFromId(const FString& DeviceId) const
{
	FScopeLock Lock(&DevicesCriticalSection);
	for (const FVoiceChatDeviceInfo& Info : CachedOutputDeviceInfos)
	{
		if (Info.Id == DeviceId)
		{
			return Info;
		}
	}
	return FVoiceChatDeviceInfo();
}

#endif // WITH_EOS_RTC
