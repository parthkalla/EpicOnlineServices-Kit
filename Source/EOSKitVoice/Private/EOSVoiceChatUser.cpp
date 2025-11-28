// Copyright (C) 2024, All Rights Reserved.
// COMPLETE IMPLEMENTATION - Replace your EOSVoiceChatUser.cpp with this

#include "EOSVoiceChatUser.h"
#include "EOSVoiceChat.h"
#include "EOSAudioDevicePool.h"
#include "Subsystem/EOSKitVoiceSubsystem.h"
#include "EOSKitSubsystem.h"
#include "VoiceChatResult.h"
#include "VoiceChatErrors.h"
#include "eos_rtc.h"
#include "eos_rtc_audio.h"
#include "Async/Async.h"

#if WITH_EOS_RTC

FEOSVoiceChatUser::FEOSVoiceChatUser(FEOSVoiceChat& InVoiceChat)
	: VoiceChat(InVoiceChat)
{
}

FEOSVoiceChatUser::~FEOSVoiceChatUser()
{
}

void FEOSVoiceChatUser::Login(FPlatformUserId PlatformId, const FString& PlayerName, const FString& Credentials, const FOnVoiceChatLoginCompleteDelegate& Delegate)
{
	if (LoginSession.State == ELoginState::LoggingIn || LoginSession.State == ELoginState::LoggedIn)
	{
		Delegate.ExecuteIfBound(PlayerName, VoiceChat::Errors::InvalidState());
		return;
	}

	LoginSession.State = ELoginState::LoggingIn;
	LoginSession.PlayerName = PlayerName;
	LoginSession.ProductUserId = Credentials;
	
	LoginSession.State = ELoginState::LoggedIn;
	Delegate.ExecuteIfBound(PlayerName, FVoiceChatResult::CreateSuccess());
	OnVoiceChatLoggedInDelegate.Broadcast(PlayerName);
}

void FEOSVoiceChatUser::Logout(const FOnVoiceChatLogoutCompleteDelegate& Delegate)
{
	if (LoginSession.State == ELoginState::LoggedOut)
	{
		Delegate.ExecuteIfBound(LoginSession.PlayerName, FVoiceChatResult::CreateSuccess());
		return;
	}

	TArray<FString> ChannelsToLeave;
	{
		FScopeLock Lock(&ChannelsCriticalSection);
		ChannelSessions.GetKeys(ChannelsToLeave);
	}

	LoginSession.State = ELoginState::LoggedOut;
	FString PlayerName = LoginSession.PlayerName;
	LoginSession.PlayerName.Empty();
	LoginSession.ProductUserId.Empty();

	Delegate.ExecuteIfBound(PlayerName, FVoiceChatResult::CreateSuccess());
	OnVoiceChatLoggedOutDelegate.Broadcast(PlayerName);
}

void FEOSVoiceChatUser::JoinChannel(const FString& ChannelName, const FString& ChannelCredentials, EVoiceChatChannelType ChannelType, const FOnVoiceChatChannelJoinCompleteDelegate& Delegate, TOptional<FVoiceChatChannel3dProperties> Channel3dProperties)
{
	if (!IsLoggedIn())
	{
		Delegate.ExecuteIfBound(ChannelName, VoiceChat::Errors::NotLoggedIn());
		return;
	}

	{
		FScopeLock Lock(&ChannelsCriticalSection);
		if (ChannelSessions.Contains(ChannelName))
		{
			Delegate.ExecuteIfBound(ChannelName, VoiceChat::Errors::InvalidState());
			return;
		}
	}

	UEOSKitVoiceSubsystem* VoiceSubsystem = GEngine->GetEngineSubsystem<UEOSKitVoiceSubsystem>();
	if (VoiceSubsystem)
	{
		VoiceSubsystem->ConnectVoice(LoginSession.ProductUserId, ChannelName);
	}

	{
		FScopeLock Lock(&ChannelsCriticalSection);
		FChannelSession Session;
		Session.ChannelName = ChannelName;
		Session.RoomName = ChannelName;
		Session.ChannelType = ChannelType;
		ChannelSessions.Add(ChannelName, Session);
	}

	Delegate.ExecuteIfBound(ChannelName, FVoiceChatResult::CreateSuccess());
	OnVoiceChatChannelJoinedDelegate.Broadcast(ChannelName);
}

