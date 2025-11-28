// Copyright (C) 2024, All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintAsyncActionBase.h"
#include "eos_auth.h"
#include "eos_connect.h"
#include "EOSLoginAsync.generated.h"

/** Delegate for login completion with detailed outputs */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnEOSLoginComplete, const FString&, EpicUserId, const FString&, ProductUserId, const FString&, Error);

/** Login credential types */
UENUM(BlueprintType)
enum class EEOSCredentialType : uint8
{
	AccountPortal UMETA(DisplayName = "Account Portal"),
	PersistentAuth UMETA(DisplayName = "Persistent Auth"),
	Developer UMETA(DisplayName = "Developer"),
	DeviceCode UMETA(DisplayName = "Device Code"),
	ExchangeCode UMETA(DisplayName = "Exchange Code")
};

/** External credential types for Connect */
UENUM(BlueprintType)
enum class EEOSExternalCredentialType : uint8
{
	Epic UMETA(DisplayName = "Epic Games"),
	Steam UMETA(DisplayName = "Steam"),
	PSN UMETA(DisplayName = "PSN"),
	XBL UMETA(DisplayName = "Xbox Live"),
	Discord UMETA(DisplayName = "Discord"),
	Nintendo UMETA(DisplayName = "Nintendo"),
	Apple UMETA(DisplayName = "Apple"),
	Google UMETA(DisplayName = "Google"),
	Oculus UMETA(DisplayName = "Oculus"),
	OpenID UMETA(DisplayName = "OpenID"),
	DeviceID UMETA(DisplayName = "Device ID")
};

/**
 * Login using Auth Interface (Epic Account Services)
 * Provides Epic Account ID and Product User ID
 */
UCLASS()
class EOSKITAUTH_API UEOSLoginUsingAuthInterface : public UBlueprintAsyncActionBase
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintAssignable)
	FOnEOSLoginComplete OnSuccess;

	UPROPERTY(BlueprintAssignable)
	FOnEOSLoginComplete OnFail;

	/**
	 * Login using Auth Interface with Epic Account Services
	 * @param CredentialType - Type of credential to use
	 * @param ExternalCredentialType - External account type (for display purposes)
	 * @param Id - Credential ID (optional, for some types)
	 * @param Token - Credential token (optional, for some types)
	 */
	UFUNCTION(BlueprintCallable, meta = (BlueprintInternalUseOnly = "true", WorldContext = "WorldContextObject", DisplayName = "Login using Auth Interface"), Category = "EOSKit|Auth")
	static UEOSLoginUsingAuthInterface* LoginUsingAuthInterface(
		UObject* WorldContextObject,
		EEOSCredentialType CredentialType,
		EEOSExternalCredentialType ExternalCredentialType,
		FString Id,
		FString Token
	);

	virtual void Activate() override;

private:
	static void EOS_CALL OnAuthLoginComplete(const EOS_Auth_LoginCallbackInfo* Data);
	static void EOS_CALL OnConnectLoginComplete(const EOS_Connect_LoginCallbackInfo* Data);
	static void EOS_CALL OnCreateUserComplete(const EOS_Connect_CreateUserCallbackInfo* Data);

	void StartConnectLogin(EOS_EpicAccountId EpicAccountId);

	UObject* WorldContextObject;
	EEOSCredentialType CredentialType;
	EEOSExternalCredentialType ExternalCredentialType;
	FString Id;
	FString Token;
	EOS_EpicAccountId CachedEpicAccountId;
};

/**
 * Login using Connect Interface (Product User ID only)
 * For platform-specific logins without Epic Account
 */
UCLASS()
class EOSKITAUTH_API UEOSLoginUsingConnectInterface : public UBlueprintAsyncActionBase
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintAssignable)
	FOnEOSLoginComplete OnSuccess;

	UPROPERTY(BlueprintAssignable)
	FOnEOSLoginComplete OnFail;

	/**
	 * Login using Connect Interface (Product User ID)
	 * @param LoginMethod - Login method (Device ID recommended)
	 * @param DisplayName - Display name for the user
	 * @param Token - Authentication token (if needed)
	 */
	UFUNCTION(BlueprintCallable, meta = (BlueprintInternalUseOnly = "true", WorldContext = "WorldContext Object", DisplayName = "Login using Connect Interface"), Category = "EOSKit|Auth")
	static UEOSLoginUsingConnectInterface* LoginUsingConnectInterface(
		UObject* WorldContextObject,
		FString LoginMethod,
		FString DisplayName,
		FString Token
	);

	virtual void Activate() override;

private:
	static void EOS_CALL OnConnectLoginComplete(const EOS_Connect_LoginCallbackInfo* Data);
	static void EOS_CALL OnCreateUserComplete(const EOS_Connect_CreateUserCallbackInfo* Data);
	static void EOS_CALL OnCreateDeviceIdComplete(const EOS_Connect_CreateDeviceIdCallbackInfo* Data);

	UObject* WorldContextObject;
	FString LoginMethod;
	FString DisplayName;
	FString Token;
};
