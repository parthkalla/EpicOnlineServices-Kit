// Copyright (C) 2024, All Rights Reserved.

#include "Subsystem/EOSKitVoiceSubsystem.h"

#if WITH_EOS_RTC
#include "EOSVoiceChat.h"
#endif

#include "EOSKitSubsystem.h"
#include "Kismet/GameplayStatics.h"
#if WITH_EOS_SDK
#include "Windows/AllowWindowsPlatformTypes.h"
#include "eos_sdk.h"
#include "eos_rtc.h"
#include "eos_rtc_types.h"
#include "Windows/HideWindowsPlatformTypes.h"
#endif
#include "Async/Async.h"

void UEOSKitVoiceSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

#if WITH_EOS_RTC
	// Create voice chat manager
	VoiceChatManager = MakeShared<FEOSVoiceChat, ESPMode::ThreadSafe>();
	VoiceChatManager->Initialize(this);
#endif

	bIsMuted = false;

	UE_LOG(LogTemp, Log, TEXT("EOSKitVoice: Subsystem initialized"));
}

void UEOSKitVoiceSubsystem::Deinitialize()
{
	// Disconnect from any active voice rooms
	if (!CurrentRoomName.IsEmpty() && !CurrentLocalUserId.IsEmpty())
	{
		DisconnectVoice(CurrentLocalUserId, CurrentRoomName);
	}

#if WITH_EOS_RTC
	if (VoiceChatManager)
	{
		VoiceChatManager->Shutdown();
		VoiceChatManager.Reset();
	}
#endif

	Super::Deinitialize();
	UE_LOG(LogTemp, Log, TEXT("EOSKitVoice: Subsystem deinitialized"));
}

void UEOSKitVoiceSubsystem::ConnectVoice(const FString& LocalUserId, const FString& RoomName)
{
	if (LocalUserId.IsEmpty() || RoomName.IsEmpty())
	{
		UE_LOG(LogTemp, Error, TEXT("EOSKitVoice: Invalid parameters for ConnectVoice"));
		OnVoiceConnectionComplete.Broadcast(EEOSResult::EOS_InvalidParameters, RoomName);
		return;
	}

	// Get the game instance from the first world (engine subsystems need to access game instance this way)
	UGameInstance* GameInstance = nullptr;
	if (GEngine)
	{
		for (const FWorldContext& Context : GEngine->GetWorldContexts())
		{
			if (Context.WorldType == EWorldType::Game || Context.WorldType == EWorldType::PIE)
			{
				GameInstance = Context.OwningGameInstance;
				break;
			}
		}
	}

	if (!GameInstance)
	{
		UE_LOG(LogTemp, Error, TEXT("EOSKitVoice: Failed to get game instance"));
		OnVoiceConnectionComplete.Broadcast(EEOSResult::EOS_InvalidState, RoomName);
		return;
	}

	UEOSKitSubsystem* EOSSubsystem = GameInstance->GetSubsystem<UEOSKitSubsystem>();
	if (!EOSSubsystem || !EOSSubsystem->GetPlatformHandle())
	{
		UE_LOG(LogTemp, Error, TEXT("EOSKitVoice: EOS subsystem not initialized"));
		OnVoiceConnectionComplete.Broadcast(EEOSResult::EOS_InvalidState, RoomName);
		return;
	}

	EOS_HPlatform PlatformHandle = EOSSubsystem->GetPlatformHandle();
	EOS_HRTC RTCHandle = EOS_Platform_GetRTCInterface(PlatformHandle);

	if (!RTCHandle)
	{
		UE_LOG(LogTemp, Error, TEXT("EOSKitVoice: Failed to get RTC interface"));
		OnVoiceConnectionComplete.Broadcast(EEOSResult::EOS_ServiceFailure, RoomName);
		return;
	}

	// Convert user ID
	EOS_ProductUserId LocalPUID = EOS_ProductUserId_FromString(TCHAR_TO_UTF8(*LocalUserId));
	if (!LocalPUID)
	{
		UE_LOG(LogTemp, Error, TEXT("EOSKitVoice: Invalid Product User ID"));
		OnVoiceConnectionComplete.Broadcast(EEOSResult::EOS_InvalidUser, RoomName);
		return;
	}

	// Setup join room options
	EOS_RTC_JoinRoomOptions JoinOptions = {};
	JoinOptions.ApiVersion = EOS_RTC_JOINROOM_API_LATEST;
	JoinOptions.LocalUserId = LocalPUID;

	FTCHARToUTF8 RoomNameConverter(*RoomName);
	JoinOptions.RoomName = RoomNameConverter.Get();

	// Enable manual audio input/output
	JoinOptions.bManualAudioInputEnabled = EOS_TRUE;
	JoinOptions.bManualAudioOutputEnabled = EOS_TRUE;

	CurrentLocalUserId = LocalUserId;
	CurrentRoomName = RoomName;

	UE_LOG(LogTemp, Log, TEXT("EOSKitVoice: Joining room %s"), *RoomName);

	// Join the room
	EOS_RTC_JoinRoom(RTCHandle, &JoinOptions, this, [](const EOS_RTC_JoinRoomCallbackInfo* Data)
	{
		if (!Data || !Data->ClientData)
		{
			return;
		}

		UEOSKitVoiceSubsystem* Self = static_cast<UEOSKitVoiceSubsystem*>(Data->ClientData);

		AsyncTask(ENamedThreads::GameThread, [Self, Data]()
		{
			FString RoomName = UTF8_TO_TCHAR(Data->RoomName);

			if (Data->ResultCode == EOS_EResult::EOS_Success)
			{
				UE_LOG(LogTemp, Log, TEXT("EOSKitVoice: Successfully joined room %s"), *RoomName);
				Self->OnVoiceConnectionComplete.Broadcast(EEOSResult::EOS_Success, RoomName);

#if WITH_EOS_RTC
				// Initialize voice chat manager
				if (Self->VoiceChatManager)
				{
					Self->VoiceChatManager->OnRoomJoined(RoomName);
				}
#endif
			}
			else
			{
				UE_LOG(LogTemp, Error, TEXT("EOSKitVoice: Failed to join room %s: %s"), 
					*RoomName, UTF8_TO_TCHAR(EOS_EResult_ToString(Data->ResultCode)));
				Self->OnVoiceConnectionComplete.Broadcast(EEOSResult::EOS_ServiceFailure, RoomName);
			}
		});
	});
}

