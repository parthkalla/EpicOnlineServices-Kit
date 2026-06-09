// Copyright (c) 2023 Betide Studio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintAsyncActionBase.h"
#include "OnlineSubsystemEOK/SdkFunctions/EOK_SharedFunctionFile.h"
#include "EOK_UserInfo_QueryUserInfoByExternalAccount.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_FiveParams(FEOK_OnQueryUserInfoByExternalAccountCallback, const TEnumAsByte<EEOK_Result>&, ResultCode, const FEOK_EpicAccountId&, LocalUserId, const FEOK_EpicAccountId&, TargetUserId, const TEnumAsByte<EEOK_EExternalAccountType>&, ExternalAccountType, const FString&, ExternalAccountId);

UCLASS()
class OnlineSubsystemEOK_API UEOK_UserInfo_QueryUserInfoByExternalAccount : public UBlueprintAsyncActionBase
{
	GENERATED_BODY()

public:
	//EOS_UserInfo_QueryUserInfoByExternalAccount is used to start an asynchronous query to retrieve user information by external accounts. This can be useful for getting the EOS_EpicAccountId for external accounts. Once the callback has been fired with a successful ResultCode, it is possible to call CopyUserInfo to receive an EOS_UserInfo containing the available information.
	UFUNCTION(BlueprintCallable, Category = "Epic Online Services-Kit V2 | SDK Functions | User Info Interface", DisplayName="EOS_UserInfo_QueryUserInfoByExternalAccount")
	static UEOK_UserInfo_QueryUserInfoByExternalAccount* EOK_UserInfo_QueryUserInfoByExternalAccount(const FEOK_EpicAccountId& LocalUserId, const TEnumAsByte<EEOK_EExternalAccountType>& AccountType, const FString& ExternalAccountId);

	UPROPERTY(BlueprintAssignable, Category = "Epic Online Services-Kit V2")
	FEOK_OnQueryUserInfoByExternalAccountCallback OnCallback;

private:
	FEOK_EpicAccountId Var_LocalUserId;
	TEnumAsByte<EEOK_EExternalAccountType> Var_AccountType;
	FString Var_ExternalAccountId;
	virtual void Activate() override;
	static void EOS_CALL OnQueryUserInfoByExternalAccountCallback(const EOS_UserInfo_QueryUserInfoByExternalAccountCallbackInfo* Data);
};
