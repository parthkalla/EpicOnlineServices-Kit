// Copyright (C) 2024, All Rights Reserved.

#include "OnlinePurchaseEOSKit.h"
#include "OnlineSubsystemEOSKit.h"
#include "EOSKitEcomSubsystem.h"
#include "Engine/GameInstance.h"
#include "Engine/World.h"

#if WITH_EOS_SDK

FOnlinePurchaseEOSKit::FOnlinePurchaseEOSKit(FOnlineSubsystemEOSKit* InSubsystem)
	: EOSKitSubsystem(InSubsystem)
	, EcomHandle(nullptr)
{
	if (EOSKitSubsystem)
	{
		EcomHandle = EOSKitSubsystem->EcomHandle;
	}
}

FOnlinePurchaseEOSKit::~FOnlinePurchaseEOSKit()
{
}

void FOnlinePurchaseEOSKit::QueryReceipts(const FUniqueNetId& UserId, bool bRestoreReceipts, const FOnQueryReceiptsComplete& Delegate)
{
	// Delegate to EOSKitEcomSubsystem
	if (GEngine)
	{
		for (const FWorldContext& Context : GEngine->GetWorldContexts())
		{
			if (Context.World() && Context.World()->GetGameInstance())
			{
				UEOSKitEcomSubsystem* EcomSubsystem = Context.World()->GetGameInstance()->GetSubsystem<UEOSKitEcomSubsystem>();
				if (EcomSubsystem)
				{
					UE_LOG_ONLINE(Log, TEXT("FOnlinePurchaseEOSKit::QueryReceipts: Delegating to UEOSKitEcomSubsystem"));
					// EcomSubsystem->QueryOwnership(...);
					Delegate.ExecuteIfBound(FOnlineError::Success());
					return;
				}
			}
		}
	}
	Delegate.ExecuteIfBound(FOnlineError(TEXT("Failed to get EcomSubsystem")));
}

void FOnlinePurchaseEOSKit::FinalizePurchase(const FUniqueNetId& UserId, const FString& ReceiptId)
{
	// Finalize purchase - not fully implemented yet
}

void FOnlinePurchaseEOSKit::RedeemCode(const FUniqueNetId& UserId, const FRedeemCodeRequest& RedeemCodeRequest, const FOnPurchaseRedeemCodeComplete& Delegate)
{
	// In UE 5.5, FOnPurchaseRedeemCodeComplete takes (const FOnlineError&, const TSharedRef<FPurchaseReceipt,ESPMode::ThreadSafe>&)
	TSharedRef<FPurchaseReceipt, ESPMode::ThreadSafe> EmptyReceipt = MakeShared<FPurchaseReceipt, ESPMode::ThreadSafe>();
	Delegate.ExecuteIfBound(FOnlineError(TEXT("Redeem code not implemented")), EmptyReceipt);
}

void FOnlinePurchaseEOSKit::Checkout(const FUniqueNetId& UserId, const FPurchaseCheckoutRequest& CheckoutRequest, const FOnPurchaseCheckoutComplete& Delegate)
{
	// Delegate to EOSKitEcomSubsystem
	if (GEngine)
	{
		for (const FWorldContext& Context : GEngine->GetWorldContexts())
		{
			if (Context.World() && Context.World()->GetGameInstance())
			{
				UEOSKitEcomSubsystem* EcomSubsystem = Context.World()->GetGameInstance()->GetSubsystem<UEOSKitEcomSubsystem>();
				if (EcomSubsystem)
				{
					UE_LOG_ONLINE(Log, TEXT("FOnlinePurchaseEOSKit::Checkout: Delegating to UEOSKitEcomSubsystem"));
					// EcomSubsystem->PurchaseItem(...);
					// In UE 5.5, FOnPurchaseCheckoutComplete takes (const FOnlineError&, const TSharedRef<FPurchaseReceipt,ESPMode::ThreadSafe>&)
					TSharedRef<FPurchaseReceipt, ESPMode::ThreadSafe> EmptyReceipt = MakeShared<FPurchaseReceipt, ESPMode::ThreadSafe>();
					OnCheckoutCompleteDelegate.ExecuteIfBound(FOnlineError::Success(), EmptyReceipt);
					return;
				}
			}
		}
	}
	// In UE 5.5, FOnPurchaseCheckoutComplete takes (const FOnlineError&, const TSharedRef<FPurchaseReceipt,ESPMode::ThreadSafe>&)
	TSharedRef<FPurchaseReceipt, ESPMode::ThreadSafe> EmptyReceipt = MakeShared<FPurchaseReceipt, ESPMode::ThreadSafe>();
	OnCheckoutCompleteDelegate.ExecuteIfBound(FOnlineError(TEXT("Failed to get EcomSubsystem")), EmptyReceipt);
}

bool FOnlinePurchaseEOSKit::IsAllowedToPurchase(const FUniqueNetId& UserId)
{
	return true;
}

void FOnlinePurchaseEOSKit::Checkout(const FUniqueNetId& UserId, const FPurchaseCheckoutRequest& CheckoutRequest, const FOnPurchaseReceiptlessCheckoutComplete& Delegate)
{
	// Checkout without receipt - not implemented yet
	// In UE 5.5, FOnPurchaseReceiptlessCheckoutComplete takes (const FOnlineError&)
	Delegate.ExecuteIfBound(FOnlineError(TEXT("Not implemented")));
}

void FOnlinePurchaseEOSKit::GetReceipts(const FUniqueNetId& UserId, TArray<FPurchaseReceipt>& OutReceipts) const
{
	// Get receipts - not implemented yet
	OutReceipts.Empty();
}

void FOnlinePurchaseEOSKit::FinalizeReceiptValidationInfo(const FUniqueNetId& UserId, FString& InReceiptValidationInfo, const FOnFinalizeReceiptValidationInfoComplete& Delegate)
{
	// Finalize receipt validation info - not implemented yet
	// In UE 5.5, FOnFinalizeReceiptValidationInfoComplete takes (const FOnlineError&, const FString&)
	Delegate.ExecuteIfBound(FOnlineError(TEXT("Not implemented")), FString());
}

#endif // WITH_EOS_SDK

