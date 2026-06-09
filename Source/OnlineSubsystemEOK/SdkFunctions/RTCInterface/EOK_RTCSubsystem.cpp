// Copyright (c) 2023 Betide Studio. All Rights Reserved.


#include "EOK_RTCSubsystem.h"

#include "eos_rtc_admin.h"
#include "eos_rtc_data.h"
#include "OnlineSubsystemEOS.h"
#include "OnlineSubsystemEOK/SdkFunctions/ConnectInterface/EOK_ConnectSubsystem.h"

FEOK_NotificationId UEOK_RTCSubsystem::EOK_RTC_AddNotifyDisconnected(FEOK_ProductUserId LocalUserId,
                                                                     const FString& RoomName, const FEOK_RTC_OnDisconnectedCallback& Callback)
{
	OnDisconnectedCallback = Callback;
	if (IOnlineSubsystem* OnlineSub = IOnlineSubsystem::Get("EOK"))
	{
		if (FOnlineSubsystemEOS* EOSRef = static_cast<FOnlineSubsystemEOS*>(OnlineSub))
		{
			EOS_RTC_AddNotifyDisconnectedOptions Options = { };
			Options.ApiVersion = EOS_RTC_ADDNOTIFYDISCONNECTED_API_LATEST;
			Options.LocalUserId = LocalUserId.GetValueAsEosType();
			Options.RoomName = TCHAR_TO_ANSI(*RoomName);
			return EOS_RTC_AddNotifyDisconnected(EOS_Platform_GetRTCInterface(EOSRef->EOSPlatformHandle->PlatformHandle), &Options, this, [](const EOS_RTC_DisconnectedCallbackInfo* Data)
			{
				UEOK_RTCSubsystem* Subsystem = static_cast<UEOK_RTCSubsystem*>(Data->ClientData);
				if (Subsystem)
				{
					Subsystem->OnDisconnectedCallback.ExecuteIfBound(static_cast<EEOK_Result>(Data->ResultCode), Data->LocalUserId, UTF8_TO_TCHAR(Data->RoomName));
				}
			});
		}
	}
	return FEOK_NotificationId();
}

FEOK_NotificationId UEOK_RTCSubsystem::EOK_RTC_AddNotifyParticipantStatusChanged(FEOK_ProductUserId LocalUserId,
	const FString& RoomName, const FEOK_RTC_OnParticipantStatusChangedCallback& Callback)
{
	OnParticipantStatusChangedCallback = Callback;
	if (IOnlineSubsystem* OnlineSub = IOnlineSubsystem::Get("EOK"))
	{
		if (FOnlineSubsystemEOS* EOSRef = static_cast<FOnlineSubsystemEOS*>(OnlineSub))
		{
			EOS_RTC_AddNotifyParticipantStatusChangedOptions Options = { };
			Options.ApiVersion = EOS_RTC_ADDNOTIFYPARTICIPANTSTATUSCHANGED_API_LATEST;
			Options.LocalUserId = LocalUserId.GetValueAsEosType();
			Options.RoomName = TCHAR_TO_ANSI(*RoomName);
			return EOS_RTC_AddNotifyParticipantStatusChanged(EOS_Platform_GetRTCInterface(EOSRef->EOSPlatformHandle->PlatformHandle), &Options, this, [](const EOS_RTC_ParticipantStatusChangedCallbackInfo* Data)
			{
				UEOK_RTCSubsystem* Subsystem = static_cast<UEOK_RTCSubsystem*>(Data->ClientData);
				if (Subsystem)
				{
					FEOK_RTC_ParticipantStatusChangedCallbackInfo CallbackInfo(Data);
					Subsystem->OnParticipantStatusChangedCallback.ExecuteIfBound(CallbackInfo);
				}
			});
		}
	}
	return FEOK_NotificationId();
}

FEOK_NotificationId UEOK_RTCSubsystem::EOK_RTC_AddNotifyRoomStatisticsUpdated(FEOK_ProductUserId LocalUserId,
	const FString& RoomName, const FEOK_RTC_OnRoomStatisticsUpdatedCallback& Callback)
{
	OnRoomStatisticsUpdatedCallback = Callback;
	if (IOnlineSubsystem* OnlineSub = IOnlineSubsystem::Get("EOK"))
	{
		if (FOnlineSubsystemEOS* EOSRef = static_cast<FOnlineSubsystemEOS*>(OnlineSub))
		{
			EOS_RTC_AddNotifyRoomStatisticsUpdatedOptions Options = { };
			Options.ApiVersion = EOS_RTC_ADDNOTIFYROOMSTATISTICSUPDATED_API_LATEST;
			Options.LocalUserId = LocalUserId.GetValueAsEosType();
			Options.RoomName = TCHAR_TO_ANSI(*RoomName);
			return EOS_RTC_AddNotifyRoomStatisticsUpdated(EOS_Platform_GetRTCInterface(EOSRef->EOSPlatformHandle->PlatformHandle), &Options, this, [](const EOS_RTC_RoomStatisticsUpdatedInfo* Data)
			{
				UEOK_RTCSubsystem* Subsystem = static_cast<UEOK_RTCSubsystem*>(Data->ClientData);
				if (Subsystem)
				{
					Subsystem->OnRoomStatisticsUpdatedCallback.ExecuteIfBound(Data->LocalUserId, UTF8_TO_TCHAR(Data->RoomName), UTF8_TO_TCHAR(Data->Statistic));
				}
			});
		}
	}
	return FEOK_NotificationId();
}

void UEOK_RTCSubsystem::EOK_RTC_BlockParticipant(FEOK_ProductUserId LocalUserId, const FString& RoomName,
	FEOK_ProductUserId ParticipantId, bool bBlocked, const FEOK_RTC_OnBlockParticipantCallback& Callback)
{
	OnBlockParticipantCallback = Callback;
	if (IOnlineSubsystem* OnlineSub = IOnlineSubsystem::Get("EOK"))
	{
		if (FOnlineSubsystemEOS* EOSRef = static_cast<FOnlineSubsystemEOS*>(OnlineSub))
		{
			EOS_RTC_BlockParticipantOptions Options = { };
			Options.ApiVersion = EOS_RTC_BLOCKPARTICIPANT_API_LATEST;
			Options.LocalUserId = LocalUserId.GetValueAsEosType();
			Options.RoomName = TCHAR_TO_ANSI(*RoomName);
			Options.ParticipantId = ParticipantId.GetValueAsEosType();
			Options.bBlocked = bBlocked;
			EOS_RTC_BlockParticipant(EOS_Platform_GetRTCInterface(EOSRef->EOSPlatformHandle->PlatformHandle), &Options, this, [](const EOS_RTC_BlockParticipantCallbackInfo* Data)
			{
				UEOK_RTCSubsystem* Subsystem = static_cast<UEOK_RTCSubsystem*>(Data->ClientData);
				if (Subsystem)
				{
					Subsystem->OnBlockParticipantCallback.ExecuteIfBound(static_cast<EEOK_Result>(Data->ResultCode), Data->LocalUserId, UTF8_TO_TCHAR(Data->RoomName), Data->ParticipantId, Data->bBlocked ? true : false);
				}
			});
		}
	}
}

void UEOK_RTCSubsystem::EOK_RTC_JoinRoom(FEOK_ProductUserId LocalUserId, const FString& RoomName,
	const FString& ClientBaseUrl, const FString& ParticipantToken, FEOK_ProductUserId ParticipantId,
	bool bEnabledEcho, bool bManualAudioInputEnabled, bool bManualAudioOutputEnabled,
	const FEOK_RTC_OnJoinRoomCallback& Callback)
{
	OnJoinRoomCallback = Callback;
	if (IOnlineSubsystem* OnlineSub = IOnlineSubsystem::Get("EOK"))
	{
		if (FOnlineSubsystemEOS* EOSRef = static_cast<FOnlineSubsystemEOS*>(OnlineSub))
		{
			EOS_RTC_JoinRoomOptions Options = { };
			Options.ApiVersion = EOS_RTC_JOINROOM_API_LATEST;
			Options.LocalUserId = LocalUserId.GetValueAsEosType();
			Options.RoomName = TCHAR_TO_ANSI(*RoomName);
			Options.ClientBaseUrl = TCHAR_TO_ANSI(*ClientBaseUrl);
			Options.ParticipantToken = TCHAR_TO_ANSI(*ParticipantToken);
			Options.ParticipantId = ParticipantId.GetValueAsEosType();
			Options.Flags = bEnabledEcho ? EOS_RTC_JOINROOMFLAGS_ENABLE_ECHO : 0x0;
			Options.bManualAudioInputEnabled = bManualAudioInputEnabled;
			Options.bManualAudioOutputEnabled = bManualAudioOutputEnabled;
			EOS_RTC_JoinRoom(EOS_Platform_GetRTCInterface(EOSRef->EOSPlatformHandle->PlatformHandle), &Options, this, [](const EOS_RTC_JoinRoomCallbackInfo* Data)
			{
				UEOK_RTCSubsystem* Subsystem = static_cast<UEOK_RTCSubsystem*>(Data->ClientData);
				if (Subsystem)
				{
					TArray<FEOK_RTC_Option> Options;
					for (uint32_t i = 0; i < Data->RoomOptionsCount; i++)
					{
						Options.Add(Data->RoomOptions[i]);
					}
					Subsystem->OnJoinRoomCallback.ExecuteIfBound(static_cast<EEOK_Result>(Data->ResultCode), Data->LocalUserId, UTF8_TO_TCHAR(Data->RoomName), Options);
				}
			});
		}
	}
}

