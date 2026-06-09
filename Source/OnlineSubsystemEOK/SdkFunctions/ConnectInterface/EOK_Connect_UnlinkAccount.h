// Copyright (c) 2023 Betide Studio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintAsyncActionBase.h"
#include "OnlineSubsystemEOK/SdkFunctions/EOK_SharedFunctionFile.h"
#include "EOK_Connect_UnlinkAccount.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FEOK_Connect_UnlinkAccount_Delegate, const FEOK_ProductUserId&, LocalUserId, TEnumAsByte<EEOK_Result>, Result);
UCLASS()
class OnlineSubsystemEOK_API UEOK_Connect_UnlinkAccount : public UBlueprintAsyncActionBase
{
	GENERATED_BODY()

public:

	//Unlink external auth credentials from the owning keychain of a logged in product user. This function allows recovering the user from scenarios where they have accidentally proceeded to creating a new product user for the local native user account, instead of linking it with an existing keychain that they have previously created by playing the game (or another game owned by the organization) on another platform. In such scenario, after the initial platform login and a new product user creation, the user wishes to re-login using other set of external auth credentials to connect with their existing game progression data. In order to allow automatic login also on the current platform, they will need to unlink the accidentally created new keychain and product user and then use the EOS_Connect_Login and EOS_Connect_LinkAccount APIs to link the local native platform account with that previously created existing product user and its owning keychain. In another scenario, the user may simply want to disassociate the account that they have logged in with from the current keychain that it is linked with, perhaps to link it against another keychain or to separate the game progressions again. In order to protect against account theft, it is only possible to unlink user accounts that have been authenticated and logged in to the product user in the current session. This prevents a malicious actor from gaining access to one of the linked accounts and using it to remove all other accounts linked with the keychain. This also prevents a malicious actor from replacing the unlinked account with their own corresponding account on the same platform, as the unlinking operation will ensure that any existing authentication session cannot be used to re-link and overwrite the entry without authenticating with one of the other linked accounts in the keychain. These restrictions limit the potential attack surface related to account theft scenarios.
	UFUNCTION(BlueprintCallable, Category = "Epic Online Services-Kit V2 | SDK Functions | Connect Interface", DisplayName="EOS_Connect_UnlinkAccount")
	static UEOK_Connect_UnlinkAccount* EOK_Connect_UnlinkAccount(FEOK_ProductUserId LocalUserId);

	UPROPERTY(BlueprintAssignable, Category = "Epic Online Services-Kit V2")
	FEOK_Connect_UnlinkAccount_Delegate OnCallback;
private:
	static void OnUnlinkAccountCallback(const EOS_Connect_UnlinkAccountCallbackInfo* Data);
	virtual void Activate() override;
	FEOK_ProductUserId Var_LocalUserId;
};
