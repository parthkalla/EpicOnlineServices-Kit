// Copyright (c) 2023 Betide Studio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "OnlineSubsystemEOK/SdkFunctions/EOK_SharedFunctionFile.h"
THIRD_PARTY_INCLUDES_START
#include "eos_auth.h"
THIRD_PARTY_INCLUDES_END
#include "Subsystems/GameInstanceSubsystem.h"
#include "EOK_AuthSubsystem.generated.h"

DECLARE_DYNAMIC_DELEGATE_ThreeParams(FEOK_Auth_OnLoginStatusChangedCallback, FEOK_EpicAccountId, LocalUserId, const TEnumAsByte<EOK_ELoginStatus>&, PrevStatus, const TEnumAsByte<EOK_ELoginStatus>&, CurrentStatus);
UCLASS(meta=(DisplayName="Auth Interface"), Category="Epic Online Services-Kit V2", DisplayName="Auth Interface")
class OnlineSubsystemEOK_API UEOK_AuthSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

	FEOK_Auth_OnLoginStatusChangedCallback OnLoginStatusChanged;
	/*
	 Register to receive login status updates.
	 *
	 If the returned NotificationId is valid, you must call EOS_Auth_RemoveNotifyLoginStatusChanged when you no longer wish to have your NotificationHandler called.
	 */
	UFUNCTION(BlueprintCallable, Category = "Epic Online Services-Kit V2 | SDK Functions | Auth Interface", DisplayName="EOS_Auth_AddNotifyLoginStatusChanged")
	FEOK_NotificationId EOK_Auth_AddNotifyLoginStatusChanged(const FEOK_Auth_OnLoginStatusChangedCallback& Callback);

	//Fetch an ID token for an Epic Account ID. ID tokens are used to securely verify user identities with online services. The most common use case is using an ID token to authenticate the local user by their selected account ID, which is the account ID that should be used to access any game-scoped data for the current application. An ID token for the selected account ID of a locally authenticated user will always be readily available. To retrieve it for the selected account ID, you can use EOS_Auth_CopyIdToken directly after a successful user login.
	UFUNCTION(BlueprintCallable, Category = "Epic Online Services-Kit V2 | SDK Functions | Auth Interface", DisplayName="EOS_Auth_CopyIdToken")
	TEnumAsByte<EEOK_Result> EOK_Auth_CopyIdToken(FEOK_EpicAccountId AccountId, FEOK_Auth_IdToken& OutToken);

	//Fetch a user auth token for an Epic Account ID. A user authentication token allows any code with possession (backend/client) to perform certain actions on behalf of the user. Because of this, for the purposes of user identity verification, the EOS_Auth_CopyIdToken API should be used instead.
	UFUNCTION(BlueprintCallable, Category = "Epic Online Services-Kit V2 | SDK Functions | Auth Interface", DisplayName="EOS_Auth_CopyUserAuthToken")
	TEnumAsByte<EEOK_Result> EOK_Auth_CopyUserAuthToken(FEOK_EpicAccountId LocalUserId, FEOK_Auth_Token& OutToken);

	//Fetch an Epic Account ID that is logged in.
	UFUNCTION(BlueprintCallable, Category = "Epic Online Services-Kit V2 | SDK Functions | Auth Interface", DisplayName="EOS_Auth_GetLoggedInAccountByIndex")
	FEOK_EpicAccountId EOK_Auth_GetLoggedInAccountByIndex(int32 Index);

	//Fetch the number of accounts that are logged in.
	UFUNCTION(BlueprintCallable, Category = "Epic Online Services-Kit V2 | SDK Functions | Auth Interface", DisplayName="EOS_Auth_GetLoggedInAccountsCount")
	int32 EOK_Auth_GetLoggedInAccountsCount();
	
	//Fetches the login status for an Epic Account ID.
	UFUNCTION(BlueprintCallable, Category = "Epic Online Services-Kit V2 | SDK Functions | Auth Interface", DisplayName="EOS_Auth_GetLoginStatus")
	TEnumAsByte<EOK_ELoginStatus> EOK_Auth_GetLoginStatus(FEOK_EpicAccountId LocalUserId);

	//Fetch one of the merged account IDs for a given logged in account.
	UFUNCTION(BlueprintCallable, Category = "Epic Online Services-Kit V2 | SDK Functions | Auth Interface", DisplayName="EOS_Auth_GetMergedAccountByIndex")
	FEOK_EpicAccountId EOK_Auth_GetMergedAccountByIndex(FEOK_EpicAccountId LocalUserId, int32 Index);

	//Fetch the number of merged accounts for a given logged in account.
	UFUNCTION(BlueprintCallable, Category = "Epic Online Services-Kit V2 | SDK Functions | Auth Interface", DisplayName="EOS_Auth_GetMergedAccountsCount")
	int32 EOK_Auth_GetMergedAccountsCount(FEOK_EpicAccountId LocalUserId);

	//Fetch the selected account ID to the current application for a local authenticated user.
	UFUNCTION(BlueprintCallable, Category = "Epic Online Services-Kit V2 | SDK Functions | Auth Interface", DisplayName="EOS_Auth_GetSelectedAccountId")
	TEnumAsByte<EEOK_Result> EOK_Auth_GetSelectedAccountId(FEOK_EpicAccountId LocalUserId, FEOK_EpicAccountId& OutSelectedAccountId);

	//Release the memory associated with an EOS_Auth_IdToken structure. This must be called on data retrieved from EOS_Auth_CopyIdToken.
	UFUNCTION(BlueprintCallable, Category = "Epic Online Services-Kit V2 | SDK Functions | Auth Interface", DisplayName="EOS_Auth_IdToken_Release")
	void EOK_Auth_IdToken_Release(FEOK_Auth_IdToken& Token);

	//Unregister from receiving login status updates.
	UFUNCTION(BlueprintCallable, Category = "Epic Online Services-Kit V2 | SDK Functions | Auth Interface", DisplayName="EOS_Auth_RemoveNotifyLoginStatusChanged")
	void EOK_Auth_RemoveNotifyLoginStatusChanged(FEOK_NotificationId NotificationId);

	//Release the memory associated with an EOS_Auth_Token structure. This must be called on data retrieved from EOS_Auth_CopyUserAuthToken.
	UFUNCTION(BlueprintCallable, Category = "Epic Online Services-Kit V2 | SDK Functions | Auth Interface", DisplayName="EOS_Auth_Token_Release")
	void EOK_Auth_Token_Release(FEOK_Auth_Token& Token);
};
