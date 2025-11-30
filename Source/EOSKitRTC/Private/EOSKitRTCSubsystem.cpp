// Copyright (C) 2024, All Rights Reserved.

#include "EOSKitRTCSubsystem.h"
#include "EOSKitSubsystem.h"
#include "Kismet/GameplayStatics.h"
#if WITH_EOS_SDK
#include "Windows/AllowWindowsPlatformTypes.h"
#include "Windows/PreWindowsApi.h"
#include "eos_platform.h"
#include "eos_rtc.h"
#include "eos_rtc_types.h"
#include "eos_rtc_audio.h"
#include "eos_rtc_audio_types.h"
#include "Windows/PostWindowsApi.h"
#include "Windows/HideWindowsPlatformTypes.h"
#endif
#include "EOSKitSharedTypes.h"
#include "Async/Async.h"

void UEOSKitRTCSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	NextNotificationId = 1;
	UE_LOG(LogTemp, Log, TEXT("EOSKitRTCSubsystem: Initialized"));
}

void UEOSKitRTCSubsystem::Deinitialize()
{
	// Remove all notifications
	EOS_HRTC RTCHandle = GetRTCHandle();
	EOS_HRTCAudio RTCAudioHandle = GetRTCAudioHandle();

	if (RTCHandle)
	{
		for (const auto& Pair : ParticipantStatusNotificationIdMap)
		{
			EOS_RTC_RemoveNotifyParticipantStatusChanged(RTCHandle, Pair.Value);
		}
	}

	if (RTCAudioHandle)
	{
		for (const auto& Pair : AudioBeforeRenderNotificationIdMap)
		{
			EOS_RTCAudio_RemoveNotifyAudioBeforeRender(RTCAudioHandle, Pair.Value);
		}
		for (const auto& Pair : AudioBeforeSendNotificationIdMap)
		{
			EOS_RTCAudio_RemoveNotifyAudioBeforeSend(RTCAudioHandle, Pair.Value);
		}
	}

	ParticipantStatusNotificationIdMap.Empty();
	AudioBeforeRenderNotificationIdMap.Empty();
	AudioBeforeSendNotificationIdMap.Empty();

	Super::Deinitialize();
}

UEOSKitSubsystem* UEOSKitRTCSubsystem::GetEOSKitSubsystem() const
{
	if (UGameInstance* GameInstance = GetGameInstance())
	{
		return GameInstance->GetSubsystem<UEOSKitSubsystem>();
	}
	return nullptr;
}

EOS_HRTC UEOSKitRTCSubsystem::GetRTCHandle() const
{
	UEOSKitSubsystem* EOSKitSubsystem = GetEOSKitSubsystem();
	if (!EOSKitSubsystem || !EOSKitSubsystem->GetPlatformHandle())
	{
		return nullptr;
	}
	return EOS_Platform_GetRTCInterface(EOSKitSubsystem->GetPlatformHandle());
}

EOS_HRTCAudio UEOSKitRTCSubsystem::GetRTCAudioHandle() const
{
	EOS_HRTC RTCHandle = GetRTCHandle();
	if (!RTCHandle)
	{
		return nullptr;
	}
	return EOS_RTC_GetAudioInterface(RTCHandle);
}