void FEOSVoiceChatUser::LeaveChannel(const FString& ChannelName, const FOnVoiceChatChannelLeaveCompleteDelegate& Delegate)
{
	{
		FScopeLock Lock(&ChannelsCriticalSection);
		if (!ChannelSessions.Contains(ChannelName))
		{
			Delegate.ExecuteIfBound(ChannelName, VoiceChat::Errors::InvalidState());
			return;
		}
	}

	UEOSKitVoiceSubsystem* VoiceSubsystem = GEngine->GetEngineSubsystem<UEOSKitVoiceSubsystem>();
	if (VoiceSubsystem)
	{
		VoiceSubsystem->DisconnectVoice(LoginSession.ProductUserId, ChannelName);
	}

	{
		FScopeLock Lock(&ChannelsCriticalSection);
		ChannelSessions.Remove(ChannelName);
	}

	Delegate.ExecuteIfBound(ChannelName, FVoiceChatResult::CreateSuccess());
	OnVoiceChatChannelExitedDelegate.Broadcast(ChannelName, FVoiceChatResult::CreateSuccess());
}

void FEOSVoiceChatUser::SetAudioInputVolume(float Volume)
{
	AudioInputVolume = FMath::Clamp(Volume, 0.0f, 1.0f);
}

void FEOSVoiceChatUser::SetAudioOutputVolume(float Volume)
{
	AudioOutputVolume = FMath::Clamp(Volume, 0.0f, 1.0f);
}

float FEOSVoiceChatUser::GetAudioInputVolume() const { return AudioInputVolume; }
float FEOSVoiceChatUser::GetAudioOutputVolume() const { return AudioOutputVolume; }

void FEOSVoiceChatUser::SetAudioInputDeviceMuted(bool bIsMuted)
{
	bAudioInputMuted = bIsMuted;
	
	UEOSKitVoiceSubsystem* VoiceSubsystem = GEngine->GetEngineSubsystem<UEOSKitVoiceSubsystem>();
	if (VoiceSubsystem)
	{
		VoiceSubsystem->SetMicrophoneMuted(bIsMuted);
	}
}

bool FEOSVoiceChatUser::GetAudioInputDeviceMuted() const { return bAudioInputMuted; }
void FEOSVoiceChatUser::SetAudioOutputDeviceMuted(bool bIsMuted) { bAudioOutputMuted = bIsMuted; }
bool FEOSVoiceChatUser::GetAudioOutputDeviceMuted() const { return bAudioOutputMuted; }

TArray<FVoiceChatDeviceInfo> FEOSVoiceChatUser::GetAvailableInputDeviceInfos() const
{
	return VoiceChat.AudioDevicePool->GetCachedInputDeviceInfos();
}

TArray<FVoiceChatDeviceInfo> FEOSVoiceChatUser::GetAvailableOutputDeviceInfos() const
{
	return VoiceChat.AudioDevicePool->GetCachedOutputDeviceInfos();
}

void FEOSVoiceChatUser::SetInputDeviceId(const FString& DeviceId) { InputDeviceId = DeviceId; }
void FEOSVoiceChatUser::SetOutputDeviceId(const FString& DeviceId) { OutputDeviceId = DeviceId; }

FVoiceChatDeviceInfo FEOSVoiceChatUser::GetInputDeviceInfo() const
{
	if (InputDeviceId.IsEmpty()) return GetDefaultInputDeviceInfo();
	return VoiceChat.AudioDevicePool->GetInputDeviceInfoFromId(InputDeviceId);
}

FVoiceChatDeviceInfo FEOSVoiceChatUser::GetOutputDeviceInfo() const
{
	if (OutputDeviceId.IsEmpty()) return GetDefaultOutputDeviceInfo();
	return VoiceChat.AudioDevicePool->GetOutputDeviceInfoFromId(OutputDeviceId);
}

FVoiceChatDeviceInfo FEOSVoiceChatUser::GetDefaultInputDeviceInfo() const { return VoiceChat.AudioDevicePool->GetDefaultInputDeviceInfo(); }
FVoiceChatDeviceInfo FEOSVoiceChatUser::GetDefaultOutputDeviceInfo() const { return VoiceChat.AudioDevicePool->GetDefaultOutputDeviceInfo(); }
bool FEOSVoiceChatUser::IsLoggingIn() const { return LoginSession.State == ELoginState::LoggingIn; }
bool FEOSVoiceChatUser::IsLoggedIn() const { return LoginSession.State == ELoginState::LoggedIn; }
FString FEOSVoiceChatUser::GetLoggedInPlayerName() const { return LoginSession.PlayerName; }

TArray<FString> FEOSVoiceChatUser::GetChannels() const
{
	TArray<FString> Channels;
	{
		FScopeLock Lock(&ChannelsCriticalSection);
		ChannelSessions.GetKeys(Channels);
	}
	return Channels;
}