void UEOK_RTCSubsystem::EOK_RTC_LeaveRoom(FEOK_ProductUserId LocalUserId, const FString& RoomName,
	const FEOK_RTC_OnLeaveRoomCallback& Callback)
{
	OnLeaveRoomCallback = Callback;
	if (IOnlineSubsystem* OnlineSub = IOnlineSubsystem::Get("EOK"))
	{
		if (FOnlineSubsystemEOS* EOSRef = static_cast<FOnlineSubsystemEOS*>(OnlineSub))
		{
			EOS_RTC_LeaveRoomOptions Options = { };
			Options.ApiVersion = EOS_RTC_LEAVEROOM_API_LATEST;
			Options.LocalUserId = LocalUserId.GetValueAsEosType();
			Options.RoomName = TCHAR_TO_ANSI(*RoomName);
			EOS_RTC_LeaveRoom(EOS_Platform_GetRTCInterface(EOSRef->EOSPlatformHandle->PlatformHandle), &Options, this, [](const EOS_RTC_LeaveRoomCallbackInfo* Data)
			{
				UEOK_RTCSubsystem* Subsystem = static_cast<UEOK_RTCSubsystem*>(Data->ClientData);
				if (Subsystem)
				{
					Subsystem->OnLeaveRoomCallback.ExecuteIfBound(static_cast<EEOK_Result>(Data->ResultCode), Data->LocalUserId, UTF8_TO_TCHAR(Data->RoomName));
				}
			});
		}
	}
}

void UEOK_RTCSubsystem::EOK_RTC_RemoveNotifyDisconnected(FEOK_NotificationId NotificationId)
{
	if (IOnlineSubsystem* OnlineSub = IOnlineSubsystem::Get("EOK"))
	{
		if (FOnlineSubsystemEOS* EOSRef = static_cast<FOnlineSubsystemEOS*>(OnlineSub))
		{
			EOS_RTC_RemoveNotifyDisconnected(EOS_Platform_GetRTCInterface(EOSRef->EOSPlatformHandle->PlatformHandle), NotificationId.GetValueAsEosType());
		}	
	}
}

void UEOK_RTCSubsystem::EOK_RTC_RemoveNotifyParticipantStatusChanged(FEOK_NotificationId NotificationId)
{
	if (IOnlineSubsystem* OnlineSub = IOnlineSubsystem::Get("EOK"))
	{
		if (FOnlineSubsystemEOS* EOSRef = static_cast<FOnlineSubsystemEOS*>(OnlineSub))
		{
			EOS_RTC_RemoveNotifyParticipantStatusChanged(EOS_Platform_GetRTCInterface(EOSRef->EOSPlatformHandle->PlatformHandle), NotificationId.GetValueAsEosType());
		}
	}
}

void UEOK_RTCSubsystem::EOK_RTC_RemoveNotifyRoomStatisticsUpdated(FEOK_NotificationId NotificationId)
{
	if (IOnlineSubsystem* OnlineSub = IOnlineSubsystem::Get("EOK"))
	{
		if (FOnlineSubsystemEOS* EOSRef = static_cast<FOnlineSubsystemEOS*>(OnlineSub))
		{
			EOS_RTC_RemoveNotifyRoomStatisticsUpdated(EOS_Platform_GetRTCInterface(EOSRef->EOSPlatformHandle->PlatformHandle), NotificationId.GetValueAsEosType());
		}
	}
}

TEnumAsByte<EEOK_Result> UEOK_RTCSubsystem::EOK_RTC_SetRoomSetting(FEOK_ProductUserId LocalUserId,
	const FString& RoomName, const FString& SettingName, const FString& SettingValue)
{
	if (IOnlineSubsystem* OnlineSub = IOnlineSubsystem::Get("EOK"))
	{
		if (FOnlineSubsystemEOS* EOSRef = static_cast<FOnlineSubsystemEOS*>(OnlineSub))
		{
			EOS_RTC_SetRoomSettingOptions Options = { };
			Options.ApiVersion = EOS_RTC_SETROOMSETTING_API_LATEST;
			Options.LocalUserId = LocalUserId.GetValueAsEosType();
			Options.RoomName = TCHAR_TO_ANSI(*RoomName);
			Options.SettingName = TCHAR_TO_ANSI(*SettingName);
			Options.SettingValue = TCHAR_TO_ANSI(*SettingValue);
			return static_cast<EEOK_Result>(EOS_RTC_SetRoomSetting(EOS_Platform_GetRTCInterface(EOSRef->EOSPlatformHandle->PlatformHandle), &Options));
		}
	}
	UE_LOG(LogEOK, Error, TEXT("EOK_RTCSubsystem::EOK_RTC_SetRoomSetting: OnlineSubsystemEOK is not available"));
	return EEOK_Result::EOS_NotFound;
}

TEnumAsByte<EEOK_Result> UEOK_RTCSubsystem::EOK_RTC_SetSetting(const FString& SettingName, const FString& SettingValue)
{
	if (IOnlineSubsystem* OnlineSub = IOnlineSubsystem::Get("EOK"))
	{
		if (FOnlineSubsystemEOS* EOSRef = static_cast<FOnlineSubsystemEOS*>(OnlineSub))
		{
			EOS_RTC_SetSettingOptions Options = { };
			Options.ApiVersion = EOS_RTC_SETSETTING_API_LATEST;
			Options.SettingName = TCHAR_TO_ANSI(*SettingName);
			Options.SettingValue = TCHAR_TO_ANSI(*SettingValue);
			return static_cast<EEOK_Result>(EOS_RTC_SetSetting(EOS_Platform_GetRTCInterface(EOSRef->EOSPlatformHandle->PlatformHandle), &Options));
		}
	}
	UE_LOG(LogEOK, Error, TEXT("EOK_RTCSubsystem::EOK_RTC_SetSetting: OnlineSubsystemEOK is not available"));
	return EEOK_Result::EOS_NotFound;
}

TEnumAsByte<EEOK_Result> UEOK_RTCSubsystem::EOK_RTCAdmin_CopyUserTokenByIndex(int32 UserTokenIndex, int32 QueryId,
	FEOK_RTCAdmin_UserToken& OutUserToken)
{
	if (IOnlineSubsystem* OnlineSub = IOnlineSubsystem::Get("EOK"))
	{
		if (FOnlineSubsystemEOS* EOSRef = static_cast<FOnlineSubsystemEOS*>(OnlineSub))
		{
			EOS_RTCAdmin_CopyUserTokenByIndexOptions Options = { };
			Options.ApiVersion = EOS_RTCADMIN_COPYUSERTOKENBYINDEX_API_LATEST;
			Options.UserTokenIndex = UserTokenIndex;
			Options.QueryId = QueryId;
			EOS_RTCAdmin_UserToken* OutUserTokenBasic;
			TEnumAsByte<EEOK_Result> Result = static_cast<EEOK_Result>(EOS_RTCAdmin_CopyUserTokenByIndex(EOS_Platform_GetRTCAdminInterface(EOSRef->EOSPlatformHandle->PlatformHandle), &Options, &OutUserTokenBasic));
			OutUserToken = *OutUserTokenBasic;
			return Result;
		}
	}
	UE_LOG(LogEOK, Error, TEXT("EOK_RTCSubsystem::EOK_RTCAdmin_CopyUserTokenByIndex: OnlineSubsystemEOK is not available"));
	return EEOK_Result::EOS_NotFound;
}

