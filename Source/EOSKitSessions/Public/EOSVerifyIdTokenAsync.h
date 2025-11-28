// Copyright (C) 2024, All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintAsyncActionBase.h"
#include "EOSKitSessionStructs.h"
#include "EOSVerifyIdTokenAsync.generated.h"

/**
 * Callback info structure for ID token verification
 */
USTRUCT(BlueprintType)
struct FEOSKitVerifyIdTokenCallbackInfo
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, Category = "EOSKit|Connect")
	bool bSuccess;

	UPROPERTY(BlueprintReadWrite, Category = "EOSKit|Connect")
	FString ProductUserId;

	UPROPERTY(BlueprintReadWrite, Category = "EOSKit|Connect")
	bool bIsAccountInfoPresent;

	UPROPERTY(BlueprintReadWrite, Category = "EOSKit|Connect")
	TEnumAsByte<EEOSKitExternalAccountType> AccountType;

	UPROPERTY(BlueprintReadWrite, Category = "EOSKit|Connect")
	FString AccountId;

	UPROPERTY(BlueprintReadWrite, Category = "EOSKit|Connect")
	FString Platform;

	UPROPERTY(BlueprintReadWrite, Category = "EOSKit|Connect")
	FString DeviceType;

	UPROPERTY(BlueprintReadWrite, Category = "EOSKit|Connect")
	FString ClientId;
	
	UPROPERTY(BlueprintReadWrite, Category = "EOSKit|Connect")
	FString ProductId;

	UPROPERTY(BlueprintReadWrite, Category = "EOSKit|Connect")
	FString SandboxId;

	UPROPERTY(BlueprintReadWrite, Category = "EOSKit|Connect")
	FString DeploymentId;

	FEOSKitVerifyIdTokenCallbackInfo()
		: bSuccess(false)
		, ProductUserId(TEXT(""))
		, bIsAccountInfoPresent(false)
		, AccountType(EKAT_Epic)
		, AccountId(TEXT(""))
		, Platform(TEXT(""))
		, DeviceType(TEXT(""))
		, ClientId(TEXT(""))
		, ProductId(TEXT(""))
		, SandboxId(TEXT(""))
		, DeploymentId(TEXT(""))
	{
	}
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FVerifyIdToken_Delegate, const FEOSKitVerifyIdTokenCallbackInfo&, CallbackInfo);

/**
 * Verify ID Token - Verify a given ID token for authenticity and validity
 */
UCLASS()
class EOSKITSESSIONS_API UEOSVerifyIdTokenAsync : public UBlueprintAsyncActionBase
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintAssignable, DisplayName="On Complete")
	FVerifyIdToken_Delegate OnComplete;

	/**
	 * Verify a given ID token for authenticity and validity
	 * 
	 * Use Cases:
	 * - Validate user authentication tokens
	 * - Verify cross-platform account credentials
	 * - Check token expiration and validity
	 * 
	 * @param IdToken - The ID token (JWT) to verify
	 */
	UFUNCTION(BlueprintCallable, DisplayName="Verify ID Token", meta = (BlueprintInternalUseOnly = "true"), Category="EOSKit|Sessions")
	static UEOSVerifyIdTokenAsync* VerifyIdToken(FEOSKitConnectIdToken IdToken);

	virtual void Activate() override;

private:
	void PerformVerification();
	
	FEOSKitConnectIdToken VarIdToken;
};
