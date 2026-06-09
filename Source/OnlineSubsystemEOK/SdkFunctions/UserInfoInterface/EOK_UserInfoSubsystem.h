// Copyright (c) 2023 Betide Studio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "OnlineSubsystemEOK/SdkFunctions/EOK_SharedFunctionFile.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "EOK_UserInfoSubsystem.generated.h"

/**
 * 
 */
UCLASS()
class OnlineSubsystemEOK_API UEOK_UserInfoSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

	/*
	*EOS_UserInfo_CopyBestDisplayName is used to immediately retrieve a copy of user's best display name based on an Epic Account ID. This uses data cached by a previous call to EOS_UserInfo_QueryUserInfo, EOS_UserInfo_QueryUserInfoByDisplayName or EOS_UserInfo_QueryUserInfoByExternalAccount as well as EOS_Connect_QueryExternalAccountMappings. If the call returns an EOS_Success result, the out parameter, OutBestDisplayName, must be passed to EOS_UserInfo_BestDisplayName_Release to release the memory associated with it.
	The current priority for picking display name is as follows:
	-	Target is online and friends with user, then use presence platform to determine display name
	-	Target is in same lobby or is the owner of a lobby search result, then use lobby platform to determine display name (this requires the target's product user id to be cached)
	-	Target is in same rtc room, then use rtc room platform to determine display name (this requires the target's product user id to be cached)
	 */
	UFUNCTION(BlueprintCallable, Category = "Epic Online Services-Kit V2 | SDK Functions | User Info Interface", DisplayName="EOS_UserInfo_CopyBestDisplayName")
	static TEnumAsByte<EEOK_Result> EOK_UserInfo_CopyBestDisplayName(FEOK_EpicAccountId LocalUserId, FEOK_EpicAccountId TargetUserId, FEOK_UserInfo_BestDisplayName& OutBestDisplayName);

	/*
	*EOS_UserInfo_CopyBestDisplayNameWithPlatform is used to immediately retrieve a copy of user's best display name based on an Epic Account ID. This uses data cached by a previous call to EOS_UserInfo_QueryUserInfo, EOS_UserInfo_QueryUserInfoByDisplayName or EOS_UserInfo_QueryUserInfoByExternalAccount. If the call returns an EOS_Success result, the out parameter, OutBestDisplayName, must be passed to EOS_UserInfo_BestDisplayName_Release to release the memory associated with it.
	The current priority for picking display name is as follows:
		If platform is non-epic, then use platform display name (if the platform is linked to the account)
		If platform is epic and user has epic display name, then use epic display name
		If platform is epic and user has no epic display name, then use linked external account display name
	 */
	UFUNCTION(BlueprintCallable, Category = "Epic Online Services-Kit V2 | SDK Functions | User Info Interface", DisplayName="EOS_UserInfo_CopyBestDisplayNameWithPlatform")
	static TEnumAsByte<EEOK_Result> EOK_UserInfo_CopyBestDisplayNameWithPlatform(FEOK_EpicAccountId LocalUserId, FEOK_EpicAccountId TargetUserId, const int32& Platform, FEOK_UserInfo_BestDisplayName& OutBestDisplayName);

	//Fetches an external user info for a given external account ID.
	UFUNCTION(BlueprintCallable, Category = "Epic Online Services-Kit V2 | SDK Functions | User Info Interface", DisplayName="EOS_UserInfo_CopyExternalUserInfoByAccountId")
	static TEnumAsByte<EEOK_Result> EOK_UserInfo_CopyExternalUserInfoByAccountId(FEOK_EpicAccountId LocalUserId, FEOK_EpicAccountId TargetUserId, FString AccountId, FEOK_UserInfo_ExternalUserInfo& OutExternalUserInfo);

	//Fetches an external user info for a given external account type.
	UFUNCTION(BlueprintCallable, Category = "Epic Online Services-Kit V2 | SDK Functions | User Info Interface", DisplayName="EOS_UserInfo_CopyExternalUserInfoByAccountType")
	static TEnumAsByte<EEOK_Result> EOK_UserInfo_CopyExternalUserInfoByAccountType(FEOK_EpicAccountId LocalUserId, FEOK_EpicAccountId TargetUserId, const TEnumAsByte<EEOK_EExternalAccountType>& AccountType, FEOK_UserInfo_ExternalUserInfo& OutExternalUserInfo);

	//Fetches an external user info from a given index.
	UFUNCTION(BlueprintCallable, Category = "Epic Online Services-Kit V2 | SDK Functions | User Info Interface", DisplayName="EOS_UserInfo_CopyExternalUserInfoByIndex")
	static TEnumAsByte<EEOK_Result> EOK_UserInfo_CopyExternalUserInfoByIndex(FEOK_EpicAccountId LocalUserId, FEOK_EpicAccountId TargetUserId, const int32& Index, FEOK_UserInfo_ExternalUserInfo& OutExternalUserInfo);

	//EOS_UserInfo_CopyUserInfo is used to immediately retrieve a copy of user information based on an Epic Account ID, cached by a previous call to EOS_UserInfo_QueryUserInfo. If the call returns an EOS_Success result, the out parameter, OutUserInfo, must be passed to EOS_UserInfo_Release to release the memory associated with it.
	UFUNCTION(BlueprintCallable, Category = "Epic Online Services-Kit V2 | SDK Functions | User Info Interface", DisplayName="EOS_UserInfo_CopyUserInfo")
	static TEnumAsByte<EEOK_Result> EOK_UserInfo_CopyUserInfo(FEOK_EpicAccountId LocalUserId, FEOK_EpicAccountId TargetUserId, FEOK_UserInfo& OutUserInfo);

	//Fetch the number of external user infos that are cached locally.
	UFUNCTION(BlueprintCallable, Category = "Epic Online Services-Kit V2 | SDK Functions | User Info Interface", DisplayName="EOS_UserInfo_GetExternalUserInfoCount")
	static int32 EOK_UserInfo_GetExternalUserInfoCount(FEOK_EpicAccountId LocalUserId, FEOK_EpicAccountId TargetUserId);
	
	//EOS_UserInfo_GetLocalPlatformType is used to retrieve the online platform type of the current running instance of the game.
	UFUNCTION(BlueprintCallable, Category = "Epic Online Services-Kit V2 | SDK Functions | User Info Interface", DisplayName="EOS_UserInfo_GetLocalPlatformType")
	static int32 EOK_UserInfo_GetLocalPlatformType();
};