TEnumAsByte<EEOK_Result> UEOK_RTCSubsystem::EOK_RTCAdmin_CopyUserTokenByUserId(FEOK_ProductUserId UserId,
	int32 QueryId, FEOK_RTCAdmin_UserToken& OutUserToken)
{
	if (IOnlineSubsystem* OnlineSub = IOnlineSubsystem::Get("EOK"))
	{
		if (FOnlineSubsystemEOS* EOSRef = static_cast<FOnlineSubsystemEOS*>(OnlineSub))
		{
			EOS_RTCAdmin_CopyUserTokenByUserIdOptions Options = { };
			Options.ApiVersion = EOS_RTCADMIN_COPYUSERTOKENBYUSERID_API_LATEST;
			Options.TargetUserId = UserId.GetValueAsEosType();
			Options.QueryId = QueryId;
			EOS_RTCAdmin_UserToken* OutUserTokenBasic;
			TEnumAsByte<EEOK_Result> Result = static_cast<EEOK_Result>(EOS_RTCAdmin_CopyUserTokenByUserId(EOS_Platform_GetRTCAdminInterface(EOSRef->EOSPlatformHandle->PlatformHandle), &Options, &OutUserTokenBasic));
			OutUserToken = *OutUserTokenBasic;
			return Result;
		}
	}
	UE_LOG(LogEOK, Error, TEXT("EOK_RTCSubsystem::EOK_RTCAdmin_CopyUserTokenByUserId: OnlineSubsystemEOK is not available"));
	return EEOK_Result::EOS_NotFound;
}

void UEOK_RTCSubsystem::EOK_RTCAdmin_Kick(const FString& RoomName, FEOK_ProductUserId TargetUserId,
	const FEOK_RTCAdmin_OnKickCompleteCallback& Callback)
{
	OnKickCompleteCallback = Callback;
	if (IOnlineSubsystem* OnlineSub = IOnlineSubsystem::Get("EOK"))
	{
		if (FOnlineSubsystemEOS* EOSRef = static_cast<FOnlineSubsystemEOS*>(OnlineSub))
		{
			EOS_RTCAdmin_KickOptions Options = { };
			Options.ApiVersion = EOS_RTCADMIN_KICK_API_LATEST;
			Options.RoomName = TCHAR_TO_ANSI(*RoomName);
			Options.TargetUserId = TargetUserId.GetValueAsEosType();
			EOS_RTCAdmin_Kick(EOS_Platform_GetRTCAdminInterface(EOSRef->EOSPlatformHandle->PlatformHandle), &Options, this, [](const EOS_RTCAdmin_KickCompleteCallbackInfo* Data)
			{
				UEOK_RTCSubsystem* Subsystem = static_cast<UEOK_RTCSubsystem*>(Data->ClientData);
				if (Subsystem)
				{
					Subsystem->OnKickCompleteCallback.ExecuteIfBound(static_cast<EEOK_Result>(Data->ResultCode));
				}
			});
		}
	}
}

TEnumAsByte<EEOK_Result> UEOK_RTCSubsystem::EOK_RTCAdmin_QueryJoinRoomToken(const FString& RoomName,
	FEOK_ProductUserId LocalUserId, const TArray<FEOK_ProductUserId>& TargetUserIds,
	TArray<FString>& TargetUserIpAddresses, const FEOK_RTCAdmin_OnQueryJoinRoomTokenCompleteCallback& Callback)
{
	OnQueryJoinRoomTokenCompleteCallback = Callback;
	if (IOnlineSubsystem* OnlineSub = IOnlineSubsystem::Get("EOK"))
	{
		if (FOnlineSubsystemEOS* EOSRef = static_cast<FOnlineSubsystemEOS*>(OnlineSub))
		{
			TArray<EOS_ProductUserId> TargetUserIdsBasic;
			for (FEOK_ProductUserId TargetUserId : TargetUserIds)
			{
				TargetUserIdsBasic.Add(TargetUserId.GetValueAsEosType());
			}
			EOS_RTCAdmin_QueryJoinRoomTokenOptions Options = { };
			Options.ApiVersion = EOS_RTCADMIN_QUERYJOINROOMTOKEN_API_LATEST;
			Options.RoomName = TCHAR_TO_ANSI(*RoomName);
			Options.LocalUserId = LocalUserId.GetValueAsEosType();
			Options.TargetUserIdsCount = TargetUserIdsBasic.Num();
			Options.TargetUserIds = TargetUserIdsBasic.GetData();
			EOS_RTCAdmin_QueryJoinRoomToken(EOS_Platform_GetRTCAdminInterface(EOSRef->EOSPlatformHandle->PlatformHandle), &Options, this,[](const EOS_RTCAdmin_QueryJoinRoomTokenCompleteCallbackInfo* Data)
			{
				if(UEOK_RTCSubsystem* Subsystem = static_cast<UEOK_RTCSubsystem*>(Data->ClientData))
				{
					TArray<FString> TargetUserIpAddresses;
					//Subsystem->OnQueryJoinRoomTokenCompleteCallback.ExecuteIfBound(static_cast<EEOK_Result>(Data->ResultCode), UTF8_TO_TCHAR(Data->RoomName), Data->LocalUserId, TargetUserIpAddresses);
				}
			});
		}
	}
	UE_LOG(LogEOK, Error, TEXT("EOK_RTCSubsystem::EOK_RTCAdmin_QueryJoinRoomToken: OnlineSubsystemEOK is not available"));
	return EEOK_Result::EOS_NotFound;
}

FEOK_NotificationId UEOK_RTCSubsystem::EOK_RTCAudio_AddNotifyAudioBeforeRender(FEOK_ProductUserId LocalUserId,
	const FString& RoomName, bool bUnmixedAudio, const FEOK_RTCAudio_OnAudioBeforeRenderCallback& Callback)
{
	OnAudioBeforeRenderCallback = Callback;
	if (IOnlineSubsystem* OnlineSub = IOnlineSubsystem::Get("EOK"))
	{
		if (FOnlineSubsystemEOS* EOSRef = static_cast<FOnlineSubsystemEOS*>(OnlineSub))
		{
			EOS_RTCAudio_AddNotifyAudioBeforeRenderOptions Options = { };
			Options.ApiVersion = EOS_RTCAUDIO_ADDNOTIFYAUDIOBEFORERENDER_API_LATEST;
			Options.LocalUserId = LocalUserId.GetValueAsEosType();
			Options.RoomName = TCHAR_TO_ANSI(*RoomName);
			Options.bUnmixedAudio = bUnmixedAudio;
			return EOS_RTCAudio_AddNotifyAudioBeforeRender(EOS_RTC_GetAudioInterface(EOS_Platform_GetRTCInterface(EOSRef->EOSPlatformHandle->PlatformHandle)), &Options, this, [](const EOS_RTCAudio_AudioBeforeRenderCallbackInfo* Data)
			{
				UEOK_RTCSubsystem* Subsystem = static_cast<UEOK_RTCSubsystem*>(Data->ClientData);
				if (Subsystem)
				{
					Subsystem->OnAudioBeforeRenderCallback.ExecuteIfBound(Data->LocalUserId, UTF8_TO_TCHAR(Data->RoomName), Data->Buffer, Data->ParticipantId);
				}
			});
		}
	}
	return FEOK_NotificationId();
}

FEOK_NotificationId UEOK_RTCSubsystem::EOK_RTCAudio_AddNotifyAudioDevicesChanged(
	const FEOK_RTCAudio_OnAudioDevicesChangedCallback& Callback)
{
	OnAudioDevicesChangedCallback = Callback;
	if (IOnlineSubsystem* OnlineSub = IOnlineSubsystem::Get("EOK"))
	{
		if (FOnlineSubsystemEOS* EOSRef = static_cast<FOnlineSubsystemEOS*>(OnlineSub))
		{
			EOS_RTCAudio_AddNotifyAudioDevicesChangedOptions Options = { };
			Options.ApiVersion = EOS_RTCAUDIO_ADDNOTIFYAUDIODEVICESCHANGED_API_LATEST;
			return EOS_RTCAudio_AddNotifyAudioDevicesChanged(EOS_RTC_GetAudioInterface(EOS_Platform_GetRTCInterface(EOSRef->EOSPlatformHandle->PlatformHandle)), &Options, this, [](const EOS_RTCAudio_AudioDevicesChangedCallbackInfo* Data)
			{
				UEOK_RTCSubsystem* Subsystem = static_cast<UEOK_RTCSubsystem*>(Data->ClientData);
				if (Subsystem)
				{
					Subsystem->OnAudioDevicesChangedCallback.ExecuteIfBound();
				}
			});
		}
	}
	return FEOK_NotificationId();
}

FEOK_NotificationId UEOK_RTCSubsystem::EOK_RTCAudio_AddNotifyAudioInputState(FEOK_ProductUserId LocalUserId,
	const FString& RoomName, const FEOK_RTCAudio_OnAudioInputStateCallback& Callback)
{
	OnAudioInputStateCallback = Callback;
	if (IOnlineSubsystem* OnlineSub = IOnlineSubsystem::Get("EOK"))
	{
		if (FOnlineSubsystemEOS* EOSRef = static_cast<FOnlineSubsystemEOS*>(OnlineSub))
		{
			EOS_RTCAudio_AddNotifyAudioInputStateOptions Options = { };
			Options.ApiVersion = EOS_RTCAUDIO_ADDNOTIFYAUDIOINPUTSTATE_API_LATEST;
			Options.LocalUserId = LocalUserId.GetValueAsEosType();
			Options.RoomName = TCHAR_TO_ANSI(*RoomName);
			return EOS_RTCAudio_AddNotifyAudioInputState(EOS_RTC_GetAudioInterface(EOS_Platform_GetRTCInterface(EOSRef->EOSPlatformHandle->PlatformHandle)), &Options, this, [](const EOS_RTCAudio_AudioInputStateCallbackInfo* Data)
			{
				UEOK_RTCSubsystem* Subsystem = static_cast<UEOK_RTCSubsystem*>(Data->ClientData);
				if (Subsystem)
				{
					Subsystem->OnAudioInputStateCallback.ExecuteIfBound(Data->LocalUserId, UTF8_TO_TCHAR(Data->RoomName), static_cast<EEOK_ERTCAudioInputStatus>(Data->Status));
				}
			});
		}
	}
	return FEOK_NotificationId();
}