void UEOSKitVoiceSubsystem::DisconnectVoice(const FString& LocalUserId, const FString& RoomName)
{
	if (LocalUserId.IsEmpty() || RoomName.IsEmpty())
	{
		UE_LOG(LogTemp, Error, TEXT("EOSKitVoice: Invalid parameters for DisconnectVoice"));
		return;
	}

	// Get the game instance from the first world (engine subsystems need to access game instance this way)
	UGameInstance* GameInstance = nullptr;
	if (GEngine)
	{
		for (const FWorldContext& Context : GEngine->GetWorldContexts())
		{
			if (Context.WorldType == EWorldType::Game || Context.WorldType == EWorldType::PIE)
			{
				GameInstance = Context.OwningGameInstance;
				break;
			}
		}
	}

	if (!GameInstance)
	{
		return;
	}

	UEOSKitSubsystem* EOSSubsystem = GameInstance->GetSubsystem<UEOSKitSubsystem>();
	if (!EOSSubsystem || !EOSSubsystem->GetPlatformHandle())
	{
		return;
	}

	EOS_HPlatform PlatformHandle = EOSSubsystem->GetPlatformHandle();
	EOS_HRTC RTCHandle = EOS_Platform_GetRTCInterface(PlatformHandle);

	if (!RTCHandle)
	{
		return;
	}

	// Convert user ID
	EOS_ProductUserId LocalPUID = EOS_ProductUserId_FromString(TCHAR_TO_UTF8(*LocalUserId));
	if (!LocalPUID)
	{
		return;
	}

	// Setup leave room options
	EOS_RTC_LeaveRoomOptions LeaveOptions = {};
	LeaveOptions.ApiVersion = EOS_RTC_LEAVEROOM_API_LATEST;
	LeaveOptions.LocalUserId = LocalPUID;

	FTCHARToUTF8 RoomNameConverter(*RoomName);
	LeaveOptions.RoomName = RoomNameConverter.Get();

	UE_LOG(LogTemp, Log, TEXT("EOSKitVoice: Leaving room %s"), *RoomName);

	// Leave the room
	EOS_RTC_LeaveRoom(RTCHandle, &LeaveOptions, this, [](const EOS_RTC_LeaveRoomCallbackInfo* Data)
	{
		if (!Data || !Data->ClientData)
		{
			return;
		}

		UEOSKitVoiceSubsystem* Self = static_cast<UEOSKitVoiceSubsystem*>(Data->ClientData);

		AsyncTask(ENamedThreads::GameThread, [Self, Data]()
		{
			FString RoomName = UTF8_TO_TCHAR(Data->RoomName);

			if (Data->ResultCode == EOS_EResult::EOS_Success)
			{
				UE_LOG(LogTemp, Log, TEXT("EOSKitVoice: Successfully left room %s"), *RoomName);

#if WITH_EOS_RTC
				// Cleanup
				if (Self->VoiceChatManager)
				{
					Self->VoiceChatManager->OnRoomLeft(RoomName);
				}
#endif

				Self->CurrentRoomName.Empty();
				Self->CurrentLocalUserId.Empty();
			}
			else
			{
				UE_LOG(LogTemp, Error, TEXT("EOSKitVoice: Failed to leave room %s: %s"), 
					*RoomName, UTF8_TO_TCHAR(EOS_EResult_ToString(Data->ResultCode)));
			}
		});
	});
}

void UEOSKitVoiceSubsystem::SetMicrophoneMuted(bool bMuted)
{
	bIsMuted = bMuted;

#if WITH_EOS_RTC
	if (VoiceChatManager)
	{
		VoiceChatManager->SetLocalMuted(bMuted);
	}
#endif

	UE_LOG(LogTemp, Log, TEXT("EOSKitVoice: Microphone %s"), bMuted ? TEXT("muted") : TEXT("unmuted"));
}
