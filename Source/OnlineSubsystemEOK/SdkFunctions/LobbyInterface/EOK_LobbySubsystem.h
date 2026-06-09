// Copyright (c) 2023 Betide Studio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "OnlineSubsystemEOK/SdkFunctions/EOK_SharedFunctionFile.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "EOK_LobbySubsystem.generated.h"

DECLARE_DYNAMIC_DELEGATE_TwoParams(FEOK_Lobby_OnJoinLobbyAcceptedCallback, FEOK_ProductUserId, LocalUserId, const FEOK_UI_EventId&, UiEventId);
DECLARE_DYNAMIC_DELEGATE_TwoParams(FEOK_Lobby_OnLeaveLobbyRequestedCallback, FEOK_ProductUserId, LocalUserId, const FEOK_LobbyId&, LobbyId);
DECLARE_DYNAMIC_DELEGATE_FourParams(FEOK_Lobby_OnLobbyInviteAcceptedCallback, FEOK_ProductUserId, LocalUserId, FEOK_ProductUserId, TargetUserId, const FEOK_LobbyId&, LobbyId, const FString&, InviteId);
DECLARE_DYNAMIC_DELEGATE_ThreeParams(FEOK_Lobby_OnLobbyInviteReceivedCallback, FEOK_ProductUserId, LocalUserId, FEOK_ProductUserId, TargetUserId, const FString&, InviteId);
DECLARE_DYNAMIC_DELEGATE_FourParams(FEOK_Lobby_OnLobbyInviteRejectedCallback, FEOK_ProductUserId, LocalUserId, FEOK_ProductUserId, TargetUserId, const FEOK_LobbyId&, LobbyId, const FString&, InviteId);
DECLARE_DYNAMIC_DELEGATE_ThreeParams(FEOK_Lobby_OnLobbyMemberStatusReceivedCallback, FEOK_ProductUserId, TargetUserId, const FEOK_LobbyId&, LobbyId, const TEnumAsByte<EEOK_ELobbyMemberStatus>&, CurrentStatus);
DECLARE_DYNAMIC_DELEGATE_TwoParams(FEOK_Lobby_OnLobbyMemberUpdateReceivedCallback, FEOK_ProductUserId, TargetUserId, const FEOK_LobbyId&, LobbyId);
DECLARE_DYNAMIC_DELEGATE_OneParam(FEOK_Lobby_OnLobbyUpdateReceivedCallback, const FEOK_LobbyId&, LobbyId);
DECLARE_DYNAMIC_DELEGATE_FourParams(FEOK_Lobby_OnRTCRoomConnectionChangedCallback, const FEOK_LobbyId&, LobbyId, const FEOK_ProductUserId&, LocalUserId, bool, bIsConnected, const TEnumAsByte<EEOK_Result>&, DisconnectReason);
DECLARE_DYNAMIC_DELEGATE_FiveParams(FEOK_Lobby_OnSendLobbyNativeInviteCallback, FEOK_UI_EventId, UiEventId, FEOK_ProductUserId, LocalUserId, const FString&, TargetNativeAccountType, const FString&, TargetUserNativeAccountId, const FEOK_LobbyId&, LobbyId);