EEOSResult UEOSKitRTCSubsystem::JoinRoom(const FEOSKitProductUserId& LocalUserId, 
	const FString& RoomName, 
	const FString& ClientBaseUrl, 
	const FString& ParticipantToken, 
	const FEOSKitProductUserId& ParticipantId, 
	bool bEnabledEcho, 
	bool bManualAudioInputEnabled, 
	bool bManualAudioOutputEnabled)
{
	EOS_HRTC RTCHandle = GetRTCHandle();
	if (!RTCHandle)
	{
		UE_LOG(LogTemp, Error, TEXT("EOSKitRTC: Failed to get RTC Handle"));
		return EEOSResult::EOS_NotConfigured;
	}

	EOS_RTC_JoinRoomOptions Options = {};
	Options.ApiVersion = EOS_RTC_JOINROOM_API_LATEST;
	Options.LocalUserId = LocalUserId.GetValueAsEosType();
	Options.RoomName = TCHAR_TO_UTF8(*RoomName);
	Options.ClientBaseUrl = TCHAR_TO_UTF8(*ClientBaseUrl);
	Options.ParticipantToken = TCHAR_TO_UTF8(*ParticipantToken);
	Options.ParticipantId = ParticipantId.IsValid() ? ParticipantId.GetValueAsEosType() : LocalUserId.GetValueAsEosType();
	Options.Flags = bEnabledEcho ? EOS_RTC_JOINROOMFLAGS_ENABLE_ECHO : 0x0;
	Options.bManualAudioInputEnabled = bManualAudioInputEnabled ? EOS_TRUE : EOS_FALSE;
	Options.bManualAudioOutputEnabled = bManualAudioOutputEnabled ? EOS_TRUE : EOS_FALSE;

	// Note: This is a synchronous wrapper, but JoinRoom is async
	// The actual async operation should be done through async nodes
	return EEOSResult::EOS_Success;
}

EEOSResult UEOSKitRTCSubsystem::LeaveRoom(const FEOSKitProductUserId& LocalUserId, const FString& RoomName)
{
	EOS_HRTC RTCHandle = GetRTCHandle();
	if (!RTCHandle)
	{
		UE_LOG(LogTemp, Error, TEXT("EOSKitRTC: Failed to get RTC Handle"));
		return EEOSResult::EOS_NotConfigured;
	}

	EOS_RTC_LeaveRoomOptions Options = {};
	Options.ApiVersion = EOS_RTC_LEAVEROOM_API_LATEST;
	Options.LocalUserId = LocalUserId.GetValueAsEosType();
	Options.RoomName = TCHAR_TO_UTF8(*RoomName);

	// Note: This is a synchronous wrapper, but LeaveRoom is async
	// The actual async operation should be done through async nodes
	return EEOSResult::EOS_Success;
}

EEOSResult UEOSKitRTCSubsystem::BlockParticipant(const FEOSKitProductUserId& LocalUserId, 
	const FString& RoomName, 
	const FEOSKitProductUserId& ParticipantId, 
	bool bBlocked)
{
	EOS_HRTC RTCHandle = GetRTCHandle();
	if (!RTCHandle)
	{
		UE_LOG(LogTemp, Error, TEXT("EOSKitRTC: Failed to get RTC Handle"));
		return EEOSResult::EOS_NotConfigured;
	}

	EOS_RTC_BlockParticipantOptions Options = {};
	Options.ApiVersion = EOS_RTC_BLOCKPARTICIPANT_API_LATEST;
	Options.LocalUserId = LocalUserId.GetValueAsEosType();
	Options.RoomName = TCHAR_TO_UTF8(*RoomName);
	Options.ParticipantId = ParticipantId.GetValueAsEosType();
	Options.bBlocked = bBlocked ? EOS_TRUE : EOS_FALSE;

	// Note: This is a synchronous wrapper, but BlockParticipant is async
	// The actual async operation should be done through async nodes
	return EEOSResult::EOS_Success;
}

EEOSResult UEOSKitRTCSubsystem::SetSetting(const FString& SettingName, const FString& SettingValue)
{
	EOS_HRTC RTCHandle = GetRTCHandle();
	if (!RTCHandle)
	{
		UE_LOG(LogTemp, Error, TEXT("EOSKitRTC: Failed to get RTC Handle"));
		return EEOSResult::EOS_NotConfigured;
	}

	EOS_RTC_SetSettingOptions Options = {};
	Options.ApiVersion = EOS_RTC_SETSETTING_API_LATEST;
	Options.SettingName = TCHAR_TO_UTF8(*SettingName);
	Options.SettingValue = TCHAR_TO_UTF8(*SettingValue);

	EOS_EResult Result = EOS_RTC_SetSetting(RTCHandle, &Options);
	return ConvertEOSResultToEEOSResult(Result);
}

