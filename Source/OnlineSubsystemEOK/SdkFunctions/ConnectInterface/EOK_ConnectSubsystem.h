// Copyright (c) 2023 Betide Studio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "OnlineSubsystemEOK/AsyncFunctions/Extra/EOK_BlueprintFunctions.h"
#include "OnlineSubsystemEOK/SdkFunctions/EOK_SharedFunctionFile.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "EOK_ConnectSubsystem.generated.h"

DECLARE_DYNAMIC_DELEGATE_OneParam(FOnAuthExpirationCallback, FEOK_ProductUserId, LocalUserId);
DECLARE_DYNAMIC_DELEGATE_TwoParams(FOnLoginStatusChangedCallback, FEOK_ProductUserId, LocalUserId, const TEnumAsByte<EOK_ELoginStatus>&, LoginStatus);

UCLASS()
class OnlineSubsystemEOK_API UEOK_ConnectSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:

	FOnAuthExpirationCallback OnAuthExpiration;
	FOnLoginStatusChangedCallback OnLoginStatusChanged;

	//Register to receive upcoming authentication expiration notifications. Notification is approximately 10 minutes prior to expiration. Call EOS_Connect_Login again with valid third party credentials to refresh access.
	UFUNCTION(BlueprintCallable, Category = "Epic Online Services-Kit V2 | SDK Functions | Connect Interface", DisplayName="EOS_Connect_AddNotifyAuthExpiration")
	FEOK_NotificationId EOK_Connect_AddNotifyAuthExpiration(const FOnAuthExpirationCallback& Callback);

	//Register to receive user login status updates.
	UFUNCTION(BlueprintCallable, Category = "Epic Online Services-Kit V2 | SDK Functions | Connect Interface", DisplayName="EOS_Connect_AddNotifyLoginStatusChanged")
	FEOK_NotificationId EOK_Connect_AddNotifyLoginStatusChanged(const FOnLoginStatusChangedCallback& Callback);

	//Fetches an ID token for a Product User ID.
	UFUNCTION(BlueprintCallable, Category = "Epic Online Services-Kit V2 | SDK Functions | Connect Interface", DisplayName="EOS_Connect_CopyIdToken")
	TEnumAsByte<EEOK_Result> EOK_Connect_CopyIdToken(FEOK_ProductUserId LocalUserId, FEOK_Connect_IdToken& OutIdToken);

	//Fetch information about an external account linked to a Product User ID. On a successful call, the caller must release the returned structure using the EOS_Connect_ExternalAccountInfo_Release API.
	UFUNCTION(BlueprintCallable, Category = "Epic Online Services-Kit V2 | SDK Functions | Connect Interface", DisplayName="EOS_Connect_CopyProductUserExternalAccountByAccountId")
	TEnumAsByte<EEOK_Result> EOK_Connect_CopyProductUserExternalAccountByAccountId(FEOK_ProductUserId LocalUserId, FString AccountId, FEOK_Connect_ExternalAccountInfo& OutExternalAccountInfo);

	//Fetch information about an external account of a specific type linked to a Product User ID. On a successful call, the caller must release the returned structure using the EOS_Connect_ExternalAccountInfo_Release API.
	UFUNCTION(BlueprintCallable, Category = "Epic Online Services-Kit V2 | SDK Functions | Connect Interface", DisplayName="EOS_Connect_CopyProductUserExternalAccountByAccountType")
	TEnumAsByte<EEOK_Result> EOK_Connect_CopyProductUserExternalAccountByAccountType(FEOK_ProductUserId LocalUserId, TEnumAsByte<EEOK_EExternalAccountType> AccountType, FEOK_Connect_ExternalAccountInfo& OutExternalAccountInfo);

	//Fetch information about an external account linked to a Product User ID. On a successful call, the caller must release the returned structure using the EOS_Connect_ExternalAccountInfo_Release API.
	UFUNCTION(BlueprintCallable, Category = "Epic Online Services-Kit V2 | SDK Functions | Connect Interface", DisplayName="EOS_Connect_CopyProductUserExternalAccountByIndex")
	TEnumAsByte<EEOK_Result> EOK_Connect_CopyProductUserExternalAccountByIndex(FEOK_ProductUserId LocalUserId, int32 Index, FEOK_Connect_ExternalAccountInfo& OutExternalAccountInfo);

	//Fetch information about a Product User, using the external account that they most recently logged in with as the reference. On a successful call, the caller must release the returned structure using the EOS_Connect_ExternalAccountInfo_Release API.
	UFUNCTION(BlueprintCallable, Category = "Epic Online Services-Kit V2 | SDK Functions | Connect Interface", DisplayName="EOS_Connect_CopyProductUserInfo")
	TEnumAsByte<EEOK_Result> EOK_Connect_CopyProductUserInfo(FEOK_ProductUserId LocalUserId, FEOK_Connect_ExternalAccountInfo& OutProductUserInfo);

	//Release the memory associated with an external account info. This must be called on data retrieved from EOS_Connect_CopyProductUserExternalAccountByIndex, EOS_Connect_CopyProductUserExternalAccountByAccountType, EOS_Connect_CopyProductUserExternalAccountByAccountId or EOS_Connect_CopyProductUserInfo.
	UFUNCTION(BlueprintCallable, Category = "Epic Online Services-Kit V2 | SDK Functions | Connect Interface", DisplayName="EOS_Connect_ExternalAccountInfo_Release")
	void EOK_Connect_ExternalAccountInfo_Release(FEOK_Connect_ExternalAccountInfo ExternalAccountInfo);

	//Fetch a Product User ID that maps to an external account ID cached from a previous query.
	UFUNCTION(BlueprintCallable, Category = "Epic Online Services-Kit V2 | SDK Functions | Connect Interface", DisplayName="EOS_Connect_GetExternalAccountMapping")
	FEOK_ProductUserId EOK_Connect_GetExternalAccountMapping(FEOK_ProductUserId LocalUserId, TEnumAsByte<EEOK_EExternalAccountType> AccountIdType, FString TargetExternalUserId);

	//Fetch a Product User ID that is logged in. This Product User ID is in the Epic Online Services namespace.
	UFUNCTION(BlueprintCallable, Category = "Epic Online Services-Kit V2 | SDK Functions | Connect Interface", DisplayName="EOS_Connect_GetLoggedInUserByIndex")
	FEOK_ProductUserId EOK_Connect_GetLoggedInUserByIndex(int32 Index);

	//Fetch the number of product users that are logged in.
	UFUNCTION(BlueprintCallable, Category = "Epic Online Services-Kit V2 | SDK Functions | Connect Interface", DisplayName="EOS_Connect_GetLoggedInUsersCount")
	int32 EOK_Connect_GetLoggedInUsersCount();

	//Fetches the login status for an Product User ID. This Product User ID is considered logged in as long as the underlying access token has not expired.
	UFUNCTION(BlueprintCallable, Category = "Epic Online Services-Kit V2 | SDK Functions | Connect Interface", DisplayName="EOS_Connect_GetLoginStatus")
	TEnumAsByte<EEOK_LoginStatus> EOK_Connect_GetLoginStatus(FEOK_ProductUserId LocalUserId);

	//Fetch the number of linked external accounts for a Product User ID.
	UFUNCTION(BlueprintCallable, Category = "Epic Online Services-Kit V2 | SDK Functions | Connect Interface", DisplayName="EOS_Connect_GetProductUserExternalAccountCount")
	int32 EOK_Connect_GetProductUserExternalAccountCount(FEOK_ProductUserId LocalUserId);

	//Fetch an external account ID, in string form, that maps to a given Product User ID.
	UFUNCTION(BlueprintCallable, Category = "Epic Online Services-Kit V2 | SDK Functions | Connect Interface", DisplayName="EOS_Connect_GetProductUserIdMapping")
	TEnumAsByte<EEOK_Result> EOK_Connect_GetProductUserIdMapping(FEOK_ProductUserId LocalUserId, TEnumAsByte<EEOK_EExternalAccountType> AccountIdType, FEOK_ProductUserId TargetUserId, FString& OutBuffer);

	//Release the memory associated with an EOS_Connect_IdToken structure. This must be called on data retrieved from EOS_Connect_CopyIdToken.
	UFUNCTION(BlueprintCallable, Category = "Epic Online Services-Kit V2 | SDK Functions | Connect Interface", DisplayName="EOS_Connect_IdToken_Release")
	void EOK_Connect_IdToken_Release(FEOK_Connect_IdToken IdToken);

	//Unregister from receiving expiration notifications.
	UFUNCTION(BlueprintCallable, Category = "Epic Online Services-Kit V2 | SDK Functions | Connect Interface", DisplayName="EOS_Connect_RemoveNotifyAuthExpiration")
	void EOK_Connect_RemoveNotifyAuthExpiration(FEOK_NotificationId InId);

	UFUNCTION(BlueprintCallable, Category = "Epic Online Services-Kit V2 | SDK Functions | Connect Interface", DisplayName="EOS_Connect_RemoveNotifyLoginStatusChanged")
	void EOK_Connect_RemoveNotifyLoginStatusChanged(FEOK_NotificationId InId);

};
