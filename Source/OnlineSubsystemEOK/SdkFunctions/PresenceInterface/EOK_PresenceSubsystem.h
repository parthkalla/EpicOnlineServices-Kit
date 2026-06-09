// Copyright (c) 2023 Betide Studio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "OnlineSubsystemEOK/SdkFunctions/EOK_SharedFunctionFile.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "EOK_PresenceSubsystem.generated.h"

DECLARE_DYNAMIC_DELEGATE_FourParams(FEOK_Presence_JoinGameAcceptedCallbackInfo, const FString&, JoinInfo, const FEOK_EpicAccountId&, LocalUserId, const FEOK_EpicAccountId&, TargetUserId, const FEOK_UI_EventId&, UiEventId);
DECLARE_DYNAMIC_DELEGATE_TwoParams(FEOK_Presence_OnPresenceChangedCallbackInfo, const FEOK_EpicAccountId&, LocalUserId, const FEOK_EpicAccountId&, PresenceUserId);

UCLASS(DisplayName="Presense Interface", DisplayName="Presense Interface")
class OnlineSubsystemEOK_API UEOK_PresenceSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	
	FEOK_Presence_JoinGameAcceptedCallbackInfo JoinGameAcceptedCallbackInfo;
	//Register to receive notifications when a user accepts a join game option via the social overlay.
	UFUNCTION(BlueprintCallable, Category = "Epic Online Services-Kit V2 | SDK Functions | Presence Interface", DisplayName="EOS_Presence_AddNotifyJoinGameAccepted")
	FEOK_NotificationId EOK_Presence_AddNotifyJoinGameAccepted(const FEOK_Presence_JoinGameAcceptedCallbackInfo& Callback);


	FEOK_Presence_OnPresenceChangedCallbackInfo PresenceChangedCallbackInfo;
	//Register to receive notifications when presence changes.
	UFUNCTION(BlueprintCallable, Category = "Epic Online Services-Kit V2 | SDK Functions | Presence Interface", DisplayName="EOS_Presence_AddNotifyOnPresenceChanged")
	FEOK_NotificationId EOK_Presence_AddNotifyOnPresenceChanged(const FEOK_Presence_OnPresenceChangedCallbackInfo& Callback);

	//Get a user's cached presence object. If successful, this data must be released by calling EOS_Presence_Info_Release
	UFUNCTION(BlueprintCallable, Category = "Epic Online Services-Kit V2 | SDK Functions | Presence Interface", DisplayName="EOS_Presence_CopyPresence")
	TEnumAsByte<EEOK_Result> EOK_Presence_CopyPresence(FEOK_EpicAccountId LocalUserId, FEOK_EpicAccountId TargetUserId, FEOK_Presence_Info& OutPresence);


	//Creates a presence modification handle. This handle can used to add multiple changes to your presence that can be applied with EOS_Presence_SetPresence. The resulting handle must be released by calling EOS_PresenceModification_Release once it has been passed to EOS_Presence_SetPresence.
	UFUNCTION(BlueprintCallable, Category = "Epic Online Services-Kit V2 | SDK Functions | Presence Interface", DisplayName="EOS_Presence_CreatePresenceModification")
	TEnumAsByte<EEOK_Result> EOK_Presence_CreatePresenceModification(FEOK_EpicAccountId LocalUserId, FEOK_HPresenceModification& OutPresenceModificationHandle);

	//Gets a join info custom game-data string for a specific user. This is a helper function for reading the presence data related to how a user can be joined. Its meaning is entirely application dependent. This value will be valid only after a QueryPresence call has successfully completed.
	UFUNCTION(BlueprintCallable, Category = "Epic Online Services-Kit V2 | SDK Functions | Presence Interface", DisplayName="EOS_Presence_GetJoinInfo")
	TEnumAsByte<EEOK_Result> EOK_Presence_GetJoinInfo(FEOK_EpicAccountId LocalUserId, FEOK_EpicAccountId TargetUserId, FString& OutBuffer);

	//Check if we already have presence for a user
	UFUNCTION(BlueprintCallable, Category = "Epic Online Services-Kit V2 | SDK Functions | Presence Interface", DisplayName="EOS_Presence_HasPresence")
	bool EOK_Presence_HasPresence(FEOK_EpicAccountId LocalUserId, FEOK_EpicAccountId TargetUserId);

	//Release the memory associated with an EOS_Presence_Info structure and its sub-objects. This must be called on data retrieved from EOS_Presence_CopyPresence. This can be safely called on a NULL presence info object.
	UFUNCTION(BlueprintCallable, Category = "Epic Online Services-Kit V2 | SDK Functions | Presence Interface", DisplayName="EOS_Presence_Info_Release")
	void EOK_Presence_Info_Release(const FEOK_Presence_Info& PresenceInfo);

	//Unregister from receiving notifications when a user accepts a join game option via the social overlay.
	UFUNCTION(BlueprintCallable, Category = "Epic Online Services-Kit V2 | SDK Functions | Presence Interface", DisplayName="EOS_Presence_RemoveNotifyJoinGameAccepted")
	void EOK_Presence_RemoveNotifyJoinGameAccepted(FEOK_NotificationId InId);

	//Unregister a previously bound notification handler from receiving presence update notifications
	UFUNCTION(BlueprintCallable, Category = "Epic Online Services-Kit V2 | SDK Functions | Presence Interface", DisplayName="EOS_Presence_RemoveNotifyOnPresenceChanged")
	void EOK_Presence_RemoveNotifyOnPresenceChanged(FEOK_NotificationId InId);

	//Removes one or more rows of user-defined presence data for a local user. At least one DeleteDataInfo object must be specified.
	UFUNCTION(BlueprintCallable, Category = "Epic Online Services-Kit V2 | SDK Functions | Presence Interface", DisplayName="EOS_PresenceModification_DeleteData")
	TEnumAsByte<EEOK_Result> EOK_PresenceModification_DeleteData(FEOK_HPresenceModification PresenceModificationHandle, const FEOK_PresenceModification_DataRecordId& Data, int32 Count);

	//Release the memory associated with an EOS_HPresenceModification handle. This must be called on Handles retrieved from EOS_Presence_CreatePresenceModification. This can be safely called on a NULL presence modification handle. This also may be safely called while a call to SetPresence is still pending.
	UFUNCTION(BlueprintCallable, Category = "Epic Online Services-Kit V2 | SDK Functions | Presence Interface", DisplayName="EOS_PresenceModification_Release")
	void EOK_PresenceModification_Release(FEOK_HPresenceModification PresenceModificationHandle);

	//Modifies one or more rows of user-defined presence data for a local user. At least one InfoData object must be specified.
	UFUNCTION(BlueprintCallable, Category = "Epic Online Services-Kit V2 | SDK Functions | Presence Interface", DisplayName="EOS_PresenceModification_SetData")
	TEnumAsByte<EEOK_Result> EOK_PresenceModification_SetData(FEOK_HPresenceModification PresenceModificationHandle, const FEOK_Presence_DataRecord& Data, int32 Count);

	//Sets your new join info custom game-data string. This is a helper function for reading the presence data related to how a user can be joined. Its meaning is entirely application dependent.
	UFUNCTION(BlueprintCallable, Category = "Epic Online Services-Kit V2 | SDK Functions | Presence Interface", DisplayName="EOS_PresenceModification_SetJoinInfo")
	TEnumAsByte<EEOK_Result> EOK_PresenceModification_SetJoinInfo(FEOK_HPresenceModification PresenceModificationHandle, const FString& JoinInfo);

	//Modifies a user's Rich Presence string to a new state. This is the exact value other users will see when they query the local user's presence.
	UFUNCTION(BlueprintCallable, Category = "Epic Online Services-Kit V2 | SDK Functions | Presence Interface", DisplayName="EOS_PresenceModification_SetRawRichText")
	TEnumAsByte<EEOK_Result> EOK_PresenceModification_SetRawRichText(FEOK_HPresenceModification PresenceModificationHandle, const FString& RichText);

	//Modifies a user's online status to be the new state.
	UFUNCTION(BlueprintCallable, Category = "Epic Online Services-Kit V2 | SDK Functions | Presence Interface", DisplayName="EOS_PresenceModification_SetStatus")
	TEnumAsByte<EEOK_Result> EOK_PresenceModification_SetStatus(FEOK_HPresenceModification PresenceModificationHandle, TEnumAsByte<EEOK_Presence_EStatus> Status);
};
