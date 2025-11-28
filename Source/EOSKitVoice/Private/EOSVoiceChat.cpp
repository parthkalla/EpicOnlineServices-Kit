// Copyright (C) 2024, All Rights Reserved.

#include "EOSVoiceChat.h"

#if WITH_EOS_RTC

#include "Subsystem/EOSKitVoiceSubsystem.h"
#include "EOSVoiceChatSynthComponent.h"
#include "EOSVoiceChatUser.h"
#include "EOSAudioDevicePool.h"
#include "EOSKitSubsystem.h"
#include "Kismet/GameplayStatics.h"
#include "eos_rtc.h"
#include "eos_rtc_audio.h"
#include "eos_rtc_types.h"
#include "Async/Async.h"

// Forward declaration of callback function
void EOS_CALL EOSVoiceChat_OnParticipantStatusChanged(const EOS_RTC_ParticipantStatusChangedCallbackInfo* Data);

FEOSVoiceChat::FEOSVoiceChat()
{
	// Audio device pool will be initialized during Initialize() when we have proper context
}

FEOSVoiceChat::~FEOSVoiceChat()
{
}

void FEOSVoiceChat::Initialize(UEOSKitVoiceSubsystem* InSubsystem)
{
	OwningSubsystem = InSubsystem;
	bIsInitialized = false;

	// Initialize audio device pool now that we have subsystem context
	if (OwningSubsystem)
	{
		// Get the game instance from the first world
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

		if (GameInstance)
		{
			UEOSKitSubsystem* EOSSubsystem = GameInstance->GetSubsystem<UEOSKitSubsystem>();
			if (EOSSubsystem)
			{
				EOS_HPlatform PlatformHandle = EOSSubsystem->GetPlatformHandle();
				if (PlatformHandle)
				{
					EOS_HRTC RTCHandle = EOS_Platform_GetRTCInterface(PlatformHandle);
					if (RTCHandle)
					{
						EOS_HRTCAudio RTCAudioHandle = EOS_RTC_GetAudioInterface(RTCHandle);
						if (RTCAudioHandle)
						{
							AudioDevicePool = MakeShared<FEOSAudioDevicePool>(RTCAudioHandle);
						}
					}
				}
			}
		}
	}
}

void FEOSVoiceChat::Shutdown()
{
	UnregisterNotifications();

	for (auto& Pair : UserSynthComponents)
	{
		if (Pair.Value)
		{
			Pair.Value->Stop();
			Pair.Value->DestroyComponent();
		}
	}
	UserSynthComponents.Empty();

	OwningSubsystem = nullptr;
	bIsInitialized = false;
}

void FEOSVoiceChat::OnRoomJoined(const FString& RoomName)
{
	CurrentRoomName = RoomName;
	RegisterNotifications();
	bIsInitialized = true;
}

void FEOSVoiceChat::OnRoomLeft(const FString& RoomName)
{
	UnregisterNotifications();

	for (auto& Pair : UserSynthComponents)
	{
		if (Pair.Value)
		{
			Pair.Value->Stop();
			Pair.Value->DestroyComponent();
		}
	}
	UserSynthComponents.Empty();

	CurrentRoomName.Empty();
	bIsInitialized = false;
}

void FEOSVoiceChat::SetLocalMuted(bool bMuted)
{
	bIsLocalMuted = bMuted;
}

void FEOSVoiceChat::RegisterNotifications()
{
	// Notifications are registered in UEOSKitVoiceSubsystem
}

void FEOSVoiceChat::UnregisterNotifications()
{
	// Cleanup handled in subsystem
}

void FEOSVoiceChat::HandleParticipantJoined(const FString& RoomName, const FString& UserId)
{
	if (!OwningSubsystem)
	{
		return;
	}

	UE_LOG(LogTemp, Log, TEXT("FEOSVoiceChat: Participant joined - Room: %s, User: %s"), *RoomName, *UserId);

	GetOrCreateSynthComponent(UserId);

	if (OwningSubsystem)
	{
		OwningSubsystem->OnVoiceUserJoined.Broadcast(UserId, RoomName);
	}
}