UCLASS(DisplayName="Lobby Interface", meta=(DisplayName="Lobby Interface"))
class OnlineSubsystemEOK_API UEOK_LobbySubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	FEOK_Lobby_OnJoinLobbyAcceptedCallback OnJoinLobbyAccepted;
	//Register to receive notifications about lobby "JOIN" performed by local user (when no invite) via the overlay.
	UFUNCTION(BlueprintCallable, Category = "Epic Online Services-Kit V2 | SDK Functions | Lobby Interface", DisplayName="EOS_Lobby_AddNotifyJoinLobbyAccepted")
	FEOK_NotificationId EOK_Lobby_AddNotifyJoinLobbyAccepted(FEOK_Lobby_OnJoinLobbyAcceptedCallback Callback);

	//Unregister from receiving notifications when a user accepts a lobby invitation via the overlay.
	UFUNCTION(BlueprintCallable, Category = "Epic Online Services-Kit V2 | SDK Functions | Lobby Interface", DisplayName="EOS_Lobby_RemoveNotifyJoinLobbyAccepted")
	void EOK_Lobby_RemoveNotifyJoinLobbyAccepted(FEOK_NotificationId InId);


	FEOK_Lobby_OnLeaveLobbyRequestedCallback OnLeaveLobbyRequested;
	//Register to receive notifications about leave lobby requests performed by the local user via the overlay. When user requests to leave the lobby in the social overlay, the SDK does not automatically leave the lobby, it is up to the game to perform any necessary cleanup and call the EOS_Lobby_LeaveLobby method using the lobbyId sent in the notification function.
	UFUNCTION(BlueprintCallable, Category = "Epic Online Services-Kit V2 | SDK Functions | Lobby Interface", DisplayName="EOS_Lobby_AddNotifyLeaveLobbyRequested")
	FEOK_NotificationId EOK_Lobby_AddNotifyLeaveLobbyRequested(FEOK_Lobby_OnLeaveLobbyRequestedCallback Callback);

	//Unregister from receiving notifications when a user performs a leave lobby action via the overlay.
	UFUNCTION(BlueprintCallable, Category = "Epic Online Services-Kit V2 | SDK Functions | Lobby Interface", DisplayName="EOS_Lobby_RemoveNotifyLeaveLobbyRequested")
	void EOK_Lobby_RemoveNotifyLeaveLobbyRequested(FEOK_NotificationId InId);
	
	FEOK_Lobby_OnLobbyInviteAcceptedCallback OnLobbyInviteAccepted;
	//Register to receive notifications about lobby invites accepted by local user via the overlay.
	UFUNCTION(BlueprintCallable, Category = "Epic Online Services-Kit V2 | SDK Functions | Lobby Interface", DisplayName="EOS_Lobby_AddNotifyLobbyInviteAccepted")
	FEOK_NotificationId EOK_Lobby_AddNotifyLobbyInviteAccepted(FEOK_Lobby_OnLobbyInviteAcceptedCallback Callback);

	//Unregister from receiving notifications when a user accepts a lobby invitation via the overlay.
	UFUNCTION(BlueprintCallable, Category = "Epic Online Services-Kit V2 | SDK Functions | Lobby Interface", DisplayName="EOS_Lobby_RemoveNotifyLobbyInviteAccepted")
	void EOK_Lobby_RemoveNotifyLobbyInviteAccepted(FEOK_NotificationId InId);

	
	FEOK_Lobby_OnLobbyInviteReceivedCallback OnLobbyInviteReceived;
	//Register to receive notifications about lobby invites sent to local users.
	UFUNCTION(BlueprintCallable, Category = "Epic Online Services-Kit V2 | SDK Functions | Lobby Interface", DisplayName="EOS_Lobby_AddNotifyLobbyInviteReceived")
	FEOK_NotificationId EOK_Lobby_AddNotifyLobbyInviteReceived(FEOK_Lobby_OnLobbyInviteReceivedCallback Callback);

	//Unregister from receiving notifications when a user receives a lobby invitation.
	UFUNCTION(BlueprintCallable, Category = "Epic Online Services-Kit V2 | SDK Functions | Lobby Interface", DisplayName="EOS_Lobby_RemoveNotifyLobbyInviteReceived")
	void EOK_Lobby_RemoveNotifyLobbyInviteReceived(FEOK_NotificationId InId);


	
	FEOK_Lobby_OnLobbyInviteRejectedCallback OnLobbyInviteRejected;
	//Register to receive notifications about lobby invites rejected by local user.
	UFUNCTION(BlueprintCallable, Category = "Epic Online Services-Kit V2 | SDK Functions | Lobby Interface", DisplayName="EOS_Lobby_AddNotifyLobbyInviteRejected")
	FEOK_NotificationId EOK_Lobby_AddNotifyLobbyInviteRejected(FEOK_Lobby_OnLobbyInviteRejectedCallback Callback);

	//Unregister from receiving notifications when a user rejects a lobby invitation via the overlay.
	UFUNCTION(BlueprintCallable, Category = "Epic Online Services-Kit V2 | SDK Functions | Lobby Interface", DisplayName="EOS_Lobby_RemoveNotifyLobbyInviteRejected")
	void EOK_Lobby_RemoveNotifyLobbyInviteRejected(FEOK_NotificationId InId);

	
	
	FEOK_Lobby_OnLobbyMemberStatusReceivedCallback OnLobbyMemberStatusReceived;
	//Register to receive notifications about the changing status of lobby members.
	UFUNCTION(BlueprintCallable, Category = "Epic Online Services-Kit V2 | SDK Functions | Lobby Interface", DisplayName="EOS_Lobby_AddNotifyLobbyMemberStatusReceived")
	FEOK_NotificationId EOK_Lobby_AddNotifyLobbyMemberStatusReceived(FEOK_Lobby_OnLobbyMemberStatusReceivedCallback Callback);

	//Unregister from receiving notifications when lobby members status change.
	UFUNCTION(BlueprintCallable, Category = "Epic Online Services-Kit V2 | SDK Functions | Lobby Interface", DisplayName="EOS_Lobby_RemoveNotifyLobbyMemberStatusReceived")
	void EOK_Lobby_RemoveNotifyLobbyMemberStatusReceived(FEOK_NotificationId InId);

	

	FEOK_Lobby_OnLobbyMemberUpdateReceivedCallback OnLobbyMemberUpdateReceived;
	//Register to receive notifications when a lobby member updates the attributes associated with themselves inside the lobby.
	UFUNCTION(BlueprintCallable, Category = "Epic Online Services-Kit V2 | SDK Functions | Lobby Interface", DisplayName="EOS_Lobby_AddNotifyLobbyMemberUpdateReceived")
	FEOK_NotificationId EOK_Lobby_AddNotifyLobbyMemberUpdateReceived(FEOK_Lobby_OnLobbyMemberUpdateReceivedCallback Callback);

	//Unregister from receiving notifications when lobby members change their data.
	UFUNCTION(BlueprintCallable, Category = "Epic Online Services-Kit V2 | SDK Functions | Lobby Interface", DisplayName="EOS_Lobby_RemoveNotifyLobbyMemberUpdateReceived")
	void EOK_Lobby_RemoveNotifyLobbyMemberUpdateReceived(FEOK_NotificationId InId);

	
	
	FEOK_Lobby_OnLobbyUpdateReceivedCallback OnLobbyUpdateReceived;
	//Register to receive notifications when a lobby owner updates the attributes associated with the lobby.
	UFUNCTION(BlueprintCallable, Category = "Epic Online Services-Kit V2 | SDK Functions | Lobby Interface", DisplayName="EOS_Lobby_AddNotifyLobbyUpdateReceived")
	FEOK_NotificationId EOK_Lobby_AddNotifyLobbyUpdateReceived(FEOK_Lobby_OnLobbyUpdateReceivedCallback Callback);

	//Unregister from receiving notifications when a lobby changes its data.
	UFUNCTION(BlueprintCallable, Category = "Epic Online Services-Kit V2 | SDK Functions | Lobby Interface", DisplayName="EOS_Lobby_RemoveNotifyLobbyUpdateReceived")
	void EOK_Lobby_RemoveNotifyLobbyUpdateReceived(FEOK_NotificationId InId);


	
	FEOK_Lobby_OnRTCRoomConnectionChangedCallback OnRTCRoomConnectionChanged;
	//Register to receive notifications of when the RTC Room for a particular lobby has a connection status change. The RTC Room connection status is independent of the lobby connection status, however the lobby system will attempt to keep them consistent, automatically connecting to the RTC room after joining a lobby which has an associated RTC room and disconnecting from the RTC room when a lobby is left or disconnected. This notification is entirely informational and requires no action in response by the application. If the connected status is offline (bIsConnected is EOS_FALSE), the connection will automatically attempt to reconnect. The purpose of this notification is to allow applications to show the current connection status of the RTC room when the connection is not established. Unlike EOS_RTC_AddNotifyDisconnected, EOS_RTC_LeaveRoom should not be called when the RTC room is disconnected. This function will only succeed when called on a lobby the local user is currently a member of.
	UFUNCTION(BlueprintCallable, Category = "Epic Online Services-Kit V2 | SDK Functions | Lobby Interface", DisplayName="EOS_Lobby_AddNotifyRTCRoomConnectionChanged")
	FEOK_NotificationId EOK_Lobby_AddNotifyRTCRoomConnectionChanged(FEOK_Lobby_OnRTCRoomConnectionChangedCallback Callback);

	//Unregister from receiving notifications when an RTC Room's connection status changes. This should be called when the local user is leaving a lobby.
	UFUNCTION(BlueprintCallable, Category = "Epic Online Services-Kit V2 | SDK Functions | Lobby Interface", DisplayName="EOS_Lobby_RemoveNotifyRTCRoomConnectionChanged")
	void EOK_Lobby_RemoveNotifyRTCRoomConnectionChanged(FEOK_NotificationId InId);


	
	FEOK_Lobby_OnSendLobbyNativeInviteCallback OnSendLobbyNativeInvite;
	//Register to receive notifications about a lobby "INVITE" performed by a local user via the overlay. This is only needed when a configured integrated platform has EOS_IPMF_DisableSDKManagedSessions set. The EOS SDK will then use the state of EOS_IPMF_PreferEOSIdentity and EOS_IPMF_PreferIntegratedIdentity to determine when the NotificationFn is called.
	UFUNCTION(BlueprintCallable, Category = "Epic Online Services-Kit V2 | SDK Functions | Lobby Interface", DisplayName="EOS_Lobby_AddNotifySendLobbyNativeInviteRequested")
	FEOK_NotificationId EOK_Lobby_AddNotifySendLobbyNativeInviteRequested(FEOK_Lobby_OnSendLobbyNativeInviteCallback Callback);

	//Unregister from receiving notifications when a user requests a send invite via the overlay.
	UFUNCTION(BlueprintCallable, Category = "Epic Online Services-Kit V2 | SDK Functions | Lobby Interface", DisplayName="EOS_Lobby_RemoveNotifySendLobbyNativeInviteRequested")
	void EOK_Lobby_RemoveNotifySendLobbyNativeInviteRequested(FEOK_NotificationId InId);





	UFUNCTION(BlueprintCallable, Category = "Epic Online Services-Kit V2 | SDK Functions | Lobby Interface", DisplayName="EOS_Lobby_Attribute_Release")
	void EOK_Lobby_Attribute_Release(const FEOK_Lobby_Attribute& Attribute);

	//Create a handle to an existing lobby. If the call returns an EOS_Success result, the out parameter, OutLobbyDetailsHandle, must be passed to EOS_LobbyDetails_Release to release the memory associated with it.
	UFUNCTION(BlueprintCallable, Category = "Epic Online Services-Kit V2 | SDK Functions | Lobby Interface", DisplayName="EOS_Lobby_CopyLobbyDetailsHandle")
	TEnumAsByte<EEOK_Result> EOK_Lobby_CopyLobbyDetailsHandle(FEOK_LobbyId LobbyId, FEOK_ProductUserId LocalUserId, FEOK_HLobbyDetails& OutLobbyDetailsHandle);

	//EOS_Lobby_CopyLobbyDetailsHandleByInviteId is used to immediately retrieve a handle to the lobby information from after notification of an invite If the call returns an EOS_Success result, the out parameter, OutLobbyDetailsHandle, must be passed to EOS_LobbyDetails_Release to release the memory associated with it.
	UFUNCTION(BlueprintCallable, Category = "Epic Online Services-Kit V2 | SDK Functions | Lobby Interface", DisplayName="EOS_Lobby_CopyLobbyDetailsHandleByInviteId")
	TEnumAsByte<EEOK_Result> EOK_Lobby_CopyLobbyDetailsHandleByInviteId(FString InviteId, FEOK_HLobbyDetails& OutLobbyDetailsHandle);

	//EOS_Lobby_CopyLobbyDetailsHandleByUiEventId is used to immediately retrieve a handle to the lobby information from after notification of an join game If the call returns an EOS_Success result, the out parameter, OutLobbyDetailsHandle, must be passed to EOS_LobbyDetails_Release to release the memory associated with it.
	UFUNCTION(BlueprintCallable, Category = "Epic Online Services-Kit V2 | SDK Functions | Lobby Interface", DisplayName="EOS_Lobby_CopyLobbyDetailsHandleByUiEventId")
	TEnumAsByte<EEOK_Result> EOK_Lobby_CopyLobbyDetailsHandleByUiEventId(const FEOK_UI_EventId& UiEventId, FEOK_HLobbyDetails& OutLobbyDetailsHandle);


	/*
	Create a lobby search handle. This handle may be modified to include various search parameters. Searching is possible in three methods, all mutually exclusive

	set the lobby ID to find a specific lobby
	set the target user ID to find a specific user
	set lobby parameters to find an array of lobbies that match the search criteria
	*/
	UFUNCTION(BlueprintCallable, Category = "Epic Online Services-Kit V2 | SDK Functions | Lobby Interface", DisplayName="EOS_Lobby_CreateLobbySearch")
	TEnumAsByte<EEOK_Result> EOK_Lobby_CreateLobbySearch(int32 MaxResults, FEOK_HLobbySearch& OutLobbySearchHandle);

	//Get the Connection string for an EOS lobby. The connection string describes the presence of a player in terms of game state. Xbox platforms expect titles to embed this into their MultiplayerActivity at creation. When present, the SDK will use this value to populate session presence in the social overlay and facilitate platform invitations.
	UFUNCTION(BlueprintCallable, Category = "Epic Online Services-Kit V2 | SDK Functions | Lobby Interface", DisplayName="EOS_Lobby_GetConnectString")
	TEnumAsByte<EEOK_Result> EOK_Lobby_GetConnectString(FEOK_ProductUserId LocalUserId, FEOK_LobbyId LobbyId, FString& OutConnectString);

	//Get the number of known invites for a given user
	UFUNCTION(BlueprintCallable, Category = "Epic Online Services-Kit V2 | SDK Functions | Lobby Interface", DisplayName="EOS_Lobby_GetInviteCount")
	int32 EOK_Lobby_GetInviteCount(FEOK_ProductUserId LocalUserId);

	//Retrieve an invite ID from a list of active invites for a given user
	UFUNCTION(BlueprintCallable, Category = "Epic Online Services-Kit V2 | SDK Functions | Lobby Interface", DisplayName="EOS_Lobby_GetInviteIdByIndex")
	TEnumAsByte<EEOK_Result> EOK_Lobby_GetInviteIdByIndex(FEOK_ProductUserId LocalUserId, int32 Index, FString& OutInviteId);

	//Get the name of the RTC room associated with a specific lobby a local user belongs to. This value can be used whenever you need a RoomName value in the RTC_* suite of functions. RTC Room Names must not be used with EOS_RTC_JoinRoom, EOS_RTC_LeaveRoom, or EOS_RTC_AddNotifyDisconnected. Doing so will return EOS_AccessDenied or EOS_INVALID_NOTIFICATIONID if used with those functions. This function will only succeed when called on a lobby the local user is currently a member of.
	UFUNCTION(BlueprintCallable, Category = "Epic Online Services-Kit V2 | SDK Functions | Lobby Interface", DisplayName="EOS_Lobby_GetRTCRoomName")
	TEnumAsByte<EEOK_Result> EOK_Lobby_GetRTCRoomName(FEOK_ProductUserId LocalUserId, FEOK_LobbyId LobbyId, FString& OutRTCRoomName);
	
	//Get the current connection status of the RTC Room for a lobby. The RTC Room connection status is independent of the lobby connection status, however the lobby system will attempt to keep them consistent, automatically connecting to the RTC room after joining a lobby which has an associated RTC room and disconnecting from the RTC room when a lobby is left or disconnected. This function will only succeed when called on a lobby the local user is currently a member of.
	UFUNCTION(BlueprintCallable, Category = "Epic Online Services-Kit V2 | SDK Functions | Lobby Interface", DisplayName="EOS_Lobby_IsRTCRoomConnected")
	TEnumAsByte<EEOK_Result> EOK_Lobby_IsRTCRoomConnected(FEOK_ProductUserId LocalUserId, FEOK_LobbyId LobbyId, bool& bOutIsConnected);

	//Parse the ConnectString for an EOS lobby invitation to extract just the lobby ID. Used for joining a lobby from a connection string (as generated by GetConnectString) found in a platform invitation or presence.
	UFUNCTION(BlueprintCallable, Category = "Epic Online Services-Kit V2 | SDK Functions | Lobby Interface", DisplayName="EOS_Lobby_ParseLobbyIdFromConnectString")
	TEnumAsByte<EEOK_Result> EOK_Lobby_ParseLobbyIdFromConnectString(FString ConnectString, FEOK_LobbyId& OutLobbyId);

	//Creates a lobby modification handle (EOS_HLobbyModification). The lobby modification handle is used to modify an existing lobby and can be applied with EOS_Lobby_UpdateLobby. The EOS_HLobbyModification must be released by calling EOS_LobbyModification_Release once it is no longer needed.
	UFUNCTION(BlueprintCallable, Category = "Epic Online Services-Kit V2 | SDK Functions | Lobby Interface", DisplayName="EOS_Lobby_UpdateLobbyModification")
	TEnumAsByte<EEOK_Result> EOK_Lobby_UpdateLobbyModification(FEOK_ProductUserId LocalUserId, FEOK_LobbyId LobbyId, FEOK_HLobbyModification& OutLobbyModificationHandle);
	
	//EOS_LobbyDetails_CopyAttributeByIndex is used to immediately retrieve a copy of a lobby attribute from a given source such as a existing lobby or a search result. If the call returns an EOS_Success result, the out parameter, OutAttribute, must be passed to EOS_Lobby_Attribute_Release to release the memory associated with it.
	UFUNCTION(BlueprintCallable, Category = "Epic Online Services-Kit V2 | SDK Functions | Lobby Interface", DisplayName="EOS_LobbyDetails_CopyAttributeByIndex")
	TEnumAsByte<EEOK_Result> EOK_LobbyDetails_CopyAttributeByIndex(FEOK_HLobbyDetails LobbyDetailsHandle, int32 AttrIndex, FEOK_Lobby_Attribute& OutAttribute);

	//EOS_LobbyDetails_CopyAttributeByKey is used to immediately retrieve a copy of a lobby attribute from a given source such as a existing lobby or a search result. If the call returns an EOS_Success result, the out parameter, OutAttribute, must be passed to EOS_Lobby_Attribute_Release to release the memory associated with it.
	UFUNCTION(BlueprintCallable, Category = "Epic Online Services-Kit V2 | SDK Functions | Lobby Interface", DisplayName="EOS_LobbyDetails_CopyAttributeByKey")
	TEnumAsByte<EEOK_Result> EOK_LobbyDetails_CopyAttributeByKey(FEOK_HLobbyDetails LobbyDetailsHandle, const FString& AttrKey, FEOK_Lobby_Attribute& OutAttribute);

	//EOS_LobbyDetails_CopyInfo is used to immediately retrieve a copy of lobby information from a given source such as a existing lobby or a search result. If the call returns an EOS_Success result, the out parameter, OutLobbyDetailsInfo, must be passed to EOS_LobbyDetails_Info_Release to release the memory associated with it.
	UFUNCTION(BlueprintCallable, Category = "Epic Online Services-Kit V2 | SDK Functions | Lobby Interface", DisplayName="EOS_LobbyDetails_CopyInfo")
	TEnumAsByte<EEOK_Result> EOK_LobbyDetails_CopyInfo(FEOK_HLobbyDetails LobbyDetailsHandle, FEOK_LobbyDetailsInfo& OutLobbyDetailsInfo);

	//EOS_LobbyDetails_CopyMemberAttributeByIndex is used to immediately retrieve a copy of a lobby member attribute from an existing lobby. If the call returns an EOS_Success result, the out parameter, OutAttribute, must be passed to EOS_Lobby_Attribute_Release to release the memory associated with it. Note: this information is only available if you are actively in the lobby. It is not available for search results.
	UFUNCTION(BlueprintCallable, Category = "Epic Online Services-Kit V2 | SDK Functions | Lobby Interface", DisplayName="EOS_LobbyDetails_CopyMemberAttributeByIndex")
	TEnumAsByte<EEOK_Result> EOK_LobbyDetails_CopyMemberAttributeByIndex(FEOK_HLobbyDetails LobbyDetailsHandle, FEOK_ProductUserId TargetUserId, int32 AttrIndex, FEOK_Lobby_Attribute& OutAttribute);

	//EOS_LobbyDetails_CopyMemberAttributeByIndex is used to immediately retrieve a copy of a lobby member attribute from an existing lobby. If the call returns an EOS_Success result, the out parameter, OutAttribute, must be passed to EOS_Lobby_Attribute_Release to release the memory associated with it. Note: this information is only available if you are actively in the lobby. It is not available for search results.
	UFUNCTION(BlueprintCallable, Category = "Epic Online Services-Kit V2 | SDK Functions | Lobby Interface", DisplayName="EOS_LobbyDetails_CopyMemberAttributeByKey")
	TEnumAsByte<EEOK_Result> EOK_LobbyDetails_CopyMemberAttributeByKey(FEOK_HLobbyDetails LobbyDetailsHandle, FEOK_ProductUserId TargetUserId, const FString& AttrKey, FEOK_Lobby_Attribute& OutAttribute);

	UFUNCTION(BlueprintCallable, Category = "Epic Online Services-Kit V2 | SDK Functions | Lobby Interface", DisplayName="EOS_LobbyDetails_CopyMemberInfo")
	TEnumAsByte<EEOK_Result> EOK_LobbyDetails_CopyMemberInfo(FEOK_HLobbyDetails LobbyDetailsHandle, FEOK_ProductUserId TargetUserId, FEOK_LobbyDetails_MemberInfo& OutMemberInfo);

	UFUNCTION(BlueprintCallable, Category = "Epic Online Services-Kit V2 | SDK Functions | Lobby Interface", DisplayName="EOS_LobbyDetails_GetAttributeCount")
	int32 EOK_LobbyDetails_GetAttributeCount(FEOK_HLobbyDetails LobbyDetailsHandle);

	UFUNCTION(BlueprintCallable, Category = "Epic Online Services-Kit V2 | SDK Functions | Lobby Interface", DisplayName="EOS_LobbyDetails_GetLobbyOwner")
	FEOK_ProductUserId EOK_LobbyDetails_GetLobbyOwner(FEOK_HLobbyDetails LobbyDetailsHandle);

	UFUNCTION(BlueprintCallable, Category = "Epic Online Services-Kit V2 | SDK Functions | Lobby Interface", DisplayName="EOS_LobbyDetails_GetMemberAttributeCount")
	int32 EOK_LobbyDetails_GetMemberAttributeCount(FEOK_HLobbyDetails LobbyDetailsHandle, FEOK_ProductUserId TargetUserId);

	UFUNCTION(BlueprintCallable, Category = "Epic Online Services-Kit V2 | SDK Functions | Lobby Interface", DisplayName="EOS_LobbyDetails_GetMemberByIndex")
	FEOK_ProductUserId EOK_LobbyDetails_GetMemberByIndex(FEOK_HLobbyDetails LobbyDetailsHandle, int32 MemberIndex);

	UFUNCTION(BlueprintCallable, Category = "Epic Online Services-Kit V2 | SDK Functions | Lobby Interface", DisplayName="EOS_LobbyDetails_GetMemberCount")
	int32 EOK_LobbyDetails_GetMemberCount(FEOK_HLobbyDetails LobbyDetailsHandle);

	UFUNCTION(BlueprintCallable, Category = "Epic Online Services-Kit V2 | SDK Functions | Lobby Interface", DisplayName="EOS_LobbyDetails_Info_Release")
	void EOK_LobbyDetails_Info_Release(FEOK_LobbyDetailsInfo& LobbyDetailsInfo);

	UFUNCTION(BlueprintCallable, Category = "Epic Online Services-Kit V2 | SDK Functions | Lobby Interface", DisplayName="EOS_LobbyDetails_MemberInfo_Release")
	void EOK_LobbyDetails_MemberInfo_Release(FEOK_LobbyDetails_MemberInfo& MemberInfo);

	UFUNCTION(BlueprintCallable, Category = "Epic Online Services-Kit V2 | SDK Functions | Lobby Interface", DisplayName="EOS_LobbyDetails_Release")
	void EOK_LobbyDetails_Release(FEOK_HLobbyDetails LobbyDetailsHandle);

	UFUNCTION(BlueprintCallable, Category = "Epic Online Services-Kit V2 | SDK Functions | Lobby Interface", DisplayName="EOS_LobbyModification_AddAttribute")
	TEnumAsByte<EEOK_Result> EOK_LobbyModification_AddAttribute(FEOK_HLobbyModification LobbyModificationHandle, const FEOK_Lobby_AttributeData& Attribute, const TEnumAsByte<EEOK_ELobbyAttributeVisibility>& Visibility);

	UFUNCTION(BlueprintCallable, Category = "Epic Online Services-Kit V2 | SDK Functions | Lobby Interface", DisplayName="EOS_LobbyModification_AddMemberAttribute")
	TEnumAsByte<EEOK_Result> EOK_LobbyModification_AddMemberAttribute(FEOK_HLobbyModification LobbyModificationHandle, const FEOK_Lobby_AttributeData& Attribute, const TEnumAsByte<EEOK_ELobbyAttributeVisibility>& Visibility);

	UFUNCTION(BlueprintCallable, Category = "Epic Online Services-Kit V2 | SDK Functions | Lobby Interface", DisplayName="EOS_LobbyModification_Release")
	void EOK_LobbyModification_Release(FEOK_HLobbyModification LobbyModificationHandle);

	UFUNCTION(BlueprintCallable, Category = "Epic Online Services-Kit V2 | SDK Functions | Lobby Interface", DisplayName="EOS_LobbyModification_RemoveAttribute")
	TEnumAsByte<EEOK_Result> EOK_LobbyModification_RemoveAttribute(FEOK_HLobbyModification LobbyModificationHandle, const FString& Options);

	UFUNCTION(BlueprintCallable, Category = "Epic Online Services-Kit V2 | SDK Functions | Lobby Interface", DisplayName="EOS_LobbyModification_RemoveMemberAttribute")
	TEnumAsByte<EEOK_Result> EOK_LobbyModification_RemoveMemberAttribute(FEOK_HLobbyModification LobbyModificationHandle, const FString& Options);

	UFUNCTION(BlueprintCallable, Category = "Epic Online Services-Kit V2 | SDK Functions | Lobby Interface", DisplayName="EOS_LobbyModification_SetAllowedPlatformIds")
	TEnumAsByte<EEOK_Result> EOK_LobbyModification_SetAllowedPlatformIds(FEOK_HLobbyModification LobbyModificationHandle, const TArray<int32>& Options);

	UFUNCTION(BlueprintCallable, Category = "Epic Online Services-Kit V2 | SDK Functions | Lobby Interface", DisplayName="EOS_LobbyModification_SetBucketId")
	TEnumAsByte<EEOK_Result> EOK_LobbyModification_SetBucketId(FEOK_HLobbyModification LobbyModificationHandle, const FString& Options);

	UFUNCTION(BlueprintCallable, Category = "Epic Online Services-Kit V2 | SDK Functions | Lobby Interface", DisplayName="EOS_LobbyModification_SetInvitesAllowed")
	TEnumAsByte<EEOK_Result> EOK_LobbyModification_SetInvitesAllowed(FEOK_HLobbyModification LobbyModificationHandle, const bool& Options);

	UFUNCTION(BlueprintCallable, Category = "Epic Online Services-Kit V2 | SDK Functions | Lobby Interface", DisplayName="EOS_LobbyModification_SetMaxMembers")
	TEnumAsByte<EEOK_Result> EOK_LobbyModification_SetMaxMembers(FEOK_HLobbyModification LobbyModificationHandle, const int32& Options);

	UFUNCTION(BlueprintCallable, Category = "Epic Online Services-Kit V2 | SDK Functions | Lobby Interface", DisplayName="EOS_LobbyModification_SetPermissionLevel")
	TEnumAsByte<EEOK_Result> EOK_LobbyModification_SetPermissionLevel(FEOK_HLobbyModification LobbyModificationHandle, const TEnumAsByte<EEOK_ELobbyPermissionLevel>& Options);

	UFUNCTION(BlueprintCallable, Category = "Epic Online Services-Kit V2 | SDK Functions | Lobby Interface", DisplayName="EOS_LobbySearch_CopySearchResultByIndex")
	TEnumAsByte<EEOK_Result> EOK_LobbySearch_CopySearchResultByIndex(FEOK_HLobbySearch LobbySearchHandle, int32 LobbyIndex, FEOK_HLobbyDetails& OutLobbyDetailsHandle);

	UFUNCTION(BlueprintCallable, Category = "Epic Online Services-Kit V2 | SDK Functions | Lobby Interface", DisplayName="EOS_LobbySearch_GetSearchResultCount")
	int32 EOK_LobbySearch_GetSearchResultCount(FEOK_HLobbySearch LobbySearchHandle);

	UFUNCTION(BlueprintCallable, Category = "Epic Online Services-Kit V2 | SDK Functions | Lobby Interface", DisplayName="EOS_LobbySearch_Release")
	void EOK_LobbySearch_Release(FEOK_HLobbySearch LobbySearchHandle);

	UFUNCTION(BlueprintCallable, Category = "Epic Online Services-Kit V2 | SDK Functions | Lobby Interface", DisplayName="EOS_LobbySearch_RemoveParameter")
	TEnumAsByte<EEOK_Result> EOK_LobbySearch_RemoveParameter(FEOK_HLobbySearch LobbySearchHandle, const FString& Key, const TEnumAsByte<EEOK_EComparisonOp>& ComparisonOp);

	UFUNCTION(BlueprintCallable, Category = "Epic Online Services-Kit V2 | SDK Functions | Lobby Interface", DisplayName="EOS_LobbySearch_SetLobbyId")
	TEnumAsByte<EEOK_Result> EOK_LobbySearch_SetLobbyId(FEOK_HLobbySearch LobbySearchHandle, const FEOK_LobbyId& Options);

	UFUNCTION(BlueprintCallable, Category = "Epic Online Services-Kit V2 | SDK Functions | Lobby Interface", DisplayName="EOS_LobbySearch_SetMaxResults")
	TEnumAsByte<EEOK_Result> EOK_LobbySearch_SetMaxResults(FEOK_HLobbySearch LobbySearchHandle, const int32 MaxResults);

	UFUNCTION(BlueprintCallable, Category = "Epic Online Services-Kit V2 | SDK Functions | Lobby Interface", DisplayName="EOS_LobbySearch_SetParameter")
	TEnumAsByte<EEOK_Result> EOK_LobbySearch_SetParameter(FEOK_HLobbySearch LobbySearchHandle, const FEOK_Lobby_AttributeData& Parameter, const TEnumAsByte<EEOK_EComparisonOp>& ComparisonOp);

	UFUNCTION(BlueprintCallable, Category = "Epic Online Services-Kit V2 | SDK Functions | Lobby Interface", DisplayName="EOS_LobbySearch_SetTargetUserId")
	TEnumAsByte<EEOK_Result> EOK_LobbySearch_SetTargetUserId(FEOK_HLobbySearch LobbySearchHandle, FEOK_ProductUserId Options);
};