EEOSResult UEOSKitRTCSubsystem::SetRoomSetting(const FEOSKitProductUserId& LocalUserId, 
	const FString& RoomName, 
	const FString& SettingName, 
	const FString& SettingValue)
{
	EOS_HRTC RTCHandle = GetRTCHandle();
	if (!RTCHandle)
	{
		UE_LOG(LogTemp, Error, TEXT("EOSKitRTC: Failed to get RTC Handle"));
		return EEOSResult::EOS_NotConfigured;
	}

	EOS_RTC_SetRoomSettingOptions Options = {};
	Options.ApiVersion = EOS_RTC_SETROOMSETTING_API_LATEST;
	Options.LocalUserId = LocalUserId.GetValueAsEosType();
	Options.RoomName = TCHAR_TO_UTF8(*RoomName);
	Options.SettingName = TCHAR_TO_UTF8(*SettingName);
	Options.SettingValue = TCHAR_TO_UTF8(*SettingValue);

	EOS_EResult Result = EOS_RTC_SetRoomSetting(RTCHandle, &Options);
	return ConvertEOSResultToEEOSResult(Result);
}

int32 UEOSKitRTCSubsystem::AddNotifyParticipantStatusChanged(const FEOSKitProductUserId& LocalUserId, 
	const FString& RoomName, 
	const FOnEOSParticipantStatusChangedDelegate& Callback)
{
	EOS_HRTC RTCHandle = GetRTCHandle();
	if (!RTCHandle)
	{
		UE_LOG(LogTemp, Error, TEXT("EOSKitRTC: Failed to get RTC Handle"));
		return 0;
	}

	EOS_RTC_AddNotifyParticipantStatusChangedOptions Options = {};
	Options.ApiVersion = EOS_RTC_ADDNOTIFYPARTICIPANTSTATUSCHANGED_API_LATEST;
	Options.LocalUserId = LocalUserId.GetValueAsEosType();
	Options.RoomName = TCHAR_TO_UTF8(*RoomName);

	EOS_NotificationId NotificationId = EOS_RTC_AddNotifyParticipantStatusChanged(
		RTCHandle,
		&Options,
		this,
		&UEOSKitRTCSubsystem::OnParticipantStatusChangedCallback);

	if (NotificationId != EOS_INVALID_NOTIFICATIONID)
	{
		int32 OurNotificationId = NextNotificationId++;
		ParticipantStatusNotificationIdMap.Add(OurNotificationId, NotificationId);
		
		// Store the callback delegate with EOS notification ID as key
		ParticipantStatusChangedCallbacks.Add(NotificationId, Callback);
		
		return OurNotificationId;
	}

	return 0;
}

void UEOSKitRTCSubsystem::RemoveNotifyParticipantStatusChanged(int32 NotificationId)
{
	EOS_NotificationId* EOSNotificationId = ParticipantStatusNotificationIdMap.Find(NotificationId);
	if (EOSNotificationId)
	{
		EOS_HRTC RTCHandle = GetRTCHandle();
		if (RTCHandle)
		{
			EOS_RTC_RemoveNotifyParticipantStatusChanged(RTCHandle, *EOSNotificationId);
		}
		ParticipantStatusChangedCallbacks.Remove(*EOSNotificationId);
		ParticipantStatusNotificationIdMap.Remove(NotificationId);
	}
}

