// Copyright (c) 2023 Betide Studio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "OnlineSubsystemEOK/SdkFunctions/EOK_SharedFunctionFile.h"
#include "Subsystems/GameInstanceSubsystem.h"

THIRD_PARTY_INCLUDES_START
#include "eos_rtc.h"
THIRD_PARTY_INCLUDES_END
#include "EOK_RTCSubsystem.generated.h"

USTRUCT(BlueprintType)
struct FEOK_RTC_ParticipantStatusChangedCallbackInfo
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, Category = "Epic Online Services-Kit V2 | SDK Functions | RTC Interface")
	FEOK_ProductUserId LocalUserId;

	UPROPERTY(BlueprintReadWrite, Category = "Epic Online Services-Kit V2 | SDK Functions | RTC Interface")
	FString RoomName;

	UPROPERTY(BlueprintReadWrite, Category = "Epic Online Services-Kit V2 | SDK Functions | RTC Interface")
	FEOK_ProductUserId ParticipantId;

	UPROPERTY(BlueprintReadWrite, Category = "Epic Online Services-Kit V2 | SDK Functions | RTC Interface")
	TEnumAsByte<EEOK_ERTCParticipantStatus> ParticipantStatus;

	//The participant metadata items. This is only set for the first notification where ParticipantStatus is EOS_RTCPS_Joined. Subsequent notifications such as when bParticipantInBlocklist changes will not contain any metadata.
	UPROPERTY(BlueprintReadWrite, Category = "Epic Online Services-Kit V2 | SDK Functions | RTC Interface")
	TArray<FEOK_RTC_ParticipantMetadata> ParticipantMetadata;

	//The participant's block list status, if ParticipantStatus is EOS_RTCPS_Joined. This is set to true if the participant is in any of the local user's applicable block lists, such Epic block list or any of the current platform's block lists. It can be used to detect when an internal automatic RTC block is applied because of trust and safety restrictions.
	UPROPERTY(BlueprintReadWrite, Category = "Epic Online Services-Kit V2 | SDK Functions | RTC Interface")
	bool bParticipantInBlocklist;

	FEOK_RTC_ParticipantStatusChangedCallbackInfo()
	{
		ParticipantStatus = EEOK_ERTCParticipantStatus::EOK_RTCPS_Left;
		bParticipantInBlocklist = false;
	}
	FEOK_RTC_ParticipantStatusChangedCallbackInfo(const EOS_RTC_ParticipantStatusChangedCallbackInfo* Data)
	{
		LocalUserId = Data->LocalUserId;
		RoomName = UTF8_TO_TCHAR(Data->RoomName);
		ParticipantId = Data->ParticipantId;
		ParticipantStatus = static_cast<EEOK_ERTCParticipantStatus>(Data->ParticipantStatus);
		bParticipantInBlocklist = Data->bParticipantInBlocklist ? true : false;
		for (uint32_t i = 0; i < Data->ParticipantMetadataCount; i++)
		{
			ParticipantMetadata.Add(Data->ParticipantMetadata[i]);
		}
	}
	
};