void FEOSVoiceChat::HandleParticipantLeft(const FString& RoomName, const FString& UserId)
{
	if (!OwningSubsystem)
	{
		return;
	}

	UE_LOG(LogTemp, Log, TEXT("FEOSVoiceChat: Participant left - Room: %s, User: %s"), *RoomName, *UserId);

	RemoveSynthComponent(UserId);

	if (OwningSubsystem)
	{
		OwningSubsystem->OnVoiceUserLeft.Broadcast(UserId, RoomName);
	}
}

void FEOSVoiceChat::HandleAudioData(const FString& RoomName, const FString& UserId, const float* AudioBuffer, int32 NumSamples, int32 SampleRate, int32 NumChannels)
{
	UEOSVoiceChatSynthComponent* SynthComp = GetOrCreateSynthComponent(UserId);
	if (SynthComp && AudioBuffer)
	{
		SynthComp->AddAudioBuffer(AudioBuffer, NumSamples, SampleRate, NumChannels);

		if (OwningSubsystem)
		{
			OwningSubsystem->OnVoiceUserTalking.Broadcast(UserId, true);
		}
	}
}

UEOSVoiceChatSynthComponent* FEOSVoiceChat::GetOrCreateSynthComponent(const FString& UserId)
{
	if (UEOSVoiceChatSynthComponent** Found = UserSynthComponents.Find(UserId))
	{
		return *Found;
	}

	if (!OwningSubsystem)
	{
		return nullptr;
	}

	UEOSVoiceChatSynthComponent* NewSynth = NewObject<UEOSVoiceChatSynthComponent>(OwningSubsystem);
	if (NewSynth)
	{
		NewSynth->RegisterComponent();
		NewSynth->Start();

		UserSynthComponents.Add(UserId, NewSynth);

		UE_LOG(LogTemp, Log, TEXT("FEOSVoiceChat: Created synth component for user %s"), *UserId);
	}

	return NewSynth;
}

void FEOSVoiceChat::RemoveSynthComponent(const FString& UserId)
{
	if (UEOSVoiceChatSynthComponent** Found = UserSynthComponents.Find(UserId))
	{
		UEOSVoiceChatSynthComponent* SynthComp = *Found;
		if (SynthComp)
		{
			SynthComp->Stop();
			SynthComp->DestroyComponent();
		}

		UserSynthComponents.Remove(UserId);

		UE_LOG(LogTemp, Log, TEXT("FEOSVoiceChat: Removed synth component for user %s"), *UserId);
	}
}

void EOS_CALL EOSVoiceChat_OnParticipantStatusChanged(const EOS_RTC_ParticipantStatusChangedCallbackInfo* Data)
{
	if (!Data || !Data->ClientData)
	{
		return;
	}

	FEOSVoiceChat* VoiceChat = static_cast<FEOSVoiceChat*>(Data->ClientData);

	char UserIdStr[EOS_PRODUCTUSERID_MAX_LENGTH + 1];
	int32_t UserIdStrSize = sizeof(UserIdStr);
	
	if (EOS_ProductUserId_ToString(Data->ParticipantId, UserIdStr, &UserIdStrSize) != EOS_EResult::EOS_Success)
	{
		return;
	}

	FString UserId = UTF8_TO_TCHAR(UserIdStr);
	FString RoomName = UTF8_TO_TCHAR(Data->RoomName);

	AsyncTask(ENamedThreads::GameThread, [VoiceChat, RoomName, UserId, Data]()
	{
		if (Data->ParticipantStatus == EOS_ERTCParticipantStatus::EOS_RTCPS_Joined)
		{
			VoiceChat->HandleParticipantJoined(RoomName, UserId);
		}
		else if (Data->ParticipantStatus == EOS_ERTCParticipantStatus::EOS_RTCPS_Left)
		{
			VoiceChat->HandleParticipantLeft(RoomName, UserId);
		}
	});
}

#endif // WITH_EOS_RTC