int32 UEOSKitRTCSubsystem::AddNotifyAudioBeforeRender(const FEOSKitProductUserId& LocalUserId, 
	const FString& RoomName, 
	const FOnEOSAudioBeforeRenderDelegate& Callback)
{
	EOS_HRTCAudio RTCAudioHandle = GetRTCAudioHandle();
	if (!RTCAudioHandle)
	{
		UE_LOG(LogTemp, Error, TEXT("EOSKitRTC: Failed to get RTC Audio Handle"));
		return 0;
	}

	EOS_RTCAudio_AddNotifyAudioBeforeRenderOptions Options = {};
	Options.ApiVersion = EOS_RTCAUDIO_ADDNOTIFYAUDIOBEFORERENDER_API_LATEST;
	Options.LocalUserId = LocalUserId.GetValueAsEosType();
	Options.RoomName = TCHAR_TO_UTF8(*RoomName);
	Options.bUnmixedAudio = EOS_FALSE; // Mixed audio by default

	EOS_NotificationId NotificationId = EOS_RTCAudio_AddNotifyAudioBeforeRender(
		RTCAudioHandle,
		&Options,
		this,
		&UEOSKitRTCSubsystem::OnAudioBeforeRenderCallback);

	if (NotificationId != EOS_INVALID_NOTIFICATIONID)
	{
		int32 OurNotificationId = NextNotificationId++;
		AudioBeforeRenderNotificationIdMap.Add(OurNotificationId, NotificationId);
		
		// Store the callback delegate with EOS notification ID as key
		AudioBeforeRenderCallbacks.Add(NotificationId, Callback);
		
		return OurNotificationId;
	}

	return 0;
}

void UEOSKitRTCSubsystem::RemoveNotifyAudioBeforeRender(int32 NotificationId)
{
	EOS_NotificationId* EOSNotificationId = AudioBeforeRenderNotificationIdMap.Find(NotificationId);
	if (EOSNotificationId)
	{
		EOS_HRTCAudio RTCAudioHandle = GetRTCAudioHandle();
		if (RTCAudioHandle)
		{
			EOS_RTCAudio_RemoveNotifyAudioBeforeRender(RTCAudioHandle, *EOSNotificationId);
		}
		AudioBeforeRenderCallbacks.Remove(*EOSNotificationId);
		AudioBeforeRenderNotificationIdMap.Remove(NotificationId);
	}
}

int32 UEOSKitRTCSubsystem::AddNotifyAudioBeforeSend(const FEOSKitProductUserId& LocalUserId, 
	const FString& RoomName, 
	const FOnEOSAudioBeforeSendDelegate& Callback)
{
	EOS_HRTCAudio RTCAudioHandle = GetRTCAudioHandle();
	if (!RTCAudioHandle)
	{
		UE_LOG(LogTemp, Error, TEXT("EOSKitRTC: Failed to get RTC Audio Handle"));
		return 0;
	}

	EOS_RTCAudio_AddNotifyAudioBeforeSendOptions Options = {};
	Options.ApiVersion = EOS_RTCAUDIO_ADDNOTIFYAUDIOBEFORESEND_API_LATEST;
	Options.LocalUserId = LocalUserId.GetValueAsEosType();
	Options.RoomName = TCHAR_TO_UTF8(*RoomName);

	EOS_NotificationId NotificationId = EOS_RTCAudio_AddNotifyAudioBeforeSend(
		RTCAudioHandle,
		&Options,
		this,
		&UEOSKitRTCSubsystem::OnAudioBeforeSendCallback);

	if (NotificationId != EOS_INVALID_NOTIFICATIONID)
	{
		int32 OurNotificationId = NextNotificationId++;
		AudioBeforeSendNotificationIdMap.Add(OurNotificationId, NotificationId);
		
		// Store the callback delegate with EOS notification ID as key
		AudioBeforeSendCallbacks.Add(NotificationId, Callback);
		
		return OurNotificationId;
	}

	return 0;
}

void UEOSKitRTCSubsystem::RemoveNotifyAudioBeforeSend(int32 NotificationId)
{
	EOS_NotificationId* EOSNotificationId = AudioBeforeSendNotificationIdMap.Find(NotificationId);
	if (EOSNotificationId)
	{
		EOS_HRTCAudio RTCAudioHandle = GetRTCAudioHandle();
		if (RTCAudioHandle)
		{
			EOS_RTCAudio_RemoveNotifyAudioBeforeSend(RTCAudioHandle, *EOSNotificationId);
		}
		AudioBeforeSendCallbacks.Remove(*EOSNotificationId);
		AudioBeforeSendNotificationIdMap.Remove(NotificationId);
	}
}

