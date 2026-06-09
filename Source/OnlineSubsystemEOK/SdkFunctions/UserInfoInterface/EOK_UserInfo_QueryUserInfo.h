// Copyright (c) 2023 Betide Studio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintAsyncActionBase.h"
#include "OnlineSubsystemEOK/SdkFunctions/EOK_SharedFunctionFile.h"
#include "OnlineSubsystemEOS.h"
#include "eos_userinfo.h"
#include "EOK_UserInfo_QueryUserInfo.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FEOK_OnQueryUserInfoCallback, const TEnumAsByte<EEOK_Result>&, ResultCode, const FEOK_EpicAccountId&, LocalUserId, const FEOK_EpicAccountId&, TargetUserId);

UCLASS()
class OnlineSubsystemEOK_API UEOK_UserInfo_QueryUserInfo : public UBlueprintAsyncActionBase
{
	GENERATED_BODY()

public:
	//EOS_UserInfo_QueryUserInfo is used to start an asynchronous query to retrieve information, such as display name, about another account. Once the callback has been fired with a successful ResultCode, it is possible to call EOS_UserInfo_CopyUserInfo to receive an EOS_UserInfo containing the available information.
	UFUNCTION(BlueprintCallable, Category = "Epic Online Services-Kit V2 | SDK Functions | User Info Interface", DisplayName="EOS_UserInfo_QueryUserInfo")
	static UEOK_UserInfo_QueryUserInfo* EOK_UserInfo_QueryUserInfo(const FEOK_EpicAccountId& LocalUserId, const FEOK_EpicAccountId& TargetUserId);

	UPROPERTY(BlueprintAssignable, Category = "Epic Online Services-Kit V2")
	FEOK_OnQueryUserInfoCallback OnCallback;

private:
	FEOK_EpicAccountId Var_LocalUserId;
	FEOK_EpicAccountId Var_TargetUserId;
	virtual void Activate() override;
	static void EOS_CALL OnQueryUserInfoCallback(const EOS_UserInfo_QueryUserInfoCallbackInfo* Data);
};