FEOK_NotificationId UEOK_RTCSubsystem::EOK_RTCAudio_AddNotifyAudioOutputState(FEOK_ProductUserId LocalUserId,
	const FString& RoomName, const FEOK_RTCAudio_OnAudioOutputStateCallback& Callback)
{
	OnAudioOutputStateCallback = Callback;
	if (IOnlineSubsystem* OnlineSub = IOnlineSubsystem::Get("EOK"))
	{
		if (FOnlineSubsystemEOS* EOSRef = static_cast<FOnlineSubsystemEOS*>(OnlineSub))
		{
			EOS_RTCAudio_AddNotifyAudioOutputStateOptions Options = { };
			Options.ApiVersion = EOS_RTCAUDIO_ADDNOTIFYAUDIOOUTPUTSTATE_API_LATEST;
			Options.LocalUserId = LocalUserId.GetValueAsEosType();
			Options.RoomName = TCHAR_TO_ANSI(*RoomName);
			return EOS_RTCAudio_AddNotifyAudioOutputState(EOS_RTC_GetAudioInterface(EOS_Platform_GetRTCInterface(EOSRef->EOSPlatformHandle->PlatformHandle)), &Options, this, [](const EOS_RTCAudio_AudioOutputStateCallbackInfo* Data)
			{
				UEOK_RTCSubsystem* Subsystem = static_cast<UEOK_RTCSubsystem*>(Data->ClientData);
				if (Subsystem)
				{
					Subsystem->OnAudioOutputStateCallback.ExecuteIfBound(Data->LocalUserId, UTF8_TO_TCHAR(Data->RoomName), static_cast<EEOK_ERTCAudioOutputStatus>(Data->Status));
				}
			});
		}
	}
	return FEOK_NotificationId();
}

FEOK_NotificationId UEOK_RTCSubsystem::EOK_RTCAudio_AddNotifyParticipantUpdated(FEOK_ProductUserId LocalUserId,
	const FString& RoomName, const FEOK_RTCAudio_OnParticipantUpdatedCallback& Callback)
{
	OnParticipantUpdatedCallback = Callback;
	if (IOnlineSubsystem* OnlineSub = IOnlineSubsystem::Get("EOK"))
	{
		if (FOnlineSubsystemEOS* EOSRef = static_cast<FOnlineSubsystemEOS*>(OnlineSub))
		{
			EOS_RTCAudio_AddNotifyParticipantUpdatedOptions Options = { };
			Options.ApiVersion = EOS_RTCAUDIO_ADDNOTIFYPARTICIPANTUPDATED_API_LATEST;
			Options.LocalUserId = LocalUserId.GetValueAsEosType();
			Options.RoomName = TCHAR_TO_ANSI(*RoomName);
			return EOS_RTCAudio_AddNotifyParticipantUpdated(EOS_RTC_GetAudioInterface(EOS_Platform_GetRTCInterface(EOSRef->EOSPlatformHandle->PlatformHandle)), &Options, this, [](const EOS_RTCAudio_ParticipantUpdatedCallbackInfo* Data)
			{
				UEOK_RTCSubsystem* Subsystem = static_cast<UEOK_RTCSubsystem*>(Data->ClientData);
				if (Subsystem)
				{
					Subsystem->OnParticipantUpdatedCallback.ExecuteIfBound(Data->LocalUserId, UTF8_TO_TCHAR(Data->RoomName), Data->ParticipantId,Data->bSpeaking ? true : false, static_cast<EEOK_ERTCAudioStatus>(Data->AudioStatus));
				}
			});
		}
	}
	return FEOK_NotificationId();
}

TEnumAsByte<EEOK_Result> UEOK_RTCSubsystem::EOK_RTCAudio_CopyInputDeviceInformationByIndex(int32 DeviceIndex,
	FEOK_RTCAudio_InputDeviceInformation& OutDeviceInfo)
{
	if (IOnlineSubsystem* OnlineSub = IOnlineSubsystem::Get("EOK"))
	{
		if (FOnlineSubsystemEOS* EOSRef = static_cast<FOnlineSubsystemEOS*>(OnlineSub))
		{
			EOS_RTCAudio_CopyInputDeviceInformationByIndexOptions Options = { };
			Options.ApiVersion = EOS_RTCAUDIO_COPYINPUTDEVICEINFORMATIONBYINDEX_API_LATEST;
			Options.DeviceIndex = DeviceIndex;
			EOS_RTCAudio_InputDeviceInformation* OutDeviceInfoBasic;
			TEnumAsByte<EEOK_Result> Result = static_cast<EEOK_Result>(EOS_RTCAudio_CopyInputDeviceInformationByIndex(EOS_RTC_GetAudioInterface(EOS_Platform_GetRTCInterface(EOSRef->EOSPlatformHandle->PlatformHandle)), &Options, &OutDeviceInfoBasic));
			OutDeviceInfo = *OutDeviceInfoBasic;
			return Result;
		}
	}
	UE_LOG(LogEOK, Error, TEXT("EOK_RTCSubsystem::EOK_RTCAudio_CopyInputDeviceInformationByIndex: OnlineSubsystemEOK is not available"));
	return EEOK_Result::EOS_NotFound;
}

TEnumAsByte<EEOK_Result> UEOK_RTCSubsystem::EOK_RTCAudio_CopyOutputDeviceInformationByIndex(int32 DeviceIndex,
	FEOK_RTCAudio_OutputDeviceInformation& OutDeviceInfo)
{
	if (IOnlineSubsystem* OnlineSub = IOnlineSubsystem::Get("EOK"))
	{
		if (FOnlineSubsystemEOS* EOSRef = static_cast<FOnlineSubsystemEOS*>(OnlineSub))
		{
			EOS_RTCAudio_CopyOutputDeviceInformationByIndexOptions Options = { };
			Options.ApiVersion = EOS_RTCAUDIO_COPYOUTPUTDEVICEINFORMATIONBYINDEX_API_LATEST;
			Options.DeviceIndex = DeviceIndex;
			EOS_RTCAudio_OutputDeviceInformation* OutDeviceInfoBasic;
			TEnumAsByte<EEOK_Result> Result = static_cast<EEOK_Result>(EOS_RTCAudio_CopyOutputDeviceInformationByIndex(EOS_RTC_GetAudioInterface(EOS_Platform_GetRTCInterface(EOSRef->EOSPlatformHandle->PlatformHandle)), &Options, &OutDeviceInfoBasic));
			OutDeviceInfo = *OutDeviceInfoBasic;
			return Result;
		}
	}
	UE_LOG(LogEOK, Error, TEXT("EOK_RTCSubsystem::EOK_RTCAudio_CopyOutputDeviceInformationByIndex: OnlineSubsystemEOK is not available"));
	return EEOK_Result::EOS_NotFound;
}

int32 UEOK_RTCSubsystem::EOK_RTCAudio_GetInputDevicesCount()
{
	if (IOnlineSubsystem* OnlineSub = IOnlineSubsystem::Get("EOK"))
	{
		if (FOnlineSubsystemEOS* EOSRef = static_cast<FOnlineSubsystemEOS*>(OnlineSub))
		{
			EOS_RTCAudio_GetInputDevicesCountOptions Option = { };
			Option.ApiVersion = EOS_RTCAUDIO_GETINPUTDEVICESCOUNT_API_LATEST;
			return EOS_RTCAudio_GetInputDevicesCount(EOS_RTC_GetAudioInterface(EOS_Platform_GetRTCInterface(EOSRef->EOSPlatformHandle->PlatformHandle)), &Option);
		}
	}
	UE_LOG(LogEOK, Error, TEXT("EOK_RTCSubsystem::EOK_RTCAudio_GetInputDevicesCount: OnlineSubsystemEOK is not available"));
	return 0;
}