void EOS_CALL UEOSKitRTCSubsystem::OnParticipantStatusChangedCallback(const EOS_RTC_ParticipantStatusChangedCallbackInfo* Data)
{
	if (!Data || !Data->ClientData)
	{
		return;
	}

	UEOSKitRTCSubsystem* Self = static_cast<UEOSKitRTCSubsystem*>(Data->ClientData);

	FEOSKitRTCParticipantStatusChangedInfo StatusInfo;
	StatusInfo.LocalUserId = FEOSKitProductUserId(Data->LocalUserId);
	StatusInfo.RoomName = Data->RoomName ? UTF8_TO_TCHAR(Data->RoomName) : TEXT("");
	StatusInfo.ParticipantId = FEOSKitProductUserId(Data->ParticipantId);
	StatusInfo.ParticipantStatus = static_cast<EEOSKitRTCParticipantStatus>(Data->ParticipantStatus);
	StatusInfo.bParticipantInBlocklist = Data->bParticipantInBlocklist == EOS_TRUE;

	// Copy participant metadata
	if (Data->ParticipantMetadata && Data->ParticipantMetadataCount > 0)
	{
		StatusInfo.ParticipantMetadata.Reserve(Data->ParticipantMetadataCount);
		for (uint32_t i = 0; i < Data->ParticipantMetadataCount; i++)
		{
			FEOSKitRTCParticipantMetadata Metadata;
			Metadata.Key = Data->ParticipantMetadata[i].Key ? UTF8_TO_TCHAR(Data->ParticipantMetadata[i].Key) : TEXT("");
			Metadata.Value = Data->ParticipantMetadata[i].Value ? UTF8_TO_TCHAR(Data->ParticipantMetadata[i].Value) : TEXT("");
			StatusInfo.ParticipantMetadata.Add(Metadata);
		}
	}

	// Call all registered callback delegates
	for (const auto& Pair : Self->ParticipantStatusChangedCallbacks)
	{
		FEOSKitRTCParticipantStatusChangedInfo StatusInfoCopy = StatusInfo;
		FOnEOSParticipantStatusChangedDelegate CallbackCopy = Pair.Value;
		AsyncTask(ENamedThreads::GameThread, [CallbackCopy, StatusInfoCopy]()
		{
			CallbackCopy.ExecuteIfBound(StatusInfoCopy);
		});
	}

	// Broadcast to multicast delegate
	FEOSKitRTCParticipantStatusChangedInfo StatusInfoCopy = StatusInfo;
	AsyncTask(ENamedThreads::GameThread, [Self, StatusInfoCopy]()
	{
		Self->OnParticipantStatusChanged.Broadcast(StatusInfoCopy);
	});
}