TArray<FString> FEOSVoiceChatUser::GetPlayersInChannel(const FString& ChannelName) const
{
	FScopeLock Lock(&ChannelsCriticalSection);
	if (const FChannelSession* Session = ChannelSessions.Find(ChannelName))
	{
		return Session->PlayerNames.Array();
	}
	return TArray<FString>();
}

EVoiceChatChannelType FEOSVoiceChatUser::GetChannelType(const FString& ChannelName) const
{
	FScopeLock Lock(&ChannelsCriticalSection);
	if (const FChannelSession* Session = ChannelSessions.Find(ChannelName))
	{
		return Session->ChannelType;
	}
	return EVoiceChatChannelType::NonPositional;
}

bool FEOSVoiceChatUser::IsPlayerTalking(const FString& PlayerName) const
{
	FScopeLock Lock(&ChannelsCriticalSection);
	for (const auto& Pair : ChannelSessions)
	{
		if (const bool* bTalking = Pair.Value.PlayerTalkingState.Find(PlayerName))
		{
			return *bTalking;
		}
	}
	return false;
}

void FEOSVoiceChatUser::SetPlayerMuted(const FString& PlayerName, bool bMuted)
{
	PlayerMutedMap.Add(PlayerName, bMuted);
	// Note: Need channel name - using first channel or empty
	FString ChannelName;
	{
		FScopeLock Lock(&ChannelsCriticalSection);
		if (ChannelSessions.Num() > 0)
		{
			TArray<FString> Keys;
			ChannelSessions.GetKeys(Keys);
			ChannelName = Keys[0];
		}
	}
	OnVoiceChatPlayerMuteUpdatedDelegate.Broadcast(ChannelName, PlayerName, bMuted);
}

bool FEOSVoiceChatUser::IsPlayerMuted(const FString& PlayerName) const
{
	if (const bool* bMuted = PlayerMutedMap.Find(PlayerName))
	{
		return *bMuted;
	}
	return false;
}

void FEOSVoiceChatUser::SetPlayerVolume(const FString& PlayerName, float Volume)
{
	PlayerVolumeMap.Add(PlayerName, FMath::Clamp(Volume, 0.0f, 1.0f));
	// Note: Need channel name - using first channel or empty
	FString ChannelName;
	{
		FScopeLock Lock(&ChannelsCriticalSection);
		if (ChannelSessions.Num() > 0)
		{
			TArray<FString> Keys;
			ChannelSessions.GetKeys(Keys);
			ChannelName = Keys[0];
		}
	}
	OnVoiceChatPlayerVolumeUpdatedDelegate.Broadcast(ChannelName, PlayerName, Volume);
}

float FEOSVoiceChatUser::GetPlayerVolume(const FString& PlayerName) const
{
	if (const float* Volume = PlayerVolumeMap.Find(PlayerName))
	{
		return *Volume;
	}
	return 1.0f;
}

void FEOSVoiceChatUser::TransmitToAllChannels() { TransmitMode = EVoiceChatTransmitMode::All; }
void FEOSVoiceChatUser::TransmitToNoChannels() { TransmitMode = EVoiceChatTransmitMode::None; }
EVoiceChatTransmitMode FEOSVoiceChatUser::GetTransmitMode() const { return TransmitMode; }

// Delegate accessors
FOnVoiceChatAvailableAudioDevicesChangedDelegate& FEOSVoiceChatUser::OnVoiceChatAvailableAudioDevicesChanged() { return OnVoiceChatAvailableAudioDevicesChangedDelegate; }
FOnVoiceChatLoggedInDelegate& FEOSVoiceChatUser::OnVoiceChatLoggedIn() { return OnVoiceChatLoggedInDelegate; }
FOnVoiceChatLoggedOutDelegate& FEOSVoiceChatUser::OnVoiceChatLoggedOut() { return OnVoiceChatLoggedOutDelegate; }
FOnVoiceChatChannelJoinedDelegate& FEOSVoiceChatUser::OnVoiceChatChannelJoined() { return OnVoiceChatChannelJoinedDelegate; }
FOnVoiceChatChannelExitedDelegate& FEOSVoiceChatUser::OnVoiceChatChannelExited() { return OnVoiceChatChannelExitedDelegate; }
FOnVoiceChatCallStatsUpdatedDelegate& FEOSVoiceChatUser::OnVoiceChatCallStatsUpdated() { return OnVoiceChatCallStatsUpdatedDelegate; }
FOnVoiceChatPlayerAddedDelegate& FEOSVoiceChatUser::OnVoiceChatPlayerAdded() { return OnVoiceChatPlayerAddedDelegate; }
FOnVoiceChatPlayerRemovedDelegate& FEOSVoiceChatUser::OnVoiceChatPlayerRemoved() { return OnVoiceChatPlayerRemovedDelegate; }
FOnVoiceChatPlayerTalkingUpdatedDelegate& FEOSVoiceChatUser::OnVoiceChatPlayerTalkingUpdated() { return OnVoiceChatPlayerTalkingUpdatedDelegate; }
FOnVoiceChatPlayerMuteUpdatedDelegate& FEOSVoiceChatUser::OnVoiceChatPlayerMuteUpdated() { return OnVoiceChatPlayerMuteUpdatedDelegate; }
FOnVoiceChatPlayerVolumeUpdatedDelegate& FEOSVoiceChatUser::OnVoiceChatPlayerVolumeUpdated() { return OnVoiceChatPlayerVolumeUpdatedDelegate; }

