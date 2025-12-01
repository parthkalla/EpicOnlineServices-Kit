// Copyright (C) 2024, All Rights Reserved.
#pragma once
#include "CoreMinimal.h"
#if WITH_EOS_RTC
#include "VoiceChat.h"
#include "Misc/Optional.h"
#include "eos_rtc_types.h"

class FEOSVoiceChat;

class EOSKITRTC_API FEOSVoiceChatUser : public IVoiceChatUser
{
public:
	FEOSVoiceChatUser(FEOSVoiceChat& InVoiceChat);
	virtual ~FEOSVoiceChatUser();

	virtual void SetSetting(const FString& Name, const FString& Value) override;
	virtual FString GetSetting(const FString& Name) override;
	virtual void SetAudioInputVolume(float Volume) override;
	virtual void SetAudioOutputVolume(float Volume) override;
	virtual float GetAudioInputVolume() const override;
	virtual float GetAudioOutputVolume() const override;
	virtual void SetAudioInputDeviceMuted(bool bIsMuted) override;
	virtual void SetAudioOutputDeviceMuted(bool bIsMuted) override;
	virtual bool GetAudioInputDeviceMuted() const override;
	virtual bool GetAudioOutputDeviceMuted() const override;
	virtual TArray<FVoiceChatDeviceInfo> GetAvailableInputDeviceInfos() const override;
	virtual TArray<FVoiceChatDeviceInfo> GetAvailableOutputDeviceInfos() const override;
	virtual FOnVoiceChatAvailableAudioDevicesChangedDelegate& OnVoiceChatAvailableAudioDevicesChanged() override;
	virtual void SetInputDeviceId(const FString& InputDeviceId) override;
	virtual void SetOutputDeviceId(const FString& OutputDeviceId) override;
	virtual FVoiceChatDeviceInfo GetInputDeviceInfo() const override;
	virtual FVoiceChatDeviceInfo GetOutputDeviceInfo() const override;
	virtual FVoiceChatDeviceInfo GetDefaultInputDeviceInfo() const override;
	virtual FVoiceChatDeviceInfo GetDefaultOutputDeviceInfo() const override;
	virtual void Login(FPlatformUserId PlatformId, const FString& PlayerName, const FString& Credentials, const FOnVoiceChatLoginCompleteDelegate& Delegate) override;
	virtual void Logout(const FOnVoiceChatLogoutCompleteDelegate& Delegate) override;
	virtual bool IsLoggingIn() const override;
	virtual bool IsLoggedIn() const override;
	virtual FOnVoiceChatLoggedInDelegate& OnVoiceChatLoggedIn() override;
	virtual FOnVoiceChatLoggedOutDelegate& OnVoiceChatLoggedOut() override;
	virtual FString GetLoggedInPlayerName() const override;
	virtual void BlockPlayers(const TArray<FString>& PlayerNames) override;
	virtual void UnblockPlayers(const TArray<FString>& PlayerNames) override;
	virtual void JoinChannel(const FString& ChannelName, const FString& ChannelCredentials, EVoiceChatChannelType ChannelType, const FOnVoiceChatChannelJoinCompleteDelegate& Delegate, TOptional<FVoiceChatChannel3dProperties> Channel3dProperties = TOptional<FVoiceChatChannel3dProperties>()) override;
	virtual void LeaveChannel(const FString& Channel, const FOnVoiceChatChannelLeaveCompleteDelegate& Delegate) override;
	virtual FOnVoiceChatChannelJoinedDelegate& OnVoiceChatChannelJoined() override;
	virtual FOnVoiceChatChannelExitedDelegate& OnVoiceChatChannelExited() override;
	virtual FOnVoiceChatCallStatsUpdatedDelegate& OnVoiceChatCallStatsUpdated() override;
	virtual void Set3DPosition(const FString& ChannelName, const FVector& SpeakerPosition, const FVector& ListenerPosition, const FVector& ListenerForwardDirection, const FVector& ListenerUpDirection) override;
	virtual TArray<FString> GetChannels() const override;
	virtual TArray<FString> GetPlayersInChannel(const FString& ChannelName) const override;
	virtual EVoiceChatChannelType GetChannelType(const FString& ChannelName) const override;
	virtual FOnVoiceChatPlayerAddedDelegate& OnVoiceChatPlayerAdded() override;
	virtual FOnVoiceChatPlayerRemovedDelegate& OnVoiceChatPlayerRemoved() override;
	virtual bool IsPlayerTalking(const FString& PlayerName) const override;
	virtual FOnVoiceChatPlayerTalkingUpdatedDelegate& OnVoiceChatPlayerTalkingUpdated() override;
	virtual void SetPlayerMuted(const FString& PlayerName, bool bMuted) override;
	virtual bool IsPlayerMuted(const FString& PlayerName) const override;
	virtual void SetChannelPlayerMuted(const FString& ChannelName, const FString& PlayerName, bool bMuted) override;
	virtual bool IsChannelPlayerMuted(const FString& ChannelName, const FString& PlayerName) const override;
	virtual FOnVoiceChatPlayerMuteUpdatedDelegate& OnVoiceChatPlayerMuteUpdated() override;
	virtual void SetPlayerVolume(const FString& PlayerName, float Volume) override;
	virtual float GetPlayerVolume(const FString& PlayerName) const override;
	virtual FOnVoiceChatPlayerVolumeUpdatedDelegate& OnVoiceChatPlayerVolumeUpdated() override;
	virtual void TransmitToAllChannels() override;
	virtual void TransmitToNoChannels() override;
	