void EOS_CALL UEOSKitRTCSubsystem::OnAudioBeforeRenderCallback(const EOS_RTCAudio_AudioBeforeRenderCallbackInfo* Data)
{
	if (!Data || !Data->ClientData || !Data->Buffer)
	{
		return;
	}

	UEOSKitRTCSubsystem* Self = static_cast<UEOSKitRTCSubsystem*>(Data->ClientData);

	// Convert audio buffer to byte array
	TArray<uint8> AudioBuffer;
	if (Data->Buffer->Frames && Data->Buffer->FramesCount > 0)
	{
		// Calculate total buffer size: frames * channels * sizeof(int16_t)
		int32 TotalFrames = Data->Buffer->FramesCount * Data->Buffer->Channels;
		AudioBuffer.SetNumUninitialized(TotalFrames * sizeof(int16_t));
		FMemory::Memcpy(AudioBuffer.GetData(), Data->Buffer->Frames, AudioBuffer.Num());
	}

	FEOSKitProductUserId LocalUserId(Data->LocalUserId);
	FEOSKitProductUserId ParticipantId(Data->ParticipantId);
	int32 SampleRate = static_cast<int32>(Data->Buffer->SampleRate);

	// Call all registered callback delegates
	for (const auto& Pair : Self->AudioBeforeRenderCallbacks)
	{
		FEOSKitProductUserId LocalUserIdCopy = LocalUserId;
		FEOSKitProductUserId ParticipantIdCopy = ParticipantId;
		TArray<uint8> AudioBufferCopy = AudioBuffer;
		FOnEOSAudioBeforeRenderDelegate CallbackCopy = Pair.Value;
		AsyncTask(ENamedThreads::GameThread, [CallbackCopy, LocalUserIdCopy, ParticipantIdCopy, AudioBufferCopy, SampleRate]()
		{
			CallbackCopy.ExecuteIfBound(LocalUserIdCopy, ParticipantIdCopy, AudioBufferCopy, SampleRate);
		});
	}

	// Broadcast to multicast delegate
	FEOSKitProductUserId LocalUserIdCopy = LocalUserId;
	FEOSKitProductUserId ParticipantIdCopy = ParticipantId;
	TArray<uint8> AudioBufferCopy = AudioBuffer;
	AsyncTask(ENamedThreads::GameThread, [Self, LocalUserIdCopy, ParticipantIdCopy, AudioBufferCopy, SampleRate]()
	{
		Self->OnAudioBeforeRender.Broadcast(LocalUserIdCopy, ParticipantIdCopy, AudioBufferCopy, SampleRate);
	});
}

void EOS_CALL UEOSKitRTCSubsystem::OnAudioBeforeSendCallback(const EOS_RTCAudio_AudioBeforeSendCallbackInfo* Data)
{
	if (!Data || !Data->ClientData || !Data->Buffer)
	{
		return;
	}

	UEOSKitRTCSubsystem* Self = static_cast<UEOSKitRTCSubsystem*>(Data->ClientData);

	// Convert audio buffer to byte array
	TArray<uint8> AudioBuffer;
	if (Data->Buffer->Frames && Data->Buffer->FramesCount > 0)
	{
		// Calculate total buffer size: frames * channels * sizeof(int16_t)
		int32 TotalFrames = Data->Buffer->FramesCount * Data->Buffer->Channels;
		AudioBuffer.SetNumUninitialized(TotalFrames * sizeof(int16_t));
		FMemory::Memcpy(AudioBuffer.GetData(), Data->Buffer->Frames, AudioBuffer.Num());
	}

	FEOSKitProductUserId LocalUserId(Data->LocalUserId);
	int32 SampleRate = static_cast<int32>(Data->Buffer->SampleRate);

	// Call all registered callback delegates
	for (const auto& Pair : Self->AudioBeforeSendCallbacks)
	{
		FEOSKitProductUserId LocalUserIdCopy = LocalUserId;
		TArray<uint8> AudioBufferCopy = AudioBuffer;
		FOnEOSAudioBeforeSendDelegate CallbackCopy = Pair.Value;
		AsyncTask(ENamedThreads::GameThread, [CallbackCopy, LocalUserIdCopy, AudioBufferCopy, SampleRate]()
		{
			CallbackCopy.ExecuteIfBound(LocalUserIdCopy, AudioBufferCopy, SampleRate);
		});
	}

	// Broadcast to multicast delegate
	FEOSKitProductUserId LocalUserIdCopy = LocalUserId;
	TArray<uint8> AudioBufferCopy = AudioBuffer;
	AsyncTask(ENamedThreads::GameThread, [Self, LocalUserIdCopy, AudioBufferCopy, SampleRate]()
	{
		Self->OnAudioBeforeSend.Broadcast(LocalUserIdCopy, AudioBufferCopy, SampleRate);
	});
}