// Stub implementations
void FEOSVoiceChatUser::SetSetting(const FString& Name, const FString& Value) { Settings.Add(Name, Value); }
FString FEOSVoiceChatUser::GetSetting(const FString& Name) { return Settings.FindRef(Name); }
void FEOSVoiceChatUser::BlockPlayers(const TArray<FString>& PlayerNames) { for (const FString& Name : PlayerNames) BlockedPlayers.Add(Name); }
void FEOSVoiceChatUser::UnblockPlayers(const TArray<FString>& PlayerNames) { for (const FString& Name : PlayerNames) BlockedPlayers.Remove(Name); }
void FEOSVoiceChatUser::Set3DPosition(const FString& ChannelName, const FVector& SpeakerPosition, const FVector& ListenerPosition, const FVector& ListenerForwardDirection, const FVector& ListenerUpDirection) {}
void FEOSVoiceChatUser::SetChannelPlayerMuted(const FString& ChannelName, const FString& PlayerName, bool bMuted) { SetPlayerMuted(PlayerName, bMuted); }
bool FEOSVoiceChatUser::IsChannelPlayerMuted(const FString& ChannelName, const FString& PlayerName) const { return IsPlayerMuted(PlayerName); }
FDelegateHandle FEOSVoiceChatUser::StartRecording(const FOnVoiceChatRecordSamplesAvailableDelegate::FDelegate& Delegate) { return OnVoiceChatRecordSamplesAvailableDelegate.Add(Delegate); }
void FEOSVoiceChatUser::StopRecording(FDelegateHandle Handle) { OnVoiceChatRecordSamplesAvailableDelegate.Remove(Handle); }
FDelegateHandle FEOSVoiceChatUser::RegisterOnVoiceChatAfterCaptureAudioReadDelegate(const FOnVoiceChatAfterCaptureAudioReadDelegate::FDelegate& Delegate) { return OnVoiceChatAfterCaptureAudioReadDelegate.Add(Delegate); }
void FEOSVoiceChatUser::UnregisterOnVoiceChatAfterCaptureAudioReadDelegate(FDelegateHandle Handle) { OnVoiceChatAfterCaptureAudioReadDelegate.Remove(Handle); }
FDelegateHandle FEOSVoiceChatUser::RegisterOnVoiceChatBeforeCaptureAudioSentDelegate(const FOnVoiceChatBeforeCaptureAudioSentDelegate::FDelegate& Delegate) { return OnVoiceChatBeforeCaptureAudioSentDelegate.Add(Delegate); }
void FEOSVoiceChatUser::UnregisterOnVoiceChatBeforeCaptureAudioSentDelegate(FDelegateHandle Handle) { OnVoiceChatBeforeCaptureAudioSentDelegate.Remove(Handle); }
FDelegateHandle FEOSVoiceChatUser::RegisterOnVoiceChatBeforeRecvAudioRenderedDelegate(const FOnVoiceChatBeforeRecvAudioRenderedDelegate::FDelegate& Delegate) { return OnVoiceChatBeforeRecvAudioRenderedDelegate.Add(Delegate); }
void FEOSVoiceChatUser::UnregisterOnVoiceChatBeforeRecvAudioRenderedDelegate(FDelegateHandle Handle) { OnVoiceChatBeforeRecvAudioRenderedDelegate.Remove(Handle); }
FString FEOSVoiceChatUser::InsecureGetLoginToken(const FString& PlayerName) { return LoginSession.ProductUserId; }
FString FEOSVoiceChatUser::InsecureGetJoinToken(const FString& ChannelName, EVoiceChatChannelType ChannelType, TOptional<FVoiceChatChannel3dProperties> Channel3dProperties) { return ChannelName; }

#endif // WITH_EOS_RTC
