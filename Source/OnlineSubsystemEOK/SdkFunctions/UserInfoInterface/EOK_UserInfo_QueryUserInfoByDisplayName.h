// Copyright (c) 2023 Betide Studio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintAsyncActionBase.h"
#include "OnlineSubsystemEOK/SdkFunctions/EOK_SharedFunctionFile.h"
#include "OnlineSubsystemEOS.h"
#include "eos_userinfo.h"
#include "EOK_UserInfo_QueryUserInfoByDisplayName.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_FourParams(FEOK_OnQueryUserInfoByDisplayNameCallback, const TEnumAsByte<EEOK_Result>&, ResultCode, const FEOK_EpicAccountId&, LocalUserId, const FEOK_EpicAccountId&, TargetUserId, const FString&, DisplayName);

UCLASS()
class OnlineSubsystemEOK_API UEOK_UserInfo_QueryUserInfoByDisplayName : public UBlueprintAsyncActionBase
{
	GENERATED_BODY()

public:

	//EOS_UserInfo_QueryUserInfoByDisplayName is used to start an asynchronous query to retrieve user information by display name. This can be useful for getting the EOS_EpicAccountId for a display name. Once the callback has been fired with a successful ResultCode, it is possible to call EOS_UserInfo_CopyUserInfo to receive an EOS_UserInfo containing the available information.
	UFUNCTION(BlueprintCallable, Category = "Epic Online Services-Kit V2 | SDK Functions | User Info Interface", DisplayName="EOS_UserInfo_QueryUserInfoByDisplayName")
	static UEOK_UserInfo_QueryUserInfoByDisplayName* EOK_UserInfo_QueryUserInfoByDisplayName(const FEOK_EpicAccountId& LocalUserId, const FString& DisplayName);

	UPROPERTY(BlueprintAssignable, Category = "Epic Online Services-Kit V2")
	FEOK_OnQueryUserInfoByDisplayNameCallback OnCallback;
private:
	FEOK_EpicAccountId Var_LocalUserId;
	FString Var_DisplayName;
	virtual void Activate() override;
	static void EOS_CALL OnQueryUserInfoByDisplayNameCallback(const EOS_UserInfo_QueryUserInfoByDisplayNameCallbackInfo* Data);
};
