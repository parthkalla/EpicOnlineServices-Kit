// Copyright (c) 2023 Betide Studio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "OnlineSubsystemEOS.h"
#include "OnlineSubsystemEOK/SdkFunctions/EOK_SharedFunctionFile.h"
THIRD_PARTY_INCLUDES_START
#include "eos_sessions.h"
#include "eos_sessions_types.h"
THIRD_PARTY_INCLUDES_END
#include "EOK_SessionsSubsystem.generated.h"

DECLARE_DYNAMIC_DELEGATE_TwoParams(FEOK_Sessions_OnJoinSessionAcceptedCallback, const FEOK_ProductUserId&, LocalUserId, const FEOK_UI_EventId&, UIEventId);
DECLARE_DYNAMIC_DELEGATE_TwoParams(FEOK_Sessions_OnLeaveSessionRequestedCallback, const FEOK_ProductUserId&, LocalUserId, const FString&, SessionName);
DECLARE_DYNAMIC_DELEGATE_FiveParams(FEOK_Sessions_OnSendSessionInviteCallback, const FEOK_ProductUserId&, LocalUserId, const FEOK_UI_EventId&, UIEventId, const FString&, TargetNativeAccountType, const FString&, TargetUserNativeAccountId, const FString&, SessionId);
DECLARE_DYNAMIC_DELEGATE_FourParams(FEOK_Sessions_OnSessionInviteAcceptedCallback, const FEOK_ProductUserId&, LocalUserId, const FString&, SessionId, const FEOK_ProductUserId&, TargetUserId, const FString&, InviteId);
DECLARE_DYNAMIC_DELEGATE_ThreeParams(FEOK_Sessions_OnSessionInviteReceivedCallback, const FEOK_ProductUserId&, LocalUserId, const FString&, InviteId, const FEOK_ProductUserId&, TargetUserId);
DECLARE_DYNAMIC_DELEGATE_FourParams(FEOK_Sessions_OnSessionInviteRejectedCallback, const FEOK_ProductUserId&, LocalUserId, const FString&, SessionId, const FEOK_ProductUserId&, TargetUserId, const FString&, InviteId);

USTRUCT(BlueprintType)
struct FEOK_Sessions_CreateSessionModificationOptions
{
	GENERATED_BODY()

	//Name of the session to create
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Epic Online Services-Kit V2 | Sessions Interface")
	FString SessionName;

	//Bucket ID associated with the session
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Epic Online Services-Kit V2 | Sessions Interface")
	FString BucketId;

	//Maximum number of players allowed in the session
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Epic Online Services-Kit V2 | Sessions Interface")
	int32 MaxPlayers;

	//The Product User ID of the local user associated with the session
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Epic Online Services-Kit V2 | Sessions Interface")
	FEOK_ProductUserId LocalUserId;

	//Determines whether or not this session should be the one associated with the local user's presence information. If true, this session will be associated with presence. Only one session at a time can have this flag true. This affects the ability of the Social Overlay to show game related actions to take in the user's social graph. * using the bPresenceEnabled flags within the Sessions interface * using the bPresenceEnabled flags within the Lobby interface * using EOS_PresenceModification_SetJoinInfo
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Epic Online Services-Kit V2 | Sessions Interface")
	bool bPresenceEnabled;

	//Optional session id - set to a globally unique value to override the backend assignment If not specified the backend service will assign one to the session. Do not mix and match. This value can be of size [EOS_SESSIONMODIFICATION_MIN_SESSIONIDOVERRIDE_LENGTH, EOS_SESSIONMODIFICATION_MAX_SESSIONIDOVERRIDE_LENGTH]
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Epic Online Services-Kit V2 | Sessions Interface")
	FString SessionId;

	//If true, sanctioned players can neither join nor register with this session and, in the case of join, will return EOS_EResult code EOS_Sessions_PlayerSanctioned
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Epic Online Services-Kit V2 | Sessions Interface")
	bool bSanctionsEnabled;