	virtual EVoiceChatTransmitMode GetTransmitMode() const override;
	
	virtual FDelegateHandle StartRecording(const FOnVoiceChatRecordSamplesAvailableDelegate::FDelegate& Delegate) override;
	virtual void StopRecording(FDelegateHandle Handle) override;
	virtual FDelegateHandle RegisterOnVoiceChatAfterCaptureAudioReadDelegate(const FOnVoiceChatAfterCaptureAudioReadDelegate::FDelegate& Delegate) override;
	virtual void UnregisterOnVoiceChatAfterCaptureAudioReadDelegate(FDelegateHandle Handle) override;
	virtual FDelegateHandle RegisterOnVoiceChatBeforeCaptureAudioSentDelegate(const FOnVoiceChatBeforeCaptureAudioSentDelegate::FDelegate& Delegate) override;
	virtual void UnregisterOnVoiceChatBeforeCaptureAudioSentDelegate(FDelegateHandle Handle) override;
	virtual FDelegateHandle RegisterOnVoiceChatBeforeRecvAudioRenderedDelegate(const FOnVoiceChatBeforeRecvAudioRenderedDelegate::FDelegate& Delegate) override;
	virtual void UnregisterOnVoiceChatBeforeRecvAudioRenderedDelegate(FDelegateHandle Handle) override;
	virtual FString InsecureGetLoginToken(const FString& PlayerName) override;
	virtual FString InsecureGetJoinToken(const FString& ChannelName, EVoiceChatChannelType ChannelType, TOptional<FVoiceChatChannel3dProperties> Channel3dProperties = TOptional<FVoiceChatChannel3dProperties>()) override;

	enum class ELoginState : uint8 { LoggedOut, LoggingIn, LoggedIn, LoggingOut };
	struct FLoginSession {
		ELoginState State = ELoginState::LoggedOut;
		FString PlayerName;
		FString ProductUserId;
	};
	FLoginSession LoginSession;

	struct FChannelSession {
		FString ChannelName;
		FString RoomName;
		EVoiceChatChannelType ChannelType;
		TSet<FString> PlayerNames;
		TMap<FString, bool> PlayerTalkingState;
	};

protected:
	FEOSVoiceChat& VoiceChat;
	float AudioInputVolume = 1.0f;
	float AudioOutputVolume = 1.0f;
	bool bAudioInputMuted = false;
	bool bAudioOutputMuted = false;
	FString InputDeviceId;
	FString OutputDeviceId;
	EVoiceChatTransmitMode TransmitMode = EVoiceChatTransmitMode::All;
	FString SingleChannelTransmit;
	TMap<FString, FChannelSession> ChannelSessions;
	mutable FCriticalSection ChannelsCriticalSection;
	TMap<FString, bool> PlayerMutedMap;
	TMap<FString, float> PlayerVolumeMap;
	TSet<FString> BlockedPlayers;
	TMap<FString, FString> Settings;

	FOnVoiceChatAvailableAudioDevicesChangedDelegate OnVoiceChatAvailableAudioDevicesChangedDelegate;
	FOnVoiceChatLoggedInDelegate OnVoiceChatLoggedInDelegate;
	FOnVoiceChatLoggedOutDelegate OnVoiceChatLoggedOutDelegate;
	FOnVoiceChatChannelJoinedDelegate OnVoiceChatChannelJoinedDelegate;
	FOnVoiceChatChannelExitedDelegate OnVoiceChatChannelExitedDelegate;
	FOnVoiceChatCallStatsUpdatedDelegate OnVoiceChatCallStatsUpdatedDelegate;
	FOnVoiceChatPlayerAddedDelegate OnVoiceChatPlayerAddedDelegate;
	FOnVoiceChatPlayerRemovedDelegate OnVoiceChatPlayerRemovedDelegate;
	FOnVoiceChatPlayerTalkingUpdatedDelegate OnVoiceChatPlayerTalkingUpdatedDelegate;
	FOnVoiceChatPlayerMuteUpdatedDelegate OnVoiceChatPlayerMuteUpdatedDelegate;
	FOnVoiceChatPlayerVolumeUpdatedDelegate OnVoiceChatPlayerVolumeUpdatedDelegate;
	FOnVoiceChatRecordSamplesAvailableDelegate OnVoiceChatRecordSamplesAvailableDelegate;
	FOnVoiceChatAfterCaptureAudioReadDelegate OnVoiceChatAfterCaptureAudioReadDelegate;
	FOnVoiceChatBeforeCaptureAudioSentDelegate OnVoiceChatBeforeCaptureAudioSentDelegate;
	FOnVoiceChatBeforeRecvAudioRenderedDelegate OnVoiceChatBeforeRecvAudioRenderedDelegate;

	friend class FEOSVoiceChat;
};
#endif // WITH_EOS_RTC


