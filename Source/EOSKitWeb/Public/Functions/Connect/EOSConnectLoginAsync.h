// Copyright (C) 2024, All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintAsyncActionBase.h"
#include "eos_connect_types.h"
#include "EOSConnectLoginAsync.generated.h"

UENUM(BlueprintType)
enum class EEOSWebCredentialType : uint8
{
	Epic UMETA(DisplayName = "Epic"),
	Steam UMETA(DisplayName = "Steam"),
	PSN UMETA(DisplayName = "PlayStation Network"),
	XBL UMETA(DisplayName = "Xbox Live"),
	Discord UMETA(DisplayName = "Discord"),
	GOG UMETA(DisplayName = "GOG"),
	Nintendo UMETA(DisplayName = "Nintendo"),
	AppleIdToken UMETA(DisplayName = "Apple ID Token"),
	GoogleIdToken UMETA(DisplayName = "Google ID Token"),
	OculusUserIdNonce UMETA(DisplayName = "Oculus User ID Nonce"),
	ItchioJwt UMETA(DisplayName = "Itch.io JWT"),
	ItchioKey UMETA(DisplayName = "Itch.io Key"),
	EpicIdToken UMETA(DisplayName = "Epic ID Token"),
	AmazonAccessToken UMETA(DisplayName = "Amazon Access Token"),
	OpenIdAccessToken UMETA(DisplayName = "OpenID Access Token")
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnConnectLoginComplete, const FString&, ProductUserId);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnConnectLoginFail, const FString&, ErrorMessage);

/**
 * Login to EOS Connect interface using external account credentials
 * This is used for linking external accounts like Steam, Apple, Google, etc.
 */
UCLASS()
class EOSKITWEB_API UEOSConnectLoginAsync : public UBlueprintAsyncActionBase
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintAssignable)
	FOnConnectLoginComplete OnSuccess;

	UPROPERTY(BlueprintAssignable)
	FOnConnectLoginFail OnFail;

	/**
	 * Login to EOS Connect using external credentials
	 * 
	 * @param WorldContextObject World context
	 * @param ExternalToken The token from the external account provider
	 * @param Type The type of external credential (Steam, Apple, Google, etc.)
	 * @param DisplayName Optional display name for the user
	 */
	UFUNCTION(BlueprintCallable, meta = (BlueprintInternalUseOnly = "true", WorldContext = "WorldContextObject"), Category = "EOSKit|Web|Connect")
	static UEOSConnectLoginAsync* ConnectLogin(
		UObject* WorldContextObject,
		const FString& ExternalToken,
		EEOSWebCredentialType Type,
		const FString& DisplayName = TEXT("")
	);

	virtual void Activate() override;

protected:
	void PerformConnectLogin();

	static void EOS_CALL OnConnectLoginCallback(const EOS_Connect_LoginCallbackInfo* Data);
	static void EOS_CALL OnCreateUserCallback(const EOS_Connect_CreateUserCallbackInfo* Data);

	UObject* WorldContextObject;
	FString Token;
	EEOSWebCredentialType CredentialType;
	FString UserDisplayName;

	// Helper to convert enum to EOS type
	static EOS_EExternalCredentialType ConvertCredentialType(EEOSWebCredentialType Type);
};
