// Copyright (C) 2024, All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "OnlineSubsystem.h"
#include "Kismet/BlueprintAsyncActionBase.h"
#include "Interfaces/OnlineIdentityInterface.h"
#include "EOSConnectLoginWithOnlineSubsystemAsync.generated.h"

/** External credential types for Connect login */
UENUM(BlueprintType)
enum class EEOSKitExternalCredentialType : uint8
{
	DeviceID UMETA(DisplayName = "Device ID"),
	Epic UMETA(DisplayName = "Epic Games"),
	Steam UMETA(DisplayName = "Steam"),
	PSN UMETA(DisplayName = "PSN"),
	XBL UMETA(DisplayName = "Xbox Live"),
	Discord UMETA(DisplayName = "Discord"),
	Nintendo UMETA(DisplayName = "Nintendo"),
	Apple UMETA(DisplayName = "Apple"),
	Google UMETA(DisplayName = "Google"),
	Oculus UMETA(DisplayName = "Oculus"),
	OpenID UMETA(DisplayName = "OpenID")
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnEOSConnectLoginOSS_Complete, const FString&, ProductUserId, const FString&, Error);

/**
 * Login using Connect Interface via Online Subsystem
 * This uses the Online Subsystem Identity Interface for a higher-level abstraction
 */
UCLASS()
class EOSKITAUTH_API UEOSConnectLoginWithOnlineSubsystemAsync : public UBlueprintAsyncActionBase
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintAssignable, DisplayName="Success")
	FOnEOSConnectLoginOSS_Complete OnSuccess;
	
	UPROPERTY(BlueprintAssignable, DisplayName="Failure")
	FOnEOSConnectLoginOSS_Complete OnFail;

	/**
	 * Login using Connect Interface through Online Subsystem
	 * This method logs in a user to EOS using the Connect interface through the Online Subsystem abstraction layer
	 * @param LoginMethod - Login method to use (Device ID recommended for anonymous login)
	 * @param DisplayName - Display name for the user (optional)
	 * @param Token - Authentication token (if needed for the selected login method)
	 */
	UFUNCTION(BlueprintCallable, DisplayName="Login via Connect (Online Subsystem)", meta = (BlueprintInternalUseOnly = "true"), Category="EOSKit|Auth")
	static UEOSConnectLoginWithOnlineSubsystemAsync* LoginUsingConnectInterfaceOSS(
		EEOSKitExternalCredentialType LoginMethod = EEOSKitExternalCredentialType::DeviceID,
		FString DisplayName = "",
		FString Token = ""
	);

	virtual void Activate() override;

private:
	void LoginCallback(int32 LocalUserNum, bool bWasSuccess, const FUniqueNetId& UserId, const FString& Error);

	EEOSKitExternalCredentialType LoginMethod;
	FString DisplayName;
	FString Token;
};