int32 UEOK_RTCSubsystem::EOK_RTCAudio_GetOutputDevicesCount()
{
	if (IOnlineSubsystem* OnlineSub = IOnlineSubsystem::Get("EOK"))
	{
		if (FOnlineSubsystemEOS* EOSRef = static_cast<FOnlineSubsystemEOS*>(OnlineSub))
		{
			EOS_RTCAudio_GetOutputDevicesCountOptions Option = { };
			Option.ApiVersion = EOS_RTCAUDIO_GETOUTPUTDEVICESCOUNT_API_LATEST;
			return EOS_RTCAudio_GetOutputDevicesCount(EOS_RTC_GetAudioInterface(EOS_Platform_GetRTCInterface(EOSRef->EOSPlatformHandle->PlatformHandle)), &Option);
		}
	}
	UE_LOG(LogEOK, Error, TEXT("EOK_RTCSubsystem::EOK_RTCAudio_GetOutputDevicesCount: OnlineSubsystemEOK is not available"));
	return 0;
}

void UEOK_RTCSubsystem::EOK_RTCAudio_InputDeviceInformation_Release(FEOK_RTCAudio_InputDeviceInformation& DeviceInfo)
{
	if (IOnlineSubsystem* OnlineSub = IOnlineSubsystem::Get("EOK"))
	{
		if (FOnlineSubsystemEOS* EOSRef = static_cast<FOnlineSubsystemEOS*>(OnlineSub))
		{
			EOS_RTCAudio_InputDeviceInformation ReleaseOptions = DeviceInfo.Ref;
			EOS_RTCAudio_InputDeviceInformation_Release(&ReleaseOptions);
		}
	}
}

void UEOK_RTCSubsystem::EOK_RTCAudio_OutputDeviceInformation_Release(FEOK_RTCAudio_OutputDeviceInformation& DeviceInfo)
{
	if (IOnlineSubsystem* OnlineSub = IOnlineSubsystem::Get("EOK"))
	{
		if (FOnlineSubsystemEOS* EOSRef = static_cast<FOnlineSubsystemEOS*>(OnlineSub))
		{
			EOS_RTCAudio_OutputDeviceInformation ReleaseOptions = DeviceInfo.Ref;
			EOS_RTCAudio_OutputDeviceInformation_Release(&ReleaseOptions);
		}
	}
}

void UEOK_RTCSubsystem::EOK_RTCAudio_QueryInputDevicesInformation(
	const FEOK_RTCAudio_OnQueryInputDevicesInformationCallback& Callback)
{
	if (IOnlineSubsystem* OnlineSub = IOnlineSubsystem::Get("EOK"))
	{
		if (FOnlineSubsystemEOS* EOSRef = static_cast<FOnlineSubsystemEOS*>(OnlineSub))
		{
			EOS_RTCAudio_QueryInputDevicesInformationOptions Options = { };
			Options.ApiVersion = EOS_RTCAUDIO_QUERYINPUTDEVICESINFORMATION_API_LATEST;
			EOS_RTCAudio_QueryInputDevicesInformation(EOS_RTC_GetAudioInterface(EOS_Platform_GetRTCInterface(EOSRef->EOSPlatformHandle->PlatformHandle)), &Options, this, [](const EOS_RTCAudio_OnQueryInputDevicesInformationCallbackInfo* Data)
			{
				UEOK_RTCSubsystem* Subsystem = static_cast<UEOK_RTCSubsystem*>(Data->ClientData);
				if (Subsystem)
				{
					Subsystem->OnQueryInputDevicesInformationCallback.ExecuteIfBound(static_cast<EEOK_Result>(Data->ResultCode));
				}
			});
		}
	}
}

void UEOK_RTCSubsystem::EOK_RTCAudio_QueryOutputDevicesInformation(
	const FEOK_RTCAudio_OnQueryOutputDevicesInformationCallback& Callback)
{
	OnQueryOutputDevicesInformationCallback = Callback;
	if (IOnlineSubsystem* OnlineSub = IOnlineSubsystem::Get("EOK"))
	{
		if (FOnlineSubsystemEOS* EOSRef = static_cast<FOnlineSubsystemEOS*>(OnlineSub))
		{
			EOS_RTCAudio_QueryOutputDevicesInformationOptions Options = { };
			Options.ApiVersion = EOS_RTCAUDIO_QUERYOUTPUTDEVICESINFORMATION_API_LATEST;
			EOS_RTCAudio_QueryOutputDevicesInformation(EOS_RTC_GetAudioInterface(EOS_Platform_GetRTCInterface(EOSRef->EOSPlatformHandle->PlatformHandle)), &Options, this, [](const EOS_RTCAudio_OnQueryOutputDevicesInformationCallbackInfo* Data)
			{
				UEOK_RTCSubsystem* Subsystem = static_cast<UEOK_RTCSubsystem*>(Data->ClientData);
				if (Subsystem)
				{
					Subsystem->OnQueryOutputDevicesInformationCallback.ExecuteIfBound(static_cast<EEOK_Result>(Data->ResultCode));
				}
			});
		}
	}
}

void UEOK_RTCSubsystem::EOK_RTCAudio_RegisterPlatformUser(const FString& PlatformUserId,
	const FEOK_RTCAudio_OnRegisterPlatformUserCallback& Callback)
{
	OnRegisterPlatformUserCallback = Callback;
	if (IOnlineSubsystem* OnlineSub = IOnlineSubsystem::Get("EOK"))
	{
		if (FOnlineSubsystemEOS* EOSRef = static_cast<FOnlineSubsystemEOS*>(OnlineSub))
		{
			EOS_RTCAudio_RegisterPlatformUserOptions Options = { };
			Options.ApiVersion = EOS_RTCAUDIO_REGISTERPLATFORMUSER_API_LATEST;
			Options.PlatformUserId = TCHAR_TO_ANSI(*PlatformUserId);
			EOS_RTCAudio_RegisterPlatformUser(EOS_RTC_GetAudioInterface(EOS_Platform_GetRTCInterface(EOSRef->EOSPlatformHandle->PlatformHandle)), &Options, this, [](const EOS_RTCAudio_OnRegisterPlatformUserCallbackInfo* Data)
			{
				UEOK_RTCSubsystem* Subsystem = static_cast<UEOK_RTCSubsystem*>(Data->ClientData);
				if (Subsystem)
				{
					Subsystem->OnRegisterPlatformUserCallback.ExecuteIfBound(static_cast<EEOK_Result>(Data->ResultCode), UTF8_TO_TCHAR(Data->PlatformUserId));
				}
			});
		}
	}
}

void UEOK_RTCSubsystem::EOK_RTCAudio_RemoveNotifyAudioBeforeRender(FEOK_NotificationId NotificationId)
{
	if (IOnlineSubsystem* OnlineSub = IOnlineSubsystem::Get("EOK"))
	{
		if (FOnlineSubsystemEOS* EOSRef = static_cast<FOnlineSubsystemEOS*>(OnlineSub))
		{
			EOS_RTCAudio_RemoveNotifyAudioBeforeRender(EOS_RTC_GetAudioInterface(EOS_Platform_GetRTCInterface(EOSRef->EOSPlatformHandle->PlatformHandle)), NotificationId.GetValueAsEosType());
		}
	}
}

void UEOK_RTCSubsystem::EOK_RTCAudio_RemoveNotifyAudioBeforeSend(FEOK_NotificationId NotificationId)
{
	if (IOnlineSubsystem* OnlineSub = IOnlineSubsystem::Get("EOK"))
	{
		if (FOnlineSubsystemEOS* EOSRef = static_cast<FOnlineSubsystemEOS*>(OnlineSub))
		{
			EOS_RTCAudio_RemoveNotifyAudioBeforeSend(EOS_RTC_GetAudioInterface(EOS_Platform_GetRTCInterface(EOSRef->EOSPlatformHandle->PlatformHandle)), NotificationId.GetValueAsEosType());
		}
	}
}

void UEOK_RTCSubsystem::EOK_RTCAudio_RemoveNotifyAudioDevicesChanged(FEOK_NotificationId NotificationId)
{
	if (IOnlineSubsystem* OnlineSub = IOnlineSubsystem::Get("EOK"))
	{
		if (FOnlineSubsystemEOS* EOSRef = static_cast<FOnlineSubsystemEOS*>(OnlineSub))
		{
			EOS_RTCAudio_RemoveNotifyAudioDevicesChanged(EOS_RTC_GetAudioInterface(EOS_Platform_GetRTCInterface(EOSRef->EOSPlatformHandle->PlatformHandle)), NotificationId.GetValueAsEosType());
		}
	}
}

void UEOK_RTCSubsystem::EOK_RTCAudio_RemoveNotifyAudioInputState(FEOK_NotificationId NotificationId)
{
	if (IOnlineSubsystem* OnlineSub = IOnlineSubsystem::Get("EOK"))
	{
		if (FOnlineSubsystemEOS* EOSRef = static_cast<FOnlineSubsystemEOS*>(OnlineSub))
		{
			EOS_RTCAudio_RemoveNotifyAudioInputState(EOS_RTC_GetAudioInterface(EOS_Platform_GetRTCInterface(EOSRef->EOSPlatformHandle->PlatformHandle)), NotificationId.GetValueAsEosType());
		}
	}
}

