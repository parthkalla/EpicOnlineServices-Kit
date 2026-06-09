// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "eos_userinfo.h"
#include "eos_userinfo_types.h"
#include "Kismet/GameplayStatics.h"
#include "OnlineSubsystemUtils.h"
#include "OnlineSubsystem.h"
#include "Engine/LocalPlayer.h"
#include "GameFramework/PlayerState.h"
#include "Interfaces/OnlineIdentityInterface.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "OnlineSubsystemEOK/Subsystem/EOK_Subsystem.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "EOK_FindUserByDisplayName_Async.generated.h"

/**
 *
 */

USTRUCT(BlueprintType, Category = "Epic Online Services-Kit V2|UserInfo")
struct FEOKUserInfo
{
    GENERATED_BODY()

    /** The Epic Account ID in string format of the found user */
    UPROPERTY(BlueprintReadWrite, Category = "Epic Online Services-Kit V2|UserInfo")
    FString EpicAccountID;
    /** The name of the found users country. This may be null */
    UPROPERTY(BlueprintReadWrite, Category = "Epic Online Services-Kit V2|UserInfo")
    FString Country;
    /** The display name (un-sanitized). This may be null */
    UPROPERTY(BlueprintReadWrite, Category = "Epic Online Services-Kit V2|UserInfo")
    FString DisplayName;
    /** The ISO 639 language code for the user's preferred language. This may be null */
    UPROPERTY(BlueprintReadWrite, Category = "Epic Online Services-Kit V2|UserInfo")
    FString PreferredLanguage;
    /** A nickname/alias for the target user assigned by the local user. This may be null */
    UPROPERTY(BlueprintReadWrite, Category = "Epic Online Services-Kit V2|UserInfo")
    FString Nickname;
    /** The raw display name (sanitized). This may be null */
    UPROPERTY(BlueprintReadWrite, Category = "Epic Online Services-Kit V2|UserInfo")
    FString DisplayNameSanitized;
};


DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FFindUserByDisplayNameDelegate, const FEOKUserInfo, EOKUserInfo);

UCLASS()
class OnlineSubsystemEOK_API UEOK_FindUserByDisplayName_Async : public UBlueprintAsyncActionBase
{
	GENERATED_BODY()

	UFUNCTION(BlueprintCallable, DisplayName = "Find EOK Player By Display Name", meta = (BlueprintInternalUseOnly = "true"), Category = "Epic Online Services-Kit V2 || UserInfo")
	static UEOK_FindUserByDisplayName_Async* FindEOKUserByDisplayName(FString TargetDisplayName, FString LocalEpicID);

	FString TargetDisplayName;

	FString LocalEpicID;

	void FindUserByDisplayName();

	UPROPERTY(BlueprintAssignable, Category = "Epic Online Services-Kit V2 || UserInfo")
    FFindUserByDisplayNameDelegate Success;

	UPROPERTY(BlueprintAssignable, Category = "Epic Online Services-Kit V2 || UserInfo")
    FFindUserByDisplayNameDelegate Failure;

	static void EOS_CALL FindUserByDisplayNameCallback(const EOS_UserInfo_QueryUserInfoByDisplayNameCallbackInfo* Data);

	void ResultFaliure();

	void ResultSuccess(const FEOKUserInfo UserInfoStruct);

	void Activate() override;
};