DECLARE_DYNAMIC_DELEGATE_ThreeParams(FEOK_RTC_OnDisconnectedCallback, const TEnumAsByte<EEOK_Result>, Result, const FEOK_ProductUserId&, LocalUserId, const FString&, RoomName);
DECLARE_DYNAMIC_DELEGATE_OneParam(FEOK_RTC_OnParticipantStatusChangedCallback, const FEOK_RTC_ParticipantStatusChangedCallbackInfo&, Data);
DECLARE_DYNAMIC_DELEGATE_ThreeParams(FEOK_RTC_OnRoomStatisticsUpdatedCallback, const FEOK_ProductUserId&, LocalUserId, const FString&, RoomName, const FString&, Statistic);
DECLARE_DYNAMIC_DELEGATE_FiveParams(FEOK_RTC_OnBlockParticipantCallback, const TEnumAsByte<EEOK_Result>, ResultCode, const FEOK_ProductUserId&, LocalUserId, const FString&, RoomName, const FEOK_ProductUserId&, ParticipantId, bool, bBlocked);
DECLARE_DYNAMIC_DELEGATE_FourParams(FEOK_RTC_OnJoinRoomCallback, const TEnumAsByte<EEOK_Result>, ResultCode, const FEOK_ProductUserId&, LocalUserId, const FString&, RoomName, const TArray<FEOK_RTC_Option>&, Options);
DECLARE_DYNAMIC_DELEGATE_ThreeParams(FEOK_RTC_OnLeaveRoomCallback, const TEnumAsByte<EEOK_Result>, ResultCode, const FEOK_ProductUserId&, LocalUserId, const FString&, RoomName);
DECLARE_DYNAMIC_DELEGATE_OneParam(FEOK_RTCAdmin_OnKickCompleteCallback, const TEnumAsByte<EEOK_Result>, ResultCode);
DECLARE_DYNAMIC_DELEGATE_FourParams(FEOK_RTCAdmin_OnQueryJoinRoomTokenCompleteCallback, const TEnumAsByte<EEOK_Result>, ResultCode, const FString&, RoomName, const FEOK_ProductUserId&, LocalUserId, const TArray<FEOK_RTCAdmin_UserToken>&, UserTokens);
DECLARE_DYNAMIC_DELEGATE_FourParams(FEOK_RTCAudio_OnAudioBeforeRenderCallback, const FEOK_ProductUserId&, LocalUserId, const FString&, RoomName, const FEOK_RTCAudio_AudioBuffer&, AudioBuffer, const FEOK_ProductUserId&, ParticipantId);
DECLARE_DYNAMIC_DELEGATE(FEOK_RTCAudio_OnAudioDevicesChangedCallback);
DECLARE_DYNAMIC_DELEGATE_ThreeParams(FEOK_RTCAudio_OnAudioInputStateCallback, const FEOK_ProductUserId&, LocalUserId, const FString&, RoomName, const TEnumAsByte<EEOK_ERTCAudioInputStatus>&, AudioInputState);
DECLARE_DYNAMIC_DELEGATE_ThreeParams(FEOK_RTCAudio_OnAudioOutputStateCallback, const FEOK_ProductUserId&, LocalUserId, const FString&, RoomName, const TEnumAsByte<EEOK_ERTCAudioOutputStatus>&, AudioOutputState);
DECLARE_DYNAMIC_DELEGATE_FiveParams(FEOK_RTCAudio_OnParticipantUpdatedCallback, const FEOK_ProductUserId&, LocalUserId, const FString&, RoomName, const FEOK_ProductUserId&, ParticipantId, bool,bSpeaking, const TEnumAsByte<EEOK_ERTCAudioStatus>&, AudioStatus);
DECLARE_DYNAMIC_DELEGATE_OneParam(FEOK_RTCAudio_OnQueryInputDevicesInformationCallback, const TEnumAsByte<EEOK_Result>, ResultCode);
DECLARE_DYNAMIC_DELEGATE_OneParam(FEOK_RTCAudio_OnQueryOutputDevicesInformationCallback, const TEnumAsByte<EEOK_Result>, ResultCode);
DECLARE_DYNAMIC_DELEGATE_TwoParams(FEOK_RTCAudio_OnRegisterPlatformUserCallback, const TEnumAsByte<EEOK_Result>, ResultCode, const FString&, PlatformUserId);
DECLARE_DYNAMIC_DELEGATE_TwoParams(FEOK_RTCAudio_OnSetInputDeviceSettingsCallback, const TEnumAsByte<EEOK_Result>, ResultCode, const FString&, DeviceId);
DECLARE_DYNAMIC_DELEGATE_TwoParams(FEOK_RTCAudio_OnSetOutputDeviceSettingsCallback, const TEnumAsByte<EEOK_Result>, ResultCode, const FString&, DeviceId);
DECLARE_DYNAMIC_DELEGATE_TwoParams(FEOK_RTCAudio_OnUnregisterPlatformUserCallback, const TEnumAsByte<EEOK_Result>, ResultCode, const FString&, PlatformUserId);
DECLARE_DYNAMIC_DELEGATE_TwoParams(FEOK_RTCAudio_OnUpdateParticipantVolumeCallback, const TEnumAsByte<EEOK_Result>, ResultCode, const FEOK_ProductUserId&, ParticipantId);
DECLARE_DYNAMIC_DELEGATE_TwoParams(FEOK_RTCAudio_OnUpdateReceivingCallback, const TEnumAsByte<EEOK_Result>, ResultCode, const FEOK_ProductUserId&, LocalUserId);
DECLARE_DYNAMIC_DELEGATE_TwoParams(FEOK_RTCAudio_OnUpdateReceivingVolumeCallback, const TEnumAsByte<EEOK_Result>, ResultCode, const FEOK_ProductUserId&, LocalUserId);
DECLARE_DYNAMIC_DELEGATE_TwoParams(FEOK_RTCAudio_OnUpdateSendingCallback, const TEnumAsByte<EEOK_Result>, ResultCode, const FEOK_ProductUserId&, LocalUserId);
DECLARE_DYNAMIC_DELEGATE_TwoParams(FEOK_RTCAudio_OnUpdateSendingVolumeCallback, const TEnumAsByte<EEOK_Result>, ResultCode, const FEOK_ProductUserId&, LocalUserId);
DECLARE_DYNAMIC_DELEGATE_FourParams(FEOK_RTCData_OnDataReceivedCallback, const FEOK_ProductUserId&, LocalUserId, const FString&, RoomName, const TArray<uint8>&, Data, const FEOK_ProductUserId&, ParticipantId);
DECLARE_DYNAMIC_DELEGATE_FourParams(FEOK_RTCData_OnParticipantUpdatedCallback, const FEOK_ProductUserId&, LocalUserId, const FString&, RoomName, const FEOK_ProductUserId&, ParticipantId, const TEnumAsByte<EEOK_ERTCDataStatus>&, DataStatus);
DECLARE_DYNAMIC_DELEGATE_FourParams(FEOK_RTCData_OnUpdateReceivingCallback, const TEnumAsByte<EEOK_Result>, ResultCode, const FEOK_ProductUserId&, LocalUserId, const FString&, RoomName, const FEOK_ProductUserId&, ParticipantId);
DECLARE_DYNAMIC_DELEGATE_TwoParams(FEOK_RTCData_OnUpdateSendingCallback, const TEnumAsByte<EEOK_Result>, ResultCode, const FEOK_ProductUserId&, LocalUserId);
UCLASS(DisplayName = "RTC Interface", meta = (DisplayName = "RTC Interface"))
class OnlineSubsystemEOK_API UEOK_RTCSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	
	FEOK_RTC_OnDisconnectedCallback OnDisconnectedCallback;
	//Register to receive notifications when disconnected from the room. If the returned NotificationId is valid, you must call EOS_RTC_RemoveNotifyDisconnected when you no longer wish to have your CompletionDelegate called. This function will always return EOS_INVALID_NOTIFICATIONID when used with lobby RTC room. To be notified of the connection status of a Lobby-managed RTC room, use the EOS_Lobby_AddNotifyRTCRoomConnectionChanged function instead.
	UFUNCTION(BlueprintCallable, Category = "Epic Online Services-Kit V2 | SDK Functions | RTC Interface", DisplayName="EOS_RTC_AddNotifyDisconnected")
	FEOK_NotificationId EOK_RTC_AddNotifyDisconnected(FEOK_ProductUserId LocalUserId, const FString& RoomName, const FEOK_RTC_OnDisconnectedCallback& Callback);

	FEOK_RTC_OnParticipantStatusChangedCallback OnParticipantStatusChangedCallback;
	//Register to receive notifications when a participant's status changes (e.g: join or leave the room), or when the participant is added or removed from an applicable block list (e.g: Epic block list and/or current platform's block list). If the returned NotificationId is valid, you must call EOS_RTC_RemoveNotifyParticipantStatusChanged when you no longer wish to have your CompletionDelegate called. If you register to this notification before joining a room, you will receive a notification for every member already in the room when you join said room. This allows you to know who is already in the room when you join. To be used effectively with a Lobby-managed RTC room, this should be registered during the EOS_Lobby_CreateLobby or EOS_Lobby_JoinLobby completion callbacks when the ResultCode is EOS_Success. If this notification is registered after that point, it is possible to miss notifications for already-existing room participants. You can use this notification to detect internal automatic RTC blocks due to block lists. When a participant joins a room and while the system resolves the block list status of said participant, the participant is set to blocked and you'll receive a notification with ParticipantStatus set to EOS_RTCPS_Joined and bParticipantInBlocklist set to true. Once the block list status is resolved, if the player is not in any applicable block list(s), it is then unblocked and a new notification is sent with ParticipantStatus set to EOS_RTCPS_Joined and bParticipantInBlocklist set to false.
	UFUNCTION(BlueprintCallable, Category = "Epic Online Services-Kit V2 | SDK Functions | RTC Interface", DisplayName="EOS_RTC_AddNotifyParticipantStatusChanged")
	FEOK_NotificationId EOK_RTC_AddNotifyParticipantStatusChanged(FEOK_ProductUserId LocalUserId, const FString& RoomName, const FEOK_RTC_OnParticipantStatusChangedCallback& Callback);
	
	FEOK_RTC_OnRoomStatisticsUpdatedCallback OnRoomStatisticsUpdatedCallback;
	//Register to receive notifications to receiving periodical statistics update. If the returned NotificationId is valid, you must call EOS_RTC_RemoveNotifyRoomStatisticsUpdated when you no longer wish to have your StatisticsUpdateHandler called.
	UFUNCTION(BlueprintCallable, Category = "Epic Online Services-Kit V2 | SDK Functions | RTC Interface", DisplayName="EOS_RTC_AddNotifyRoomStatisticsUpdated")
	FEOK_NotificationId EOK_RTC_AddNotifyRoomStatisticsUpdated(FEOK_ProductUserId LocalUserId, const FString& RoomName, const FEOK_RTC_OnRoomStatisticsUpdatedCallback& Callback);

	FEOK_RTC_OnBlockParticipantCallback OnBlockParticipantCallback;
	//Use this function to block a participant already connected to the room. After blocking them no media will be sent or received between that user and the local user. This method can be used after receiving the OnParticipantStatusChanged notification.
	UFUNCTION(BlueprintCallable, Category = "Epic Online Services-Kit V2 | SDK Functions | RTC Interface", DisplayName="EOS_RTC_BlockParticipant")
	void EOK_RTC_BlockParticipant(FEOK_ProductUserId LocalUserId, const FString& RoomName, FEOK_ProductUserId ParticipantId, bool bBlocked, const FEOK_RTC_OnBlockParticipantCallback& Callback);

	FEOK_RTC_OnJoinRoomCallback OnJoinRoomCallback;
	//Use this function to join a room. This function does not need to called for the Lobby RTC Room system; doing so will return EOS_AccessDenied. The lobby system will automatically join and leave RTC Rooms for all lobbies that have RTC rooms enabled.
	UFUNCTION(BlueprintCallable, Category = "Epic Online Services-Kit V2 | SDK Functions | RTC Interface", DisplayName="EOS_RTC_JoinRoom")
	void EOK_RTC_JoinRoom(FEOK_ProductUserId LocalUserId, const FString& RoomName, const FString& ClientBaseUrl, const FString& ParticipantToken, FEOK_ProductUserId ParticipantId, bool bEnabledEcho, bool bManualAudioInputEnabled, bool bManualAudioOutputEnabled, const FEOK_RTC_OnJoinRoomCallback& Callback);

	FEOK_RTC_OnLeaveRoomCallback OnLeaveRoomCallback;
	//Use this function to leave a room and clean up all the resources associated with it. This function has to always be called when the room is abandoned even if the user is already disconnected for other reasons. This function does not need to called for the Lobby RTC Room system; doing so will return EOS_AccessDenied. The lobby system will automatically join and leave RTC Rooms for all lobbies that have RTC rooms enabled.
	UFUNCTION(BlueprintCallable, Category = "Epic Online Services-Kit V2 | SDK Functions | RTC Interface", DisplayName="EOS_RTC_LeaveRoom")
	void EOK_RTC_LeaveRoom(FEOK_ProductUserId LocalUserId, const FString& RoomName, const FEOK_RTC_OnLeaveRoomCallback& Callback);

	//Unregister a previously bound notification handler from receiving room disconnection notifications
	UFUNCTION(BlueprintCallable, Category = "Epic Online Services-Kit V2 | SDK Functions | RTC Interface", DisplayName="EOS_RTC_RemoveNotifyDisconnected")
	void EOK_RTC_RemoveNotifyDisconnected(FEOK_NotificationId NotificationId);

	//Unregister a previously bound notification handler from receiving participant status change notifications
	UFUNCTION(BlueprintCallable, Category = "Epic Online Services-Kit V2 | SDK Functions | RTC Interface", DisplayName="EOS_RTC_RemoveNotifyParticipantStatusChanged")
	void EOK_RTC_RemoveNotifyParticipantStatusChanged(FEOK_NotificationId NotificationId);

	//Unregister a previously bound notification handler from receiving periodical statistics update notifications
	UFUNCTION(BlueprintCallable, Category = "Epic Online Services-Kit V2 | SDK Functions | RTC Interface", DisplayName="EOS_RTC_RemoveNotifyRoomStatisticsUpdated")
	void EOK_RTC_RemoveNotifyRoomStatisticsUpdated(FEOK_NotificationId NotificationId);

	//Use this function to control settings for the specific room. The available settings are documented as part of EOS_RTC_SetRoomSettingOptions.
	UFUNCTION(BlueprintCallable, Category = "Epic Online Services-Kit V2 | SDK Functions | RTC Interface", DisplayName="EOS_RTC_SetRoomSetting")
	TEnumAsByte<EEOK_Result> EOK_RTC_SetRoomSetting(FEOK_ProductUserId LocalUserId, const FString& RoomName, const FString& SettingName, const FString& SettingValue);
	
	//Use this function to control settings. The available settings are documented as part of EOS_RTC_SetSettingOptions.
	UFUNCTION(BlueprintCallable, Category = "Epic Online Services-Kit V2 | SDK Functions | RTC Interface", DisplayName="EOS_RTC_SetSetting")
	TEnumAsByte<EEOK_Result> EOK_RTC_SetSetting(const FString& SettingName, const FString& SettingValue);

	//Fetches a user token when called inside of the OnQueryJoinRoomTokenComplete callback. initiating the query.
	UFUNCTION(BlueprintCallable, Category = "Epic Online Services-Kit V2 | SDK Functions | RTC Interface", DisplayName="EOS_RTCAdmin_CopyUserTokenByIndex")
	TEnumAsByte<EEOK_Result> EOK_RTCAdmin_CopyUserTokenByIndex(int32 UserTokenIndex, int32 QueryId, FEOK_RTCAdmin_UserToken& OutUserToken);

	//Fetches a user token for a given user ID when called inside of the OnQueryJoinRoomTokenComplete callback.
	UFUNCTION(BlueprintCallable, Category = "Epic Online Services-Kit V2 | SDK Functions | RTC Interface", DisplayName="EOS_RTCAdmin_CopyUserTokenByUserId")
	TEnumAsByte<EEOK_Result> EOK_RTCAdmin_CopyUserTokenByUserId(FEOK_ProductUserId UserId, int32 QueryId, FEOK_RTCAdmin_UserToken& OutUserToken);

	FEOK_RTCAdmin_OnKickCompleteCallback OnKickCompleteCallback;
	//Starts an asynchronous task that removes a participant from a room and revokes their token.
	UFUNCTION(BlueprintCallable, Category = "Epic Online Services-Kit V2 | SDK Functions | RTC Interface", DisplayName="EOS_RTCAdmin_Kick")
	void EOK_RTCAdmin_Kick(const FString& RoomName, FEOK_ProductUserId TargetUserId, const FEOK_RTCAdmin_OnKickCompleteCallback& Callback);

	FEOK_RTCAdmin_OnQueryJoinRoomTokenCompleteCallback OnQueryJoinRoomTokenCompleteCallback;
	//Query for a list of user tokens for joining a room. Each query generates a query id ( see EOS_RTCAdmin_QueryJoinRoomTokenCompleteCallbackInfo ) which should be used to retrieve the tokens from inside the callback. This query id and query result itself are only valid for the duration of the callback.
	UFUNCTION(BlueprintCallable, Category = "Epic Online Services-Kit V2 | SDK Functions | RTC Interface", DisplayName="EOS_RTCAdmin_QueryJoinRoomToken")
	TEnumAsByte<EEOK_Result> EOK_RTCAdmin_QueryJoinRoomToken(const FString& RoomName, FEOK_ProductUserId LocalUserId, const TArray<FEOK_ProductUserId>& TargetUserIds, TArray<FString>& TargetUserIpAddresses, const FEOK_RTCAdmin_OnQueryJoinRoomTokenCompleteCallback& Callback);

	FEOK_RTCAudio_OnAudioBeforeRenderCallback OnAudioBeforeRenderCallback;
	//Register to receive notifications with remote audio buffers before they are rendered. This gives you access to the audio data received, allowing for example the implementation of custom filters/effects. If the returned NotificationId is valid, you must call EOS_RTCAudio_RemoveNotifyAudioBeforeRender when you no longer wish to have your CompletionDelegate called.
	UFUNCTION(BlueprintCallable, Category = "Epic Online Services-Kit V2 | SDK Functions | RTC Interface", DisplayName="EOS_RTCAudio_AddNotifyAudioBeforeRender")
	FEOK_NotificationId EOK_RTCAudio_AddNotifyAudioBeforeRender(FEOK_ProductUserId LocalUserId, const FString& RoomName, bool bUnmixedAudio, const FEOK_RTCAudio_OnAudioBeforeRenderCallback& Callback);

	FEOK_RTCAudio_OnAudioDevicesChangedCallback OnAudioDevicesChangedCallback;
	/*
	*Register to receive notifications when an audio device is added or removed to the system. If the returned NotificationId is valid, you must call EOS_RTCAudio_RemoveNotifyAudioDevicesChanged when you no longer wish to have your CompletionDelegate called. The library will try to use user selected audio device while following these rules:

	if none of the audio devices has been available and connected before - the library will try to use it;
	if user selected device failed for some reason, default device will be used instead (and user selected device will be memorized);
	if user selected a device but it was not used for some reason (and default was used instead), when devices selection is triggered we will try to use user selected device again;
	triggers to change a device: when new audio device appears or disappears - library will try to use previously user selected;
	if for any reason, a device cannot be used - the library will fallback to using default;
	if a configuration of the current audio device has been changed, it will be restarted.
	*/
	UFUNCTION(BlueprintCallable, Category = "Epic Online Services-Kit V2 | SDK Functions | RTC Interface", DisplayName="EOS_RTCAudio_AddNotifyAudioDevicesChanged")
	FEOK_NotificationId EOK_RTCAudio_AddNotifyAudioDevicesChanged(const FEOK_RTCAudio_OnAudioDevicesChangedCallback& Callback);


	FEOK_RTCAudio_OnAudioInputStateCallback OnAudioInputStateCallback;
	//Register to receive notifications when audio input state changed. If the returned NotificationId is valid, you must call EOS_RTCAudio_RemoveNotifyAudioInputState when you no longer wish to have your CompletionDelegate called.
	UFUNCTION(BlueprintCallable, Category = "Epic Online Services-Kit V2 | SDK Functions | RTC Interface", DisplayName="EOS_RTCAudio_AddNotifyAudioInputState")
	FEOK_NotificationId EOK_RTCAudio_AddNotifyAudioInputState(FEOK_ProductUserId LocalUserId, const FString& RoomName, const FEOK_RTCAudio_OnAudioInputStateCallback& Callback);

	FEOK_RTCAudio_OnAudioOutputStateCallback OnAudioOutputStateCallback;
	//Register to receive notifications when audio output state changed. If the returned NotificationId is valid, you must call EOS_RTCAudio_RemoveNotifyAudioOutputState when you no longer wish to have your CompletionDelegate called.
	UFUNCTION(BlueprintCallable, Category = "Epic Online Services-Kit V2 | SDK Functions | RTC Interface", DisplayName="EOS_RTCAudio_AddNotifyAudioOutputState")
	FEOK_NotificationId EOK_RTCAudio_AddNotifyAudioOutputState(FEOK_ProductUserId LocalUserId, const FString& RoomName, const FEOK_RTCAudio_OnAudioOutputStateCallback& Callback);

	FEOK_RTCAudio_OnParticipantUpdatedCallback OnParticipantUpdatedCallback;
	//Register to receive notifications when a room participant audio status is updated (f.e when mute state changes or speaking flag changes). The notification is raised when the participant's audio status is updated. In order not to miss any participant status changes, applications need to add the notification before joining a room. If the returned NotificationId is valid, you must call EOS_RTCAudio_RemoveNotifyParticipantUpdated when you no longer wish to have your CompletionDelegate called.
	UFUNCTION(BlueprintCallable, Category = "Epic Online Services-Kit V2 | SDK Functions | RTC Interface", DisplayName="EOS_RTCAudio_AddNotifyParticipantUpdated")
	FEOK_NotificationId EOK_RTCAudio_AddNotifyParticipantUpdated(FEOK_ProductUserId LocalUserId, const FString& RoomName, const FEOK_RTCAudio_OnParticipantUpdatedCallback& Callback);

	//Fetches an audio input device's information from then given index that are cached locally.
	UFUNCTION(BlueprintCallable, Category = "Epic Online Services-Kit V2 | SDK Functions | RTC Interface", DisplayName="EOS_RTCAudio_CopyInputDeviceInformationByIndex")
	TEnumAsByte<EEOK_Result> EOK_RTCAudio_CopyInputDeviceInformationByIndex(int32 DeviceIndex, FEOK_RTCAudio_InputDeviceInformation& OutDeviceInfo);

	//Fetches an audio output device's information from then given index that are cached locally.
	UFUNCTION(BlueprintCallable, Category = "Epic Online Services-Kit V2 | SDK Functions | RTC Interface", DisplayName="EOS_RTCAudio_CopyOutputDeviceInformationByIndex")
	TEnumAsByte<EEOK_Result> EOK_RTCAudio_CopyOutputDeviceInformationByIndex(int32 DeviceIndex, FEOK_RTCAudio_OutputDeviceInformation& OutDeviceInfo);

	//Fetch the number of audio input devices available in the system that are cached locally. The returned value should not be cached and should instead be used immediately with the EOS_RTCAudio_CopyInputDeviceInformationByIndex function.
	UFUNCTION(BlueprintCallable, Category = "Epic Online Services-Kit V2 | SDK Functions | RTC Interface", DisplayName="EOS_RTCAudio_GetInputDevicesCount")
	int32 EOK_RTCAudio_GetInputDevicesCount();

	//Fetch the number of audio output devices available in the system that are cached locally. The returned value should not be cached and should instead be used immediately with the EOS_RTCAudio_CopyOutputDeviceInformationByIndex function.
	UFUNCTION(BlueprintCallable, Category = "Epic Online Services-Kit V2 | SDK Functions | RTC Interface", DisplayName="EOS_RTCAudio_GetOutputDevicesCount")
	int32 EOK_RTCAudio_GetOutputDevicesCount();

	//Release the memory associated with EOS_RTCAudio_InputDeviceInformation. This must be called on data retrieved from EOS_RTCAudio_CopyInputDeviceInformationByIndex.
	UFUNCTION(BlueprintCallable, Category = "Epic Online Services-Kit V2 | SDK Functions | RTC Interface", DisplayName="EOS_RTCAudio_InputDeviceInformation_Release")
	void EOK_RTCAudio_InputDeviceInformation_Release(FEOK_RTCAudio_InputDeviceInformation& DeviceInfo);

	//Release the memory associated with EOS_RTCAudio_OutputDeviceInformation. This must be called on data retrieved from EOS_RTCAudio_CopyOutputDeviceInformationByIndex.
	UFUNCTION(BlueprintCallable, Category = "Epic Online Services-Kit V2 | SDK Functions | RTC Interface", DisplayName="EOS_RTCAudio_OutputDeviceInformation_Release")
	void EOK_RTCAudio_OutputDeviceInformation_Release(FEOK_RTCAudio_OutputDeviceInformation& DeviceInfo);

	FEOK_RTCAudio_OnQueryInputDevicesInformationCallback OnQueryInputDevicesInformationCallback;
	//Query for a list of audio input devices available in the system together with their specifications.
	UFUNCTION(BlueprintCallable, Category = "Epic Online Services-Kit V2 | SDK Functions | RTC Interface", DisplayName="EOS_RTCAudio_QueryInputDevicesInformation")
	void EOK_RTCAudio_QueryInputDevicesInformation(const FEOK_RTCAudio_OnQueryInputDevicesInformationCallback& Callback);

	FEOK_RTCAudio_OnQueryOutputDevicesInformationCallback OnQueryOutputDevicesInformationCallback;
	//Query for a list of audio output devices available in the system together with their specifications.
	UFUNCTION(BlueprintCallable, Category = "Epic Online Services-Kit V2 | SDK Functions | RTC Interface", DisplayName="EOS_RTCAudio_QueryOutputDevicesInformation")
	void EOK_RTCAudio_QueryOutputDevicesInformation(const FEOK_RTCAudio_OnQueryOutputDevicesInformationCallback& Callback);

	FEOK_RTCAudio_OnRegisterPlatformUserCallback OnRegisterPlatformUserCallback;
	//Use this function to inform the audio system of a user. This function is only necessary for some platforms.
	UFUNCTION(BlueprintCallable, Category = "Epic Online Services-Kit V2 | SDK Functions | RTC Interface", DisplayName="EOS_RTCAudio_RegisterPlatformUser")
	void EOK_RTCAudio_RegisterPlatformUser(const FString& PlatformUserId, const FEOK_RTCAudio_OnRegisterPlatformUserCallback& Callback);

	//Unregister a previously bound notification handler from receiving remote audio buffers before they are rendered.
	UFUNCTION(BlueprintCallable, Category = "Epic Online Services-Kit V2 | SDK Functions | RTC Interface", DisplayName="EOS_RTCAudio_RemoveNotifyAudioBeforeRender")
	void EOK_RTCAudio_RemoveNotifyAudioBeforeRender(FEOK_NotificationId NotificationId);

	//Unregister a previously bound notification handler from receiving local audio buffers before they are encoded and sent.
	UFUNCTION(BlueprintCallable, Category = "Epic Online Services-Kit V2 | SDK Functions | RTC Interface", DisplayName="EOS_RTCAudio_RemoveNotifyAudioBeforeSend")
	void EOK_RTCAudio_RemoveNotifyAudioBeforeSend(FEOK_NotificationId NotificationId);

	//Unregister a previously bound notification handler from receiving audio devices notifications
	UFUNCTION(BlueprintCallable, Category = "Epic Online Services-Kit V2 | SDK Functions | RTC Interface", DisplayName="EOS_RTCAudio_RemoveNotifyAudioDevicesChanged")
	void EOK_RTCAudio_RemoveNotifyAudioDevicesChanged(FEOK_NotificationId NotificationId);

	//Unregister a previously bound notification handler from receiving notifications on audio input state changed.
	UFUNCTION(BlueprintCallable, Category = "Epic Online Services-Kit V2 | SDK Functions | RTC Interface", DisplayName="EOS_RTCAudio_RemoveNotifyAudioInputState")
	void EOK_RTCAudio_RemoveNotifyAudioInputState(FEOK_NotificationId NotificationId);

	//Unregister a previously bound notification handler from receiving notifications on audio output state changed.
	UFUNCTION(BlueprintCallable, Category = "Epic Online Services-Kit V2 | SDK Functions | RTC Interface", DisplayName="EOS_RTCAudio_RemoveNotifyAudioOutputState")
	void EOK_RTCAudio_RemoveNotifyAudioOutputState(FEOK_NotificationId NotificationId);

	//Unregister a previously bound notification handler from receiving participant updated notifications
	UFUNCTION(BlueprintCallable, Category = "Epic Online Services-Kit V2 | SDK Functions | RTC Interface", DisplayName="EOS_RTCAudio_RemoveNotifyParticipantUpdated")
	void EOK_RTCAudio_RemoveNotifyParticipantUpdated(FEOK_NotificationId NotificationId);

	//Use this function to push a new audio buffer to be sent to the participants of a room. This should only be used if Manual Audio Input was enabled locally for the specified room.
	UFUNCTION(BlueprintCallable, Category = "Epic Online Services-Kit V2 | SDK Functions | RTC Interface", DisplayName="EOS_RTCAudio_SendAudio")
	TEnumAsByte<EEOK_Result> EOK_RTCAudio_SendAudio(FEOK_ProductUserId LocalUserId, const FString& RoomName, const FEOK_RTCAudio_AudioBuffer& AudioBuffer);

	FEOK_RTCAudio_OnSetInputDeviceSettingsCallback OnSetInputDeviceSettingsCallback;
	//Use this function to set audio input device settings, such as the active input device, or platform AEC.
	UFUNCTION(BlueprintCallable, Category = "Epic Online Services-Kit V2 | SDK Functions | RTC Interface", DisplayName="EOS_RTCAudio_SetInputDeviceSettings")
	void EOK_RTCAudio_SetInputDeviceSettings(FEOK_ProductUserId LocalUserId, const FString& RealDeviceId, bool bPlatformAEC, const FEOK_RTCAudio_OnSetInputDeviceSettingsCallback& Callback);

	FEOK_RTCAudio_OnSetOutputDeviceSettingsCallback OnSetOutputDeviceSettingsCallback;
	//Use this function to set audio output device settings, such as the active output device.
	UFUNCTION(BlueprintCallable, Category = "Epic Online Services-Kit V2 | SDK Functions | RTC Interface", DisplayName="EOS_RTCAudio_SetOutputDeviceSettings")
	void EOK_RTCAudio_SetOutputDeviceSettings(FEOK_ProductUserId LocalUserId, const FString& RealDeviceId, const FEOK_RTCAudio_OnSetOutputDeviceSettingsCallback& Callback);

	FEOK_RTCAudio_OnUnregisterPlatformUserCallback OnUnregisterPlatformUserCallback;
	//Use this function to remove a user that was added with EOS_RTCAudio_RegisterPlatformUser. This function is only necessary for some platforms.
	UFUNCTION(BlueprintCallable, Category = "Epic Online Services-Kit V2 | SDK Functions | RTC Interface", DisplayName="EOS_RTCAudio_UnregisterPlatformUser")
	void EOK_RTCAudio_UnregisterPlatformUser(const FString& PlatformUserId, const FEOK_RTCAudio_OnUnregisterPlatformUserCallback& Callback);

	FEOK_RTCAudio_OnUpdateParticipantVolumeCallback OnUpdateParticipantVolumeCallback;
	//Use this function to change participant audio volume for a room.
	UFUNCTION(BlueprintCallable, Category = "Epic Online Services-Kit V2 | SDK Functions | RTC Interface", DisplayName="EOS_RTCAudio_UpdateParticipantVolume")
	void EOK_RTCAudio_UpdateParticipantVolume(FEOK_ProductUserId LocalUserId, const FString& RoomName, FEOK_ProductUserId ParticipantId, float Volume, const FEOK_RTCAudio_OnUpdateParticipantVolumeCallback& Callback);

	FEOK_RTCAudio_OnUpdateReceivingCallback OnUpdateReceivingCallback;
	//Use this function to tweak incoming audio options for a room.
	UFUNCTION(BlueprintCallable, Category = "Epic Online Services-Kit V2 | SDK Functions | RTC Interface", DisplayName="EOS_RTCAudio_UpdateReceiving")
	void EOK_RTCAudio_UpdateReceiving(FEOK_ProductUserId LocalUserId, const FString& RoomName, FEOK_ProductUserId ParticipantId, bool bAudioEnabled, const FEOK_RTCAudio_OnUpdateReceivingCallback& Callback);

	
	FEOK_RTCAudio_OnUpdateReceivingVolumeCallback OnUpdateReceivingVolumeCallback;
	//Use this function to change incoming audio volume for a room.
	UFUNCTION(BlueprintCallable, Category = "Epic Online Services-Kit V2 | SDK Functions | RTC Interface", DisplayName="EOS_RTCAudio_UpdateReceivingVolume")
	void EOK_RTCAudio_UpdateReceivingVolume(FEOK_ProductUserId LocalUserId, const FString& RoomName, float Volume, const FEOK_RTCAudio_OnUpdateReceivingVolumeCallback& Callback);

	FEOK_RTCAudio_OnUpdateSendingCallback OnUpdateSendingCallback;
	//Use this function to tweak outgoing audio options for a room.
	UFUNCTION(BlueprintCallable, Category = "Epic Online Services-Kit V2 | SDK Functions | RTC Interface", DisplayName="EOS_RTCAudio_UpdateSending")
	void EOK_RTCAudio_UpdateSending(FEOK_ProductUserId LocalUserId, const FString& RoomName, TEnumAsByte<EEOK_ERTCAudioStatus> AudioStatus, const FEOK_RTCAudio_OnUpdateSendingCallback& Callback);

	FEOK_RTCAudio_OnUpdateSendingVolumeCallback OnUpdateSendingVolumeCallback;
	//Use this function to change outgoing audio volume for a room.
	UFUNCTION(BlueprintCallable, Category = "Epic Online Services-Kit V2 | SDK Functions | RTC Interface", DisplayName="EOS_RTCAudio_UpdateSendingVolume")
	void EOK_RTCAudio_UpdateSendingVolume(FEOK_ProductUserId LocalUserId, const FString& RoomName, float Volume, const FEOK_RTCAudio_OnUpdateSendingVolumeCallback& Callback);

	FEOK_RTCData_OnDataReceivedCallback OnDataReceivedCallback;
	//Register to receive notifications with remote data packet received. If the returned NotificationId is valid, you must call EOS_RTCData_RemoveNotifyDataReceived when you no longer wish to have your CompletionDelegate called.
	UFUNCTION(BlueprintCallable, Category = "Epic Online Services-Kit V2 | SDK Functions | RTC Interface", DisplayName="EOS_RTCData_AddNotifyDataReceived")
	FEOK_NotificationId EOK_RTCData_AddNotifyDataReceived(FEOK_ProductUserId LocalUserId, const FString& RoomName, const FEOK_RTCData_OnDataReceivedCallback& Callback);

	FEOK_RTCData_OnParticipantUpdatedCallback OnData_ParticipantUpdatedCallback;
	//Register to receive notifications when a room participant data status is updated (f.e when connection state changes). The notification is raised when the participant's data status is updated. In order not to miss any participant status changes, applications need to add the notification before joining a room. If the returned NotificationId is valid, you must call EOS_RTCData_RemoveNotifyParticipantUpdated when you no longer wish to have your CompletionDelegate called.
	UFUNCTION(BlueprintCallable, Category = "Epic Online Services-Kit V2 | SDK Functions | RTC Interface", DisplayName="EOS_RTCData_AddNotifyParticipantUpdated")
	FEOK_NotificationId EOK_RTCData_AddNotifyParticipantUpdated(FEOK_ProductUserId LocalUserId, const FString& RoomName, const FEOK_RTCData_OnParticipantUpdatedCallback& Callback);

	//Unregister a previously bound notification handler from receiving remote data packets.
	UFUNCTION(BlueprintCallable, Category = "Epic Online Services-Kit V2 | SDK Functions | RTC Interface", DisplayName="EOS_RTCData_RemoveNotifyDataReceived")
	void EOK_RTCData_RemoveNotifyDataReceived(FEOK_NotificationId NotificationId);

	//Unregister a previously bound notification handler from receiving participant updated notifications
	UFUNCTION(BlueprintCallable, Category = "Epic Online Services-Kit V2 | SDK Functions | RTC Interface", DisplayName="EOS_RTCData_RemoveNotifyParticipantUpdated")
	void EOK_RTCData_RemoveNotifyParticipantUpdated(FEOK_NotificationId NotificationId);

	//Use this function to send a data packet to the rest of participants.
	UFUNCTION(BlueprintCallable, Category = "Epic Online Services-Kit V2 | SDK Functions | RTC Interface", DisplayName="EOS_RTCData_SendData")
	TEnumAsByte<EEOK_Result> EOK_RTCData_SendData(FEOK_ProductUserId LocalUserId, const FString& RoomName, const TArray<uint8>& Data);

	FEOK_RTCData_OnUpdateReceivingCallback OnData_UpdateReceivingCallback;
	//Use this function to tweak incoming data options for a room.
	UFUNCTION(BlueprintCallable, Category = "Epic Online Services-Kit V2 | SDK Functions | RTC Interface", DisplayName="EOS_RTCData_UpdateReceiving")
	void EOK_RTCData_UpdateReceiving(FEOK_ProductUserId LocalUserId, const FString& RoomName, FEOK_ProductUserId ParticipantId, bool bDataEnabled, const FEOK_RTCData_OnUpdateReceivingCallback& Callback);

	FEOK_RTCData_OnUpdateSendingCallback OnData_UpdateSendingCallback;
	//Use this function to tweak outgoing data options for a room.
	UFUNCTION(BlueprintCallable, Category = "Epic Online Services-Kit V2 | SDK Functions | RTC Interface", DisplayName="EOS_RTCData_UpdateSending")
	void EOK_RTCData_UpdateSending(FEOK_ProductUserId LocalUserId, const FString& RoomName, bool bDataEnabled, const FEOK_RTCData_OnUpdateSendingCallback& Callback);
};