void UEOK_RTCSubsystem::EOK_RTCAudio_RemoveNotifyAudioOutputState(FEOK_NotificationId NotificationId)
{
	if (IOnlineSubsystem* OnlineSub = IOnlineSubsystem::Get("EOK"))
	{
		if (FOnlineSubsystemEOS* EOSRef = static_cast<FOnlineSubsystemEOS*>(OnlineSub))
		{
			EOS_RTCAudio_RemoveNotifyAudioOutputState(EOS_RTC_GetAudioInterface(EOS_Platform_GetRTCInterface(EOSRef->EOSPlatformHandle->PlatformHandle)), NotificationId.GetValueAsEosType());
		}
	}
}

void UEOK_RTCSubsystem::EOK_RTCAudio_RemoveNotifyParticipantUpdated(FEOK_NotificationId NotificationId)
{
	if (IOnlineSubsystem* OnlineSub = IOnlineSubsystem::Get("EOK"))
	{
		if (FOnlineSubsystemEOS* EOSRef = static_cast<FOnlineSubsystemEOS*>(OnlineSub))
		{
			EOS_RTCAudio_RemoveNotifyParticipantUpdated(EOS_RTC_GetAudioInterface(EOS_Platform_GetRTCInterface(EOSRef->EOSPlatformHandle->PlatformHandle)), NotificationId.GetValueAsEosType());
		}
	}
}

TEnumAsByte<EEOK_Result> UEOK_RTCSubsystem::EOK_RTCAudio_SendAudio(FEOK_ProductUserId LocalUserId,
	const FString& RoomName, const FEOK_RTCAudio_AudioBuffer& AudioBuffer)
{
	if (IOnlineSubsystem* OnlineSub = IOnlineSubsystem::Get("EOK"))
	{
		if (FOnlineSubsystemEOS* EOSRef = static_cast<FOnlineSubsystemEOS*>(OnlineSub))
		{	
			EOS_RTCAudio_SendAudioOptions Options = { };
			Options.ApiVersion = EOS_RTCAUDIO_SENDAUDIO_API_LATEST;
			Options.LocalUserId = LocalUserId.GetValueAsEosType();
			Options.RoomName = TCHAR_TO_ANSI(*RoomName);
			EOS_RTCAudio_AudioBuffer OptionsBuffer = AudioBuffer.GetValueAsEosType();
			Options.Buffer = &OptionsBuffer;
			return static_cast<EEOK_Result>(EOS_RTCAudio_SendAudio(EOS_RTC_GetAudioInterface(EOS_Platform_GetRTCInterface(EOSRef->EOSPlatformHandle->PlatformHandle)), &Options));
		}
	}
	UE_LOG(LogEOK, Error, TEXT("EOK_RTCAudioSubsystem::EOK_RTCAudio_SendAudio: OnlineSubsystemEOK is not available"));
	return EEOK_Result::EOS_NotFound;
}

void UEOK_RTCSubsystem::EOK_RTCAudio_SetInputDeviceSettings(FEOK_ProductUserId LocalUserId,
	const FString& RealDeviceId, bool bPlatformAEC, const FEOK_RTCAudio_OnSetInputDeviceSettingsCallback& Callback)
{
	OnSetInputDeviceSettingsCallback = Callback;
	if (IOnlineSubsystem* OnlineSub = IOnlineSubsystem::Get("EOK"))
	{
		if (FOnlineSubsystemEOS* EOSRef = static_cast<FOnlineSubsystemEOS*>(OnlineSub))
		{	
			EOS_RTCAudio_SetInputDeviceSettingsOptions Options = { };
			Options.ApiVersion = EOS_RTCAUDIO_SETINPUTDEVICESETTINGS_API_LATEST;
			Options.LocalUserId = LocalUserId.GetValueAsEosType();
			Options.RealDeviceId = TCHAR_TO_ANSI(*RealDeviceId);
			Options.bPlatformAEC = bPlatformAEC;
			EOS_RTCAudio_SetInputDeviceSettings(EOS_RTC_GetAudioInterface(EOS_Platform_GetRTCInterface(EOSRef->EOSPlatformHandle->PlatformHandle)), &Options, this, [](const EOS_RTCAudio_OnSetInputDeviceSettingsCallbackInfo* Data)
			{
				UEOK_RTCSubsystem* Subsystem = static_cast<UEOK_RTCSubsystem*>(Data->ClientData);
				if (Subsystem)
				{
					Subsystem->OnSetInputDeviceSettingsCallback.ExecuteIfBound(static_cast<EEOK_Result>(Data->ResultCode), UTF8_TO_TCHAR(Data->RealDeviceId));
				}
			});
		}
	}
}

void UEOK_RTCSubsystem::EOK_RTCAudio_SetOutputDeviceSettings(FEOK_ProductUserId LocalUserId,
	const FString& RealDeviceId, const FEOK_RTCAudio_OnSetOutputDeviceSettingsCallback& Callback)
{
	OnSetOutputDeviceSettingsCallback = Callback;
	if (IOnlineSubsystem* OnlineSub = IOnlineSubsystem::Get("EOK"))
	{
		if (FOnlineSubsystemEOS* EOSRef = static_cast<FOnlineSubsystemEOS*>(OnlineSub))
		{	
			EOS_RTCAudio_SetOutputDeviceSettingsOptions Options = { };
			Options.ApiVersion = EOS_RTCAUDIO_SETOUTPUTDEVICESETTINGS_API_LATEST;
			Options.LocalUserId = LocalUserId.GetValueAsEosType();
			Options.RealDeviceId = TCHAR_TO_ANSI(*RealDeviceId);
			EOS_RTCAudio_SetOutputDeviceSettings(EOS_RTC_GetAudioInterface(EOS_Platform_GetRTCInterface(EOSRef->EOSPlatformHandle->PlatformHandle)), &Options, this, [](const EOS_RTCAudio_OnSetOutputDeviceSettingsCallbackInfo* Data)
			{
				UEOK_RTCSubsystem* Subsystem = static_cast<UEOK_RTCSubsystem*>(Data->ClientData);
				if (Subsystem)
				{
					Subsystem->OnSetOutputDeviceSettingsCallback.ExecuteIfBound(static_cast<EEOK_Result>(Data->ResultCode), UTF8_TO_TCHAR(Data->RealDeviceId));
				}
			});
		}
	}
}

void UEOK_RTCSubsystem::EOK_RTCAudio_UnregisterPlatformUser(const FString& PlatformUserId,
	const FEOK_RTCAudio_OnUnregisterPlatformUserCallback& Callback)
{
	OnUnregisterPlatformUserCallback = Callback;
	if (IOnlineSubsystem* OnlineSub = IOnlineSubsystem::Get("EOK"))
	{
		if (FOnlineSubsystemEOS* EOSRef = static_cast<FOnlineSubsystemEOS*>(OnlineSub))
		{	
			EOS_RTCAudio_UnregisterPlatformUserOptions Options = { };
			Options.ApiVersion = EOS_RTCAUDIO_UNREGISTERPLATFORMUSER_API_LATEST;
			Options.PlatformUserId = TCHAR_TO_ANSI(*PlatformUserId);
			EOS_RTCAudio_UnregisterPlatformUser(EOS_RTC_GetAudioInterface(EOS_Platform_GetRTCInterface(EOSRef->EOSPlatformHandle->PlatformHandle)), &Options, this, [](const EOS_RTCAudio_OnUnregisterPlatformUserCallbackInfo* Data)
			{
				UEOK_RTCSubsystem* Subsystem = static_cast<UEOK_RTCSubsystem*>(Data->ClientData);
				if (Subsystem)
				{
					Subsystem->OnUnregisterPlatformUserCallback.ExecuteIfBound(static_cast<EEOK_Result>(Data->ResultCode), UTF8_TO_TCHAR(Data->PlatformUserId));
				}
			});
		}
	}
}

