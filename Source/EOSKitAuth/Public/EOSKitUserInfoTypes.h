// Copyright (C) 2024, All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "EOSKitSharedTypes.h"
#include "EOSKitUserInfoTypes.generated.h"

/**
 * User information structure
 */
USTRUCT(BlueprintType, Category = "EOSKit|UserInfo")
struct EOSKITAUTH_API FEOSKitUserInfo
{
	GENERATED_BODY()

	/** The Epic Account ID in string format */
	UPROPERTY(BlueprintReadWrite, Category = "EOSKit|UserInfo")
	FString EpicAccountId = TEXT("");

	/** The name of the user's country. This may be empty */
	UPROPERTY(BlueprintReadWrite, Category = "EOSKit|UserInfo")
	FString Country = TEXT("");

	/** The display name (un-sanitized). This may be empty */
	UPROPERTY(BlueprintReadWrite, Category = "EOSKit|UserInfo")
	FString DisplayName = TEXT("");

	/** The ISO 639 language code for the user's preferred language. This may be empty */
	UPROPERTY(BlueprintReadWrite, Category = "EOSKit|UserInfo")
	FString PreferredLanguage = TEXT("");

	/** A nickname/alias for the target user assigned by the local user. This may be empty */
	UPROPERTY(BlueprintReadWrite, Category = "EOSKit|UserInfo")
	FString Nickname = TEXT("");

	/** The raw display name (sanitized). This may be empty */
	UPROPERTY(BlueprintReadWrite, Category = "EOSKit|UserInfo")
	FString DisplayNameSanitized = TEXT("");
};

/**
 * Product User ID and Epic Account ID pair
 */
USTRUCT(BlueprintType, Category = "EOSKit|UserInfo")
struct EOSKITAUTH_API FEOSKitProductUserIdAndEpicId
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, Category = "EOSKit|UserInfo")
	FString EpicAccountId = TEXT("");

	UPROPERTY(BlueprintReadWrite, Category = "EOSKit|UserInfo")
	FString ProductUserId = TEXT("");
};

/**
 * External account information
 */
USTRUCT(BlueprintType, Category = "EOSKit|UserInfo")
struct EOSKITAUTH_API FEOSKitExternalUserInfo
{
	GENERATED_BODY()

	/** The type of the external account */
	UPROPERTY(BlueprintReadWrite, Category = "EOSKit|UserInfo")
	int32 AccountType = 0;

	/** The ID of the external account */
	UPROPERTY(BlueprintReadWrite, Category = "EOSKit|UserInfo")
	FString AccountId = TEXT("");

	/** The display name of the external account (un-sanitized) */
	UPROPERTY(BlueprintReadWrite, Category = "EOSKit|UserInfo")
	FString DisplayName = TEXT("");

	/** The display name of the external account (sanitized) */
	UPROPERTY(BlueprintReadWrite, Category = "EOSKit|UserInfo")
	FString DisplayNameSanitized = TEXT("");
};

