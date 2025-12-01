// Copyright (C) 2024, All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Interfaces/OnlinePurchaseInterface.h"
#include "OnlineSubsystemEOSKit.h"

#if WITH_EOS_SDK
	#include "eos_ecom.h"
#endif

class FOnlineSubsystemEOSKit;

#if WITH_EOS_SDK

/**
 * Interface for EOS purchases (part of Ecom)
 */
class ONLINESUBSYSTEMEOSKIT_API FOnlinePurchaseEOSKit :
	public IOnlinePurchase
	, public TSharedFromThis<FOnlinePurchaseEOSKit, ESPMode::ThreadSafe>
{
public:
	FOnlinePurchaseEOSKit() = delete;
	explicit FOnlinePurchaseEOSKit(FOnlineSubsystemEOSKit* InSubsystem);
	virtual ~FOnlinePurchaseEOSKit();

	// IOnlinePurchase interface
	virtual void QueryReceipts(const FUniqueNetId& UserId, bool bRestoreReceipts, const FOnQueryReceiptsComplete& Delegate = FOnQueryReceiptsComplete()) override;
	virtual void FinalizePurchase(const FUniqueNetId& UserId, const FString& ReceiptId) override;
	virtual void RedeemCode(const FUniqueNetId& UserId, const FRedeemCodeRequest& RedeemCodeRequest, const FOnPurchaseRedeemCodeComplete& Delegate = FOnPurchaseRedeemCodeComplete()) override;
	virtual void Checkout(const FUniqueNetId& UserId, const FPurchaseCheckoutRequest& CheckoutRequest, const FOnPurchaseCheckoutComplete& Delegate = FOnPurchaseCheckoutComplete()) override;
	virtual bool IsAllowedToPurchase(const FUniqueNetId& UserId) override;
	virtual void Checkout(const FUniqueNetId& UserId, const FPurchaseCheckoutRequest& CheckoutRequest, const FOnPurchaseReceiptlessCheckoutComplete& Delegate) override;
	virtual void GetReceipts(const FUniqueNetId& UserId, TArray<FPurchaseReceipt>& OutReceipts) const override;
	virtual void FinalizeReceiptValidationInfo(const FUniqueNetId& UserId, FString& InReceiptValidationInfo, const FOnFinalizeReceiptValidationInfoComplete& Delegate) override;
	// These methods were removed from IOnlinePurchase in UE 5.5, but kept for backward compatibility
	FOnPurchaseCheckoutComplete& GetCheckoutCompleteDelegate() { return OnCheckoutCompleteDelegate; }
	// FOnPurchaseReceiptsUpdated was removed in UE 5.5 - use OnQueryReceiptsComplete delegate instead
	FOnQueryReceiptsComplete& GetReceiptsUpdatedDelegate() { return OnReceiptsUpdatedDelegate; }

private:
	FOnlineSubsystemEOSKit* EOSKitSubsystem;
	EOS_HEcom EcomHandle;
	
	FOnPurchaseCheckoutComplete OnCheckoutCompleteDelegate;
	FOnQueryReceiptsComplete OnReceiptsUpdatedDelegate;
};

#endif // WITH_EOS_SDK

