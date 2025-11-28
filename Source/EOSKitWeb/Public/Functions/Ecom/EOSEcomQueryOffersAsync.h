// Copyright (C) 2024, All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintAsyncActionBase.h"
#include "EOSEcomQueryOffersAsync.generated.h"

USTRUCT(BlueprintType)
struct FEOSOfferInfo
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "EOSKit|Ecom")
	FString OfferId;

	UPROPERTY(BlueprintReadOnly, Category = "EOSKit|Ecom")
	FString Title;

	UPROPERTY(BlueprintReadOnly, Category = "EOSKit|Ecom")
	FString Description;

	UPROPERTY(BlueprintReadOnly, Category = "EOSKit|Ecom")
	FString LongDescription;

	UPROPERTY(BlueprintReadOnly, Category = "EOSKit|Ecom")
	FString CurrencyCode;

	UPROPERTY(BlueprintReadOnly, Category = "EOSKit|Ecom")
	int32 Price;

	UPROPERTY(BlueprintReadOnly, Category = "EOSKit|Ecom")
	int32 OriginalPrice;

	UPROPERTY(BlueprintReadOnly, Category = "EOSKit|Ecom")
	FDateTime ExpirationTimestamp;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnQueryOffersComplete, const TArray<FEOSOfferInfo>&, Offers);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnQueryOffersFail, const FString&, ErrorMessage);

/**
 * Query available offers from the EOS Ecom catalog
 */
UCLASS()
class EOSKITWEB_API UEOSEcomQueryOffersAsync : public UBlueprintAsyncActionBase
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintAssignable)
	FOnQueryOffersComplete OnSuccess;

	UPROPERTY(BlueprintAssignable)
	FOnQueryOffersFail OnFail;

	/**
	 * Query available offers from the EOS Ecom catalog
	 * 
	 * @param WorldContextObject World context
	 * @param LocalUserId The Epic Account ID of the local user
	 */
	UFUNCTION(BlueprintCallable, meta = (BlueprintInternalUseOnly = "true", WorldContext = "WorldContextObject"), Category = "EOSKit|Web|Ecom")
	static UEOSEcomQueryOffersAsync* QueryOffers(
		UObject* WorldContextObject,
		const FString& LocalUserId
	);

	virtual void Activate() override;

	void PerformQueryOffers();

	UObject* WorldContextObject;
	FString UserId;
};