	//Array of platform IDs indicating the player platforms allowed to register with the session. Platform IDs are found in the EOS header file, e.g. EOS_OPT_Epic. For some platforms, the value will be in the EOS Platform specific header file. If null, the session will be unrestricted.
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Epic Online Services-Kit V2 | Sessions Interface")
	TArray<int32> AllowedPlatformIds;

	FEOK_Sessions_CreateSessionModificationOptions()
	{
		SessionName = "";
		BucketId = "";
		MaxPlayers = 0;
		LocalUserId = FEOK_ProductUserId();
		bPresenceEnabled = false;
		SessionId = "";
		bSanctionsEnabled = false;
	}
	EOS_Sessions_CreateSessionModificationOptions ToEosStruct()
	{
		EOS_Sessions_CreateSessionModificationOptions EosStruct;
		EosStruct.ApiVersion = EOS_SESSIONS_CREATESESSIONMODIFICATION_API_LATEST;
		EosStruct.SessionName = TCHAR_TO_ANSI(*SessionName);
		EosStruct.BucketId = TCHAR_TO_ANSI(*BucketId);
		EosStruct.MaxPlayers = MaxPlayers;
		EosStruct.LocalUserId = LocalUserId.GetValueAsEosType();
		EosStruct.bPresenceEnabled = bPresenceEnabled;
		EosStruct.SessionId = TCHAR_TO_ANSI(*SessionId);
		EosStruct.bSanctionsEnabled = bSanctionsEnabled;
		EosStruct.AllowedPlatformIdsCount = AllowedPlatformIds.Num();
		uint32_t* AllowedPlatformIdsArray = new uint32_t[AllowedPlatformIds.Num()];
		for (int i = 0; i < AllowedPlatformIds.Num(); i++)
		{
			AllowedPlatformIdsArray[i] = AllowedPlatformIds[i];
		}
		EosStruct.AllowedPlatformIds = AllowedPlatformIdsArray;
		return EosStruct;
	}
};
UCLASS()
class OnlineSubsystemEOK_API UEOK_SessionsSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:

	//EOS_ActiveSession_CopyInfo is used to immediately retrieve a copy of active session information If the call returns an EOS_Success result, the out parameter, OutActiveSessionInfo, must be passed to EOS_ActiveSession_Info_Release to release the memory associated with it.
	UFUNCTION(BlueprintCallable, Category = "Epic Online Services-Kit V2 | SDK Functions | Sessions Interface", DisplayName="EOS_ActiveSession_CopyInfo")
	static TEnumAsByte<EEOK_Result> EOK_ActiveSession_CopyInfo(FEOK_HActiveSession Handle, FEOK_ActiveSession_Info& OutActiveSessionInfo);

	//EOS_ActiveSession_GetRegisteredPlayerByIndex is used to immediately retrieve individual players registered with the active session.
	UFUNCTION(BlueprintCallable, Category = "Epic Online Services-Kit V2 | SDK Functions | Sessions Interface", DisplayName="EOS_ActiveSession_GetRegisteredPlayerByIndex")
	static FEOK_ProductUserId EOK_ActiveSession_GetRegisteredPlayerByIndex(FEOK_HActiveSession Handle, int32 PlayerIndex);

	//Release the memory associated with an active session. This must be called on data retrieved from EOS_Sessions_CopyActiveSessionHandle
	UFUNCTION(BlueprintCallable, Category = "Epic Online Services-Kit V2 | SDK Functions | Sessions Interface", DisplayName="EOS_ActiveSession_Release")
	static void EOK_ActiveSession_Release(FEOK_HActiveSession Handle);

	//EOS_SessionDetails_CopyInfo is used to immediately retrieve a copy of session information from a given source such as a active session or a search result. If the call returns an EOS_Success result, the out parameter, OutSessionInfo, must be passed to EOS_SessionDetails_Info_Release to release the memory associated with it.
	UFUNCTION(BlueprintCallable, Category = "Epic Online Services-Kit V2 | SDK Functions | Sessions Interface", DisplayName="EOS_SessionDetails_CopyInfo")
	static TEnumAsByte<EEOK_Result> EOK_SessionDetails_CopyInfo(FEOK_HSessionDetails Handle, FEOK_SessionDetails_Info& OutSessionInfo);

	//EOS_SessionDetails_CopySessionAttributeByIndex is used to immediately retrieve a copy of session attribution from a given source such as a active session or a search result. If the call returns an EOS_Success result, the out parameter, OutSessionAttribute, must be passed to EOS_SessionDetails_Attribute_Release to release the memory associated with it.
	UFUNCTION(BlueprintCallable, Category = "Epic Online Services-Kit V2 | SDK Functions | Sessions Interface", DisplayName="EOS_SessionDetails_CopySessionAttributeByIndex")
	static TEnumAsByte<EEOK_Result> EOK_SessionDetails_CopySessionAttributeByIndex(FEOK_HSessionDetails Handle, int32 AttrIndex, FEOK_SessionDetails_Attribute& OutSessionAttribute);

	//EOS_SessionDetails_CopySessionAttributeByKey is used to immediately retrieve a copy of session attribution from a given source such as a active session or a search result. If the call returns an EOS_Success result, the out parameter, OutSessionAttribute, must be passed to EOS_SessionDetails_Attribute_Release to release the memory associated with it.
	UFUNCTION(BlueprintCallable, Category = "Epic Online Services-Kit V2 | SDK Functions | Sessions Interface", DisplayName="EOS_SessionDetails_CopySessionAttributeByKey")
	static TEnumAsByte<EEOK_Result> EOK_SessionDetails_CopySessionAttributeByKey(FEOK_HSessionDetails Handle, const FString& AttrKey, FEOK_SessionDetails_Attribute& OutSessionAttribute);

	//Get the number of attributes associated with this session
	UFUNCTION(BlueprintCallable, Category = "Epic Online Services-Kit V2 | SDK Functions | Sessions Interface", DisplayName="EOS_SessionDetails_GetSessionAttributeCount")
	static int32 EOK_SessionDetails_GetSessionAttributeCount(FEOK_HSessionDetails Handle);

	//Associate an attribute with this session An attribute is something that may or may not be advertised with the session. If advertised, it can be queried for in a search, otherwise the data remains local to the client
	UFUNCTION(BlueprintCallable, Category = "Epic Online Services-Kit V2 | SDK Functions | Sessions Interface", DisplayName="EOS_SessionModification_AddAttribute")
	static TEnumAsByte<EEOK_Result> EOK_SessionModification_AddAttribute(FEOK_HSessionModification Handle, FEOK_Sessions_AttributeData AttrData, TEnumAsByte<EOK_ESessionAttributeAdvertisementType> AdvertisementType);

	//Release the memory associated with session modification. This must be called on data retrieved from EOS_Sessions_CreateSessionModification or EOS_Sessions_UpdateSessionModification
	UFUNCTION(BlueprintCallable, Category = "Epic Online Services-Kit V2 | SDK Functions | Sessions Interface", DisplayName="EOS_SessionModification_Release")
	static void EOK_SessionModification_Release(FEOK_HSessionModification Handle);

	//Remove an attribute from this session
	UFUNCTION(BlueprintCallable, Category = "Epic Online Services-Kit V2 | SDK Functions | Sessions Interface", DisplayName="EOS_SessionModification_RemoveAttribute")
	static TEnumAsByte<EEOK_Result> EOK_SessionModification_RemoveAttribute(FEOK_HSessionModification Handle, const FString& Key);

	//Set the Allowed Platform IDs for the session.
	UFUNCTION(BlueprintCallable, Category = "Epic Online Services-Kit V2 | SDK Functions | Sessions Interface", DisplayName="EOS_SessionModification_SetAllowedPlatformIds")
	static TEnumAsByte<EEOK_Result> EOK_SessionModification_SetAllowedPlatformIds(FEOK_HSessionModification Handle, const TArray<int32>& PlatformIds);

	//Set the bucket ID associated with this session. Values such as region, game mode, etc can be combined here depending on game need. Setting this is strongly recommended to improve search performance.
	UFUNCTION(BlueprintCallable, Category = "Epic Online Services-Kit V2 | SDK Functions | Sessions Interface", DisplayName="EOS_SessionModification_SetBucketId")
	static TEnumAsByte<EEOK_Result> EOK_SessionModification_SetBucketId(FEOK_HSessionModification Handle, const FString& BucketId);

	//Set the host address associated with this session Setting this is optional, if the value is not set the SDK will fill the value in from the service. It is useful to set if other addressing mechanisms are desired or if LAN addresses are preferred during development
	UFUNCTION(BlueprintCallable, Category = "Epic Online Services-Kit V2 | SDK Functions | Sessions Interface", DisplayName="EOS_SessionModification_SetHostAddress")
	static TEnumAsByte<EEOK_Result> EOK_SessionModification_SetHostAddress(FEOK_HSessionModification Handle, const FString& HostAddress);

	//Allows enabling or disabling invites for this session. The session will also need to have bPresenceEnabled true.
	UFUNCTION(BlueprintCallable, Category = "Epic Online Services-Kit V2 | SDK Functions | Sessions Interface", DisplayName="EOS_SessionModification_SetInvitesAllowed")
	static TEnumAsByte<EEOK_Result> EOK_SessionModification_SetInvitesAllowed(FEOK_HSessionModification Handle, bool bInvitesAllowed);

	//Set whether or not join in progress is allowed Once a session is started, it will no longer be visible to search queries unless this flag is set or the session returns to the pending or ended state
	UFUNCTION(BlueprintCallable, Category = "Epic Online Services-Kit V2 | SDK Functions | Sessions Interface", DisplayName="EOS_SessionModification_SetJoinInProgressAllowed")
	static TEnumAsByte<EEOK_Result> EOK_SessionModification_SetJoinInProgressAllowed(FEOK_HSessionModification Handle, bool bAllowJoinInProgress);

	//Set the maximum number of players allowed in this session. When updating the session, it is not possible to reduce this number below the current number of existing players
	UFUNCTION(BlueprintCallable, Category = "Epic Online Services-Kit V2 | SDK Functions | Sessions Interface", DisplayName="EOS_SessionModification_SetMaxPlayers")
	static TEnumAsByte<EEOK_Result> EOK_SessionModification_SetMaxPlayers(FEOK_HSessionModification Handle, int32 MaxPlayers);

	//Set the session permissions associated with this session. The permissions range from "public" to "invite only" and are described by EOS_EOnlineSessionPermissionLevel
	UFUNCTION(BlueprintCallable, Category = "Epic Online Services-Kit V2 | SDK Functions | Sessions Interface", DisplayName="EOS_SessionModification_SetPermissionLevel")
	static TEnumAsByte<EEOK_Result> EOK_SessionModification_SetPermissionLevel(FEOK_HSessionModification Handle, TEnumAsByte<EEOK_EOnlineSessionPermissionLevel> PermissionLevel);

	FEOK_Sessions_OnJoinSessionAcceptedCallback OnJoinSessionAcceptedCallback;
	//Register to receive notifications when a user accepts a session join game via the social overlay.
	UFUNCTION(BlueprintCallable, Category = "Epic Online Services-Kit V2 | SDK Functions | Sessions Interface", DisplayName="EOS_Sessions_AddNotifyJoinSessionAccepted")
	FEOK_NotificationId EOK_Sessions_AddNotifyJoinSessionAccepted( const FEOK_Sessions_OnJoinSessionAcceptedCallback& Callback);

	FEOK_Sessions_OnLeaveSessionRequestedCallback OnLeaveSessionRequestedCallback;
	//Register to receive notifications about leave session requests performed by local user via the overlay. When user requests to leave the session in the social overlay, the SDK does not automatically leave the session, it is up to the game to perform any necessary cleanup and call the EOS_Sessions_DestroySession method using the SessionName sent in the notification function.
	UFUNCTION(BlueprintCallable, Category = "Epic Online Services-Kit V2 | SDK Functions | Sessions Interface", DisplayName="EOS_Sessions_AddNotifyLeaveSessionRequested")
	FEOK_NotificationId EOK_Sessions_AddNotifyLeaveSessionRequested( const FEOK_Sessions_OnLeaveSessionRequestedCallback& Callback);


	FEOK_Sessions_OnSendSessionInviteCallback OnSendSessionInviteCallback;
	//Register to receive notifications about a session "INVITE" performed by a local user via the overlay. This is only needed when a configured integrated platform has EOS_IPMF_DisableSDKManagedSessions set. The EOS SDK will then use the state of EOS_IPMF_PreferEOSIdentity and EOS_IPMF_PreferIntegratedIdentity to determine when the NotificationFn is called.
	UFUNCTION(BlueprintCallable, Category = "Epic Online Services-Kit V2 | SDK Functions | Sessions Interface", DisplayName="EOS_Sessions_AddNotifySendSessionNativeInviteRequested")
	FEOK_NotificationId EOK_Sessions_AddNotifySendSessionNativeInviteRequested( const FEOK_Sessions_OnSendSessionInviteCallback& Callback);

	FEOK_Sessions_OnSessionInviteAcceptedCallback OnSessionInviteAcceptedCallback;
	//Register to receive notifications when a user accepts a session invite via the social overlay.
	UFUNCTION(BlueprintCallable, Category = "Epic Online Services-Kit V2 | SDK Functions | Sessions Interface", DisplayName="EOS_Sessions_AddNotifySessionInviteAccepted")
	FEOK_NotificationId EOK_Sessions_AddNotifySessionInviteAccepted( const FEOK_Sessions_OnSessionInviteAcceptedCallback& Callback);

	FEOK_Sessions_OnSessionInviteReceivedCallback OnSessionInviteReceivedCallback;
	//Register to receive session invites.
	UFUNCTION(BlueprintCallable, Category = "Epic Online Services-Kit V2 | SDK Functions | Sessions Interface", DisplayName="EOS_Sessions_AddNotifySessionInviteReceived")
	FEOK_NotificationId EOK_Sessions_AddNotifySessionInviteReceived( const FEOK_Sessions_OnSessionInviteReceivedCallback& Callback);

	FEOK_Sessions_OnSessionInviteRejectedCallback OnSessionInviteRejectedCallback;
	//Register to receive notifications when a user rejects a session invite.
	UFUNCTION(BlueprintCallable, Category = "Epic Online Services-Kit V2 | SDK Functions | Sessions Interface", DisplayName="EOS_Sessions_AddNotifySessionInviteRejected")
	FEOK_NotificationId EOK_Sessions_AddNotifySessionInviteRejected( const FEOK_Sessions_OnSessionInviteRejectedCallback& Callback);


	//Create a handle to an existing active session.
	UFUNCTION(BlueprintCallable, Category = "Epic Online Services-Kit V2 | SDK Functions | Sessions Interface", DisplayName="EOS_Sessions_CopyActiveSessionHandle")
	static TEnumAsByte<EEOK_Result> EOK_Sessions_CopyActiveSessionHandle(FString SessionName, FEOK_HActiveSession& OutActiveSessionHandle);

	//EOS_Sessions_CopySessionHandleByInviteId is used to immediately retrieve a handle to the session information from after notification of an invite If the call returns an EOS_Success result, the out parameter, OutSessionHandle, must be passed to EOS_SessionDetails_Release to release the memory associated with it.
	UFUNCTION(BlueprintCallable, Category = "Epic Online Services-Kit V2 | SDK Functions | Sessions Interface", DisplayName="EOS_Sessions_CopySessionHandleByInviteId")
	static TEnumAsByte<EEOK_Result> EOK_Sessions_CopySessionHandleByInviteId(FString InviteId, FEOK_HSessionDetails& OutSessionHandle);

	//EOS_Sessions_CopySessionHandleByUiEventId is used to immediately retrieve a handle to the session information from after notification of a join game event. If the call returns an EOS_Success result, the out parameter, OutSessionHandle, must be passed to EOS_SessionDetails_Release to release the memory associated with it.
	UFUNCTION(BlueprintCallable, Category = "Epic Online Services-Kit V2 | SDK Functions | Sessions Interface", DisplayName="EOS_Sessions_CopySessionHandleByUiEventId")
	static TEnumAsByte<EEOK_Result> EOK_Sessions_CopySessionHandleByUiEventId(FEOK_UI_EventId UiEventId, FEOK_HSessionDetails& OutSessionHandle);

	//EOS_Sessions_CopySessionHandleForPresence is used to immediately retrieve a handle to the session information which was marked with bPresenceEnabled on create or join. If the call returns an EOS_Success result, the out parameter, OutSessionHandle, must be passed to EOS_SessionDetails_Release to release the memory associated with it.
	UFUNCTION(BlueprintCallable, Category = "Epic Online Services-Kit V2 | SDK Functions | Sessions Interface", DisplayName="EOS_Sessions_CopySessionHandleForPresence")
	static TEnumAsByte<EEOK_Result> EOK_Sessions_CopySessionHandleForPresence(FEOK_ProductUserId LocalUserId, FEOK_HSessionDetails& OutSessionHandle);

	//Creates a session modification handle (EOS_HSessionModification). The session modification handle is used to build a new session and can be applied with EOS_Sessions_UpdateSession The EOS_HSessionModification must be released by calling EOS_SessionModification_Release once it no longer needed.
	UFUNCTION(BlueprintCallable, Category = "Epic Online Services-Kit V2 | SDK Functions | Sessions Interface", DisplayName="EOS_Sessions_CreateSessionModification")
	static TEnumAsByte<EEOK_Result> EOK_Sessions_CreateSessionModification(FEOK_Sessions_CreateSessionModificationOptions Options, FEOK_HSessionModification& OutSessionModificationHandle);

	/*
	*Create a session search handle. This handle may be modified to include various search parameters. Searching is possible in three methods, all mutually exclusive

		set the session ID to find a specific session
		set the target user ID to find a specific user
		set session parameters to find an array of sessions that match the search criteria
	 */
	UFUNCTION(BlueprintCallable, Category = "Epic Online Services-Kit V2 | SDK Functions | Sessions Interface", DisplayName="EOS_Sessions_CreateSessionSearch")
	static TEnumAsByte<EEOK_Result> EOK_Sessions_CreateSessionSearch(int32 MaxSearchResults, FEOK_HSessionSearch& OutSessionSearchHandle);

	//Dump the contents of active sessions that exist locally to the log output, purely for debug purposes
	UFUNCTION(BlueprintCallable, Category = "Epic Online Services-Kit V2 | SDK Functions | Sessions Interface", DisplayName="EOS_Sessions_DumpSessionState")
	static void EOK_Sessions_DumpSessionState(FString SessionName);

	//Get the number of known invites for a given user
	UFUNCTION(BlueprintCallable, Category = "Epic Online Services-Kit V2 | SDK Functions | Sessions Interface", DisplayName="EOS_Sessions_GetInviteCount")
	static int32 EOK_Sessions_GetInviteCount(FEOK_ProductUserId LocalUserId);

	//Retrieve an invite ID from a list of active invites for a given user
	UFUNCTION(BlueprintCallable, Category = "Epic Online Services-Kit V2 | SDK Functions | Sessions Interface", DisplayName="EOS_Sessions_GetInviteIdByIndex")
	static FString EOK_Sessions_GetInviteIdByIndex(FEOK_ProductUserId LocalUserId, int32 Index);

	//EOS_Sessions_IsUserInSession returns whether or not a given user can be found in a specified session
	UFUNCTION(BlueprintCallable, Category = "Epic Online Services-Kit V2 | SDK Functions | Sessions Interface", DisplayName="EOS_Sessions_IsUserInSession")
	static TEnumAsByte<EEOK_Result> EOK_Sessions_IsUserInSession(FEOK_ProductUserId TargetUserId, FString SessionName);

	//Unregister from receiving notifications when a user accepts a session join game via the social overlay.
	UFUNCTION(BlueprintCallable, Category = "Epic Online Services-Kit V2 | SDK Functions | Sessions Interface", DisplayName="EOS_Sessions_RemoveNotifyJoinSessionAccepted")
	static void EOK_Sessions_RemoveNotifyJoinSessionAccepted(FEOK_NotificationId InId);

	//Unregister from receiving notifications when a user performs a leave lobby action via the overlay.
	UFUNCTION(BlueprintCallable, Category = "Epic Online Services-Kit V2 | SDK Functions | Sessions Interface", DisplayName="EOS_Sessions_RemoveNotifyLeaveSessionRequested")
	static void EOK_Sessions_RemoveNotifyLeaveSessionRequested(FEOK_NotificationId InId);

	//Unregister from receiving notifications when a user requests a send invite via the overlay.
	UFUNCTION(BlueprintCallable, Category = "Epic Online Services-Kit V2 | SDK Functions | Sessions Interface", DisplayName="EOS_Sessions_RemoveNotifySendSessionNativeInviteRequested")
	static void EOK_Sessions_RemoveNotifySendSessionNativeInviteRequested(FEOK_NotificationId InId);

	//Unregister from receiving notifications when a user accepts a session invite via the social overlay.
	UFUNCTION(BlueprintCallable, Category = "Epic Online Services-Kit V2 | SDK Functions | Sessions Interface", DisplayName="EOS_Sessions_RemoveNotifySessionInviteAccepted")
	static void EOK_Sessions_RemoveNotifySessionInviteAccepted(FEOK_NotificationId InId);

	//Unregister from receiving session invites.
	UFUNCTION(BlueprintCallable, Category = "Epic Online Services-Kit V2 | SDK Functions | Sessions Interface", DisplayName="EOS_Sessions_RemoveNotifySessionInviteReceived")
	static void EOK_Sessions_RemoveNotifySessionInviteReceived(FEOK_NotificationId InId);

	//Unregister from receiving notifications when a user rejects a session invite via the social overlay.
	UFUNCTION(BlueprintCallable, Category = "Epic Online Services-Kit V2 | SDK Functions | Sessions Interface", DisplayName="EOS_Sessions_RemoveNotifySessionInviteRejected")
	static void EOK_Sessions_RemoveNotifySessionInviteRejected(FEOK_NotificationId InId);

	//Creates a session modification handle (EOS_HSessionModification). The session modification handle is used to modify an existing session and can be applied with EOS_Sessions_UpdateSession. The EOS_HSessionModification must be released by calling EOS_SessionModification_Release once it is no longer needed.
	UFUNCTION(BlueprintCallable, Category = "Epic Online Services-Kit V2 | SDK Functions | Sessions Interface", DisplayName="EOS_Sessions_UpdateSessionModification")
	static TEnumAsByte<EEOK_Result> EOK_Sessions_UpdateSessionModification(FString SessionName, FEOK_HSessionModification& OutSessionModificationHandle);

	//EOS_SessionSearch_CopySearchResultByIndex is used to immediately retrieve a handle to the session information from a given search result. If the call returns an EOS_Success result, the out parameter, OutSessionHandle, must be passed to EOS_SessionDetails_Release to release the memory associated with it.
	UFUNCTION(BlueprintCallable, Category = "Epic Online Services-Kit V2 | SDK Functions | Sessions Interface", DisplayName="EOS_SessionSearch_CopySearchResultByIndex")
	static TEnumAsByte<EEOK_Result> EOK_SessionSearch_CopySearchResultByIndex(FEOK_HSessionSearch Handle, int32 SessionIndex, FEOK_HSessionDetails& OutSessionHandle);

	//Get the number of search results found by the search parameters in this search
	UFUNCTION(BlueprintCallable, Category = "Epic Online Services-Kit V2 | SDK Functions | Sessions Interface", DisplayName="EOS_SessionSearch_GetSearchResultCount")
	static int32 EOK_SessionSearch_GetSearchResultCount(FEOK_HSessionSearch Handle);

	//Release the memory associated with a session search. This must be called on data retrieved from EOS_Sessions_CreateSessionSearch.
	UFUNCTION(BlueprintCallable, Category = "Epic Online Services-Kit V2 | SDK Functions | Sessions Interface", DisplayName="EOS_SessionSearch_Release")
	static void EOK_SessionSearch_Release(FEOK_HSessionSearch Handle);

	//Remove a parameter from the array of search criteria.
	UFUNCTION(BlueprintCallable, Category = "Epic Online Services-Kit V2 | SDK Functions | Sessions Interface", DisplayName="EOS_SessionSearch_RemoveParameter")
	static TEnumAsByte<EEOK_Result> EOK_SessionSearch_RemoveParameter(FEOK_HSessionSearch Handle, const FString& Key, const TEnumAsByte<EEOK_EComparisonOp>& ComparisonOp);

	//Set the maximum number of search results to return in the query, can't be more than EOS_SESSIONS_MAX_SEARCH_RESULTS
	UFUNCTION(BlueprintCallable, Category = "Epic Online Services-Kit V2 | SDK Functions | Sessions Interface", DisplayName="EOS_SessionSearch_SetMaxResults")
	static TEnumAsByte<EEOK_Result> EOK_SessionSearch_SetMaxResults(FEOK_HSessionSearch Handle, int32 MaxSearchResults);

	//Add a parameter to an array of search criteria combined via an implicit AND operator. Setting SessionId or TargetUserId will result in EOS_SessionSearch_Find failing
	UFUNCTION(BlueprintCallable, Category = "Epic Online Services-Kit V2 | SDK Functions | Sessions Interface", DisplayName="EOS_SessionSearch_SetParameter")
	static TEnumAsByte<EEOK_Result> EOK_SessionSearch_SetParameter(FEOK_HSessionSearch Handle, FEOK_Sessions_AttributeData Parameter, const TEnumAsByte<EEOK_EComparisonOp>& ComparisonOp);

	//Set a session ID to find and will return at most one search result. Setting TargetUserId or SearchParameters will result in EOS_SessionSearch_Find failing
	UFUNCTION(BlueprintCallable, Category = "Epic Online Services-Kit V2 | SDK Functions | Sessions Interface", DisplayName="EOS_SessionSearch_SetSessionId")
	static TEnumAsByte<EEOK_Result> EOK_SessionSearch_SetSessionId(FEOK_HSessionSearch Handle, const FString& SessionId);

	//Set a target user ID to find and will return at most one search result. Setting SessionId or SearchParameters will result in EOS_SessionSearch_Find failing
	UFUNCTION(BlueprintCallable, Category = "Epic Online Services-Kit V2 | SDK Functions | Sessions Interface", DisplayName="EOS_SessionSearch_SetTargetUserId")
	static TEnumAsByte<EEOK_Result> EOK_SessionSearch_SetTargetUserId(FEOK_HSessionSearch Handle, FEOK_ProductUserId TargetUserId);
	
};