void UEOK_RTCSubsystem::EOK_RTCAudio_UpdateParticipantVolume(FEOK_ProductUserId LocalUserId,
	const FString& RoomName, FEOK_ProductUserId ParticipantId, float Volume,
	const FEOK_RTCAudio_OnUpdateParticipantVolumeCallback& Callback)
{
	OnUpdateParticipantVolumeCallback = Callback;
	if (IOnlineSubsystem* OnlineSub = IOnlineSubsystem::Get("EOK"))
	{
		if (FOnlineSubsystemEOS* EOSRef = static_cast<FOnlineSubsystemEOS*>(OnlineSub))
		{	
			EOS_RTCAudio_UpdateParticipantVolumeOptions Options = { };
			Options.ApiVersion = EOS_RTCAUDIO_UPDATEPARTICIPANTVOLUME_API_LATEST;
			Options.LocalUserId = LocalUserId.GetValueAsEosType();
			Options.RoomName = TCHAR_TO_ANSI(*RoomName);
			Options.ParticipantId = ParticipantId.GetValueAsEosType();
			Options.Volume = Volume;
			EOS_RTCAudio_UpdateParticipantVolume(EOS_RTC_GetAudioInterface(EOS_Platform_GetRTCInterface(EOSRef->EOSPlatformHandle->PlatformHandle)), &Options, this, [](const EOS_RTCAudio_UpdateParticipantVolumeCallbackInfo* Data)
			{
				UEOK_RTCSubsystem* Subsystem = static_cast<UEOK_RTCSubsystem*>(Data->ClientData);
				if (Subsystem)
				{
					Subsystem->OnUpdateParticipantVolumeCallback.ExecuteIfBound(static_cast<EEOK_Result>(Data->ResultCode), Data->ParticipantId);
				}
			});
		}
	}
}
void UEOK_RTCSubsystem::EOK_RTCAudio_UpdateReceiving(FEOK_ProductUserId LocalUserId, const FString& RoomName,
	FEOK_ProductUserId ParticipantId, bool bAudioEnabled,
	const FEOK_RTCAudio_OnUpdateReceivingCallback& Callback)
{
	OnUpdateReceivingCallback = Callback;
	if (IOnlineSubsystem* OnlineSub = IOnlineSubsystem::Get("EOK"))
	{
		if (FOnlineSubsystemEOS* EOSRef = static_cast<FOnlineSubsystemEOS*>(OnlineSub))
		{	
			EOS_RTCAudio_UpdateReceivingOptions Options = { };
			Options.ApiVersion = EOS_RTCAUDIO_UPDATERECEIVING_API_LATEST;
			Options.LocalUserId = LocalUserId.GetValueAsEosType();
			Options.RoomName = TCHAR_TO_ANSI(*RoomName);
			Options.ParticipantId = ParticipantId.GetValueAsEosType();
			Options.bAudioEnabled = bAudioEnabled;
			EOS_RTCAudio_UpdateReceiving(EOS_RTC_GetAudioInterface(EOS_Platform_GetRTCInterface(EOSRef->EOSPlatformHandle->PlatformHandle)), &Options, this, [](const EOS_RTCAudio_UpdateReceivingCallbackInfo* Data)
			{
				UEOK_RTCSubsystem* Subsystem = static_cast<UEOK_RTCSubsystem*>(Data->ClientData);
				if (Subsystem)
				{
					Subsystem->OnUpdateReceivingCallback.ExecuteIfBound(static_cast<EEOK_Result>(Data->ResultCode), Data->ParticipantId);
				}
			});
		}
	
	}
}

void UEOK_RTCSubsystem::EOK_RTCAudio_UpdateReceivingVolume(FEOK_ProductUserId LocalUserId,
	const FString& RoomName, float Volume, const FEOK_RTCAudio_OnUpdateReceivingVolumeCallback& Callback)
{
	OnUpdateReceivingVolumeCallback = Callback;
	if (IOnlineSubsystem* OnlineSub = IOnlineSubsystem::Get("EOK"))
	{
		if (FOnlineSubsystemEOS* EOSRef = static_cast<FOnlineSubsystemEOS*>(OnlineSub))
		{	
			EOS_RTCAudio_UpdateReceivingVolumeOptions Options = { };
			Options.ApiVersion = EOS_RTCAUDIO_UPDATERECEIVINGVOLUME_API_LATEST;
			Options.LocalUserId = LocalUserId.GetValueAsEosType();
			Options.RoomName = TCHAR_TO_ANSI(*RoomName);
			Options.Volume = Volume;
			EOS_RTCAudio_UpdateReceivingVolume(EOS_RTC_GetAudioInterface(EOS_Platform_GetRTCInterface(EOSRef->EOSPlatformHandle->PlatformHandle)), &Options, this, [](const EOS_RTCAudio_UpdateReceivingVolumeCallbackInfo* Data)
			{
				UEOK_RTCSubsystem* Subsystem = static_cast<UEOK_RTCSubsystem*>(Data->ClientData);
				if (Subsystem)
				{
					Subsystem->OnUpdateReceivingVolumeCallback.ExecuteIfBound(static_cast<EEOK_Result>(Data->ResultCode), Data->LocalUserId);
				}
			});
		}
	}
}

void UEOK_RTCSubsystem::EOK_RTCAudio_UpdateSending(FEOK_ProductUserId LocalUserId, const FString& RoomName,
	TEnumAsByte<EEOK_ERTCAudioStatus> AudioStatus, const FEOK_RTCAudio_OnUpdateSendingCallback& Callback)
{
	OnUpdateSendingCallback = Callback;
	if (IOnlineSubsystem* OnlineSub = IOnlineSubsystem::Get("EOK"))
	{
		if (FOnlineSubsystemEOS* EOSRef = static_cast<FOnlineSubsystemEOS*>(OnlineSub))
		{	
			EOS_RTCAudio_UpdateSendingOptions Options = { };
			Options.ApiVersion = EOS_RTCAUDIO_UPDATESENDING_API_LATEST;
			Options.LocalUserId = LocalUserId.GetValueAsEosType();
			Options.RoomName = TCHAR_TO_ANSI(*RoomName);
			Options.AudioStatus = static_cast<EOS_ERTCAudioStatus>(AudioStatus.GetValue());
			EOS_RTCAudio_UpdateSending(EOS_RTC_GetAudioInterface(EOS_Platform_GetRTCInterface(EOSRef->EOSPlatformHandle->PlatformHandle)), &Options, this, [](const EOS_RTCAudio_UpdateSendingCallbackInfo* Data)
			{
				UEOK_RTCSubsystem* Subsystem = static_cast<UEOK_RTCSubsystem*>(Data->ClientData);
				if (Subsystem)
				{
					Subsystem->OnUpdateSendingCallback.ExecuteIfBound(static_cast<EEOK_Result>(Data->ResultCode), Data->LocalUserId);
				}
			});
		}
	}
}

void UEOK_RTCSubsystem::EOK_RTCAudio_UpdateSendingVolume(FEOK_ProductUserId LocalUserId, const FString& RoomName,
	float Volume, const FEOK_RTCAudio_OnUpdateSendingVolumeCallback& Callback)
{
	OnUpdateSendingVolumeCallback = Callback;
	if (IOnlineSubsystem* OnlineSub = IOnlineSubsystem::Get("EOK"))
	{
		if (FOnlineSubsystemEOS* EOSRef = static_cast<FOnlineSubsystemEOS*>(OnlineSub))
		{	
			EOS_RTCAudio_UpdateSendingVolumeOptions Options = { };
			Options.ApiVersion = EOS_RTCAUDIO_UPDATESENDINGVOLUME_API_LATEST;
			Options.LocalUserId = LocalUserId.GetValueAsEosType();
			Options.RoomName = TCHAR_TO_ANSI(*RoomName);
			Options.Volume = Volume;
			EOS_RTCAudio_UpdateSendingVolume(EOS_RTC_GetAudioInterface(EOS_Platform_GetRTCInterface(EOSRef->EOSPlatformHandle->PlatformHandle)), &Options, this, [](const EOS_RTCAudio_UpdateSendingVolumeCallbackInfo* Data)
			{
				UEOK_RTCSubsystem* Subsystem = static_cast<UEOK_RTCSubsystem*>(Data->ClientData);
				if (Subsystem)
				{
					Subsystem->OnUpdateSendingVolumeCallback.ExecuteIfBound(static_cast<EEOK_Result>(Data->ResultCode), Data->LocalUserId);
				}
			});
		}
	}
}

FEOK_NotificationId UEOK_RTCSubsystem::EOK_RTCData_AddNotifyDataReceived(FEOK_ProductUserId LocalUserId,
	const FString& RoomName, const FEOK_RTCData_OnDataReceivedCallback& Callback)
{
	OnDataReceivedCallback = Callback;
	if (IOnlineSubsystem* OnlineSub = IOnlineSubsystem::Get("EOK"))
	{
		if (FOnlineSubsystemEOS* EOSRef = static_cast<FOnlineSubsystemEOS*>(OnlineSub))
		{
			EOS_RTCData_AddNotifyDataReceivedOptions Options = { };
			Options.ApiVersion = EOS_RTCDATA_ADDNOTIFYDATARECEIVED_API_LATEST;
			Options.LocalUserId = LocalUserId.GetValueAsEosType();
			Options.RoomName = TCHAR_TO_ANSI(*RoomName);
			return EOS_RTCData_AddNotifyDataReceived(EOS_RTC_GetDataInterface(EOS_Platform_GetRTCInterface(EOSRef->EOSPlatformHandle->PlatformHandle)), &Options, this, [](const EOS_RTCData_DataReceivedCallbackInfo* Data)
			{
				UEOK_RTCSubsystem* Subsystem = static_cast<UEOK_RTCSubsystem*>(Data->ClientData);
				if (Subsystem)
				{
					TArray<uint8> DataStr;
					DataStr.SetNumUninitialized(Data->DataLengthBytes);
					FMemory::Memcpy(DataStr.GetData(), Data->Data, Data->DataLengthBytes);
					Subsystem->OnDataReceivedCallback.ExecuteIfBound(Data->LocalUserId, UTF8_TO_TCHAR(Data->RoomName), DataStr, Data->ParticipantId);
				}
			});
		}
	}
	UE_LOG(LogEOK, Error, TEXT("EOK_RTCSubsystem::EOK_RTCData_AddNotifyDataReceived: OnlineSubsystemEOK is not available"));
	return FEOK_NotificationId();
}

