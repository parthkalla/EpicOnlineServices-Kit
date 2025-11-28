// Copyright (C) 2024, All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintAsyncActionBase.h"
#include "EOSKitSessionStructs.h"
#include "EOSTransferDeviceIdAccountAsync.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FTransferDeviceIdAccount_Delegate, const FString&, LocalUserId, bool, bSuccess);

/**
 * Transfer Device ID Account - Transfer a Device ID pseudo-account into another keychain
 * 
 * This allows moving game progression from a Device ID account to a persistent account
 * (Epic Games, Steam, PSN, etc.) to protect against device loss.
 */
UCLASS()
class EOSKITSESSIONS_API UEOSTransferDeviceIdAccountAsync : public UBlueprintAsyncActionBase
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintAssignable, DisplayName="On Complete")
	FTransferDeviceIdAccount_Delegate OnComplete;

	/**
	 * Transfer a Device ID pseudo-account and its product user into another keychain
	 * 
	 * Use Case: User plays anonymously with Device ID, then logs in with real account
	 * that already has game data. Transfer Device ID progress to the real account.
	 * 
	 * WARNING: The discarded product user will be permanently deleted!
	 * 
	 * @param PrimaryLocalUserId - Primary user's Product User ID (real account)
	 * @param LocalUserId - Device ID user's Product User ID to transfer from
	 * @param ProductUserIdToPreserve - Which Product User ID's data to keep
	 */
	UFUNCTION(BlueprintCallable, DisplayName="Transfer Device ID Account", meta = (BlueprintInternalUseOnly = "true"), Category="EOSKit|Sessions")
	static UEOSTransferDeviceIdAccountAsync* TransferDeviceIdAccount(
		FString PrimaryLocalUserId,
		FString LocalUserId,
		FString ProductUserIdToPreserve
	);

	virtual void Activate() override;

private:
	void TransferAccount();
	
	FString VarPrimaryLocalUserId;
	FString VarLocalUserId;
	FString VarProductUserIdToPreserve;
};