FEOK_NotificationId UEOK_RTCSubsystem::EOK_RTCData_AddNotifyParticipantUpdated(FEOK_ProductUserId LocalUserId,
	const FString& RoomName, const FEOK_RTCData_OnParticipantUpdatedCallback& Callback)
{
	OnData_ParticipantUpdatedCallback = Callback;
	if (IOnlineSubsystem* OnlineSub = IOnlineSubsystem::Get("EOK"))
	{
		if (FOnlineSubsystemEOS* EOSRef = static_cast<FOnlineSubsystemEOS*>(OnlineSub))
		{
			EOS_RTCData_AddNotifyParticipantUpdatedOptions Options = { };
			Options.ApiVersion = EOS_RTCDATA_ADDNOTIFYPARTICIPANTUPDATED_API_LATEST;
			Options.LocalUserId = LocalUserId.GetValueAsEosType();
			Options.RoomName = TCHAR_TO_ANSI(*RoomName);
			return EOS_RTCData_AddNotifyParticipantUpdated(EOS_RTC_GetDataInterface(EOS_Platform_GetRTCInterface(EOSRef->EOSPlatformHandle->PlatformHandle)), &Options, this, [](const EOS_RTCData_ParticipantUpdatedCallbackInfo* Data)
			{
				UEOK_RTCSubsystem* Subsystem = static_cast<UEOK_RTCSubsystem*>(Data->ClientData);
				if (Subsystem)
				{
					Subsystem->OnData_ParticipantUpdatedCallback.ExecuteIfBound(Data->LocalUserId, UTF8_TO_TCHAR(Data->RoomName), Data->ParticipantId, static_cast<EEOK_ERTCDataStatus>(Data->DataStatus));
					
				}
			});
		}
	}
	UE_LOG(LogEOK, Error, TEXT("EOK_RTCSubsystem::EOK_RTCData_AddNotifyParticipantUpdated: OnlineSubsystemEOK is not available"));
	return FEOK_NotificationId();
}

void UEOK_RTCSubsystem::EOK_RTCData_RemoveNotifyDataReceived(FEOK_NotificationId NotificationId)
{
	if (IOnlineSubsystem* OnlineSub = IOnlineSubsystem::Get("EOK"))
	{
		if (FOnlineSubsystemEOS* EOSRef = static_cast<FOnlineSubsystemEOS*>(OnlineSub))
		{
			EOS_RTCData_RemoveNotifyDataReceived(EOS_RTC_GetDataInterface(EOS_Platform_GetRTCInterface(EOSRef->EOSPlatformHandle->PlatformHandle)), NotificationId.GetValueAsEosType());
		}
	}
}

void UEOK_RTCSubsystem::EOK_RTCData_RemoveNotifyParticipantUpdated(FEOK_NotificationId NotificationId)
{
	if (IOnlineSubsystem* OnlineSub = IOnlineSubsystem::Get("EOK"))
	{
		if (FOnlineSubsystemEOS* EOSRef = static_cast<FOnlineSubsystemEOS*>(OnlineSub))
		{
			EOS_RTCData_RemoveNotifyParticipantUpdated(EOS_RTC_GetDataInterface(EOS_Platform_GetRTCInterface(EOSRef->EOSPlatformHandle->PlatformHandle)), NotificationId.GetValueAsEosType());
		}
	}
}

TEnumAsByte<EEOK_Result> UEOK_RTCSubsystem::EOK_RTCData_SendData(FEOK_ProductUserId LocalUserId,
	const FString& RoomName, const TArray<uint8>& Data)
{
	if (Data.Num() == 0)
	{
		UE_LOG(LogEOK, Error, TEXT("EOK_RTCSubsystem::EOK_RTCData_SendData: Data is empty"));
		return EEOK_Result::EOS_InvalidParameters;
	}
	if (IOnlineSubsystem* OnlineSub = IOnlineSubsystem::Get("EOK"))
	{
		if (FOnlineSubsystemEOS* EOSRef = static_cast<FOnlineSubsystemEOS*>(OnlineSub))
		{	
			EOS_RTCData_SendDataOptions Options = { };
			Options.ApiVersion = EOS_RTCDATA_SENDDATA_API_LATEST;
			Options.LocalUserId = LocalUserId.GetValueAsEosType();
			Options.RoomName = TCHAR_TO_ANSI(*RoomName);
			Options.Data = Data.GetData();
			Options.DataLengthBytes = Data.Num();
			return static_cast<EEOK_Result>(EOS_RTCData_SendData(EOS_RTC_GetDataInterface(EOS_Platform_GetRTCInterface(EOSRef->EOSPlatformHandle->PlatformHandle)), &Options));
		}
	}
	UE_LOG(LogEOK, Error, TEXT("EOK_RTCSubsystem::EOK_RTCData_SendData: OnlineSubsystemEOK is not available"));
	return EEOK_Result::EOS_NotFound;
}

void UEOK_RTCSubsystem::EOK_RTCData_UpdateReceiving(FEOK_ProductUserId LocalUserId, const FString& RoomName,
	FEOK_ProductUserId ParticipantId, bool bDataEnabled, const FEOK_RTCData_OnUpdateReceivingCallback& Callback)
{
	OnData_UpdateReceivingCallback = Callback;
	if (IOnlineSubsystem* OnlineSub = IOnlineSubsystem::Get("EOK"))
	{
		if (FOnlineSubsystemEOS* EOSRef = static_cast<FOnlineSubsystemEOS*>(OnlineSub))
		{
			EOS_RTCData_UpdateReceivingOptions Options = { };
			Options.ApiVersion = EOS_RTCDATA_UPDATERECEIVING_API_LATEST;
			Options.LocalUserId = LocalUserId.GetValueAsEosType();
			Options.RoomName = TCHAR_TO_ANSI(*RoomName);
			Options.ParticipantId = ParticipantId.GetValueAsEosType();
			Options.bDataEnabled = bDataEnabled;
			EOS_RTCData_UpdateReceiving(EOS_RTC_GetDataInterface(EOS_Platform_GetRTCInterface(EOSRef->EOSPlatformHandle->PlatformHandle)), &Options, this, [](const EOS_RTCData_UpdateReceivingCallbackInfo* Data)
			{
				UEOK_RTCSubsystem* Subsystem = static_cast<UEOK_RTCSubsystem*>(Data->ClientData);
				if (Subsystem)
				{
					Subsystem->OnData_UpdateReceivingCallback.ExecuteIfBound(static_cast<EEOK_Result>(Data->ResultCode), Data->LocalUserId, UTF8_TO_TCHAR(Data->RoomName), Data->ParticipantId);
				}
			});
		}
	}
}

void UEOK_RTCSubsystem::EOK_RTCData_UpdateSending(FEOK_ProductUserId LocalUserId, const FString& RoomName,
	bool bDataEnabled, const FEOK_RTCData_OnUpdateSendingCallback& Callback)
{
	OnData_UpdateSendingCallback = Callback;
	if (IOnlineSubsystem* OnlineSub = IOnlineSubsystem::Get("EOK"))
	{
		if (FOnlineSubsystemEOS* EOSRef = static_cast<FOnlineSubsystemEOS*>(OnlineSub))
		{
			EOS_RTCData_UpdateSendingOptions Options = { };
			Options.ApiVersion = EOS_RTCDATA_UPDATESENDING_API_LATEST;
			Options.LocalUserId = LocalUserId.GetValueAsEosType();
			Options.RoomName = TCHAR_TO_ANSI(*RoomName);
			Options.bDataEnabled = bDataEnabled;
			EOS_RTCData_UpdateSending(EOS_RTC_GetDataInterface(EOS_Platform_GetRTCInterface(EOSRef->EOSPlatformHandle->PlatformHandle)), &Options, this, [](const EOS_RTCData_UpdateSendingCallbackInfo* Data)
			{
				UEOK_RTCSubsystem* Subsystem = static_cast<UEOK_RTCSubsystem*>(Data->ClientData);
				if (Subsystem)
				{
					Subsystem->OnData_UpdateSendingCallback.ExecuteIfBound(static_cast<EEOK_Result>(Data->ResultCode), Data->LocalUserId);
				}
			});
		}
	}
}
