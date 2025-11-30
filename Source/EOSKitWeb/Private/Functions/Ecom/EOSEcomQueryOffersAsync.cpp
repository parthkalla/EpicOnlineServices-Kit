// Copyright (C) 2024, All Rights Reserved.

#include "Functions/Ecom/EOSEcomQueryOffersAsync.h"
#include "EOSKitSubsystem.h"
#include "Kismet/GameplayStatics.h"
#if WITH_EOS_SDK
#include "Windows/AllowWindowsPlatformTypes.h"
#include "Windows/PreWindowsApi.h"
#include "eos_platform.h"
#include "eos_ecom.h"
#include "eos_ecom_types.h"
#include "Windows/PostWindowsApi.h"
#include "Windows/HideWindowsPlatformTypes.h"
#endif
#include "Async/Async.h"

UEOSEcomQueryOffersAsync* UEOSEcomQueryOffersAsync::QueryOffers(
	UObject* WorldContextObject,
	const FString& LocalUserId)
{
	UEOSEcomQueryOffersAsync* Node = NewObject<UEOSEcomQueryOffersAsync>();
	Node->WorldContextObject = WorldContextObject;
	Node->UserId = LocalUserId;
	return Node;
}

void UEOSEcomQueryOffersAsync::Activate()
{
	PerformQueryOffers();
}

void UEOSEcomQueryOffersAsync::PerformQueryOffers()
{
	if (!WorldContextObject)
	{
		OnFail.Broadcast(TEXT("Invalid World Context"));
		SetReadyToDestroy();
		return;
	}

	UGameInstance* GameInstance = UGameplayStatics::GetGameInstance(WorldContextObject);
	if (!GameInstance)
	{
		OnFail.Broadcast(TEXT("Failed to get Game Instance"));
		SetReadyToDestroy();
		return;
	}

	UEOSKitSubsystem* EOSSubsystem = GameInstance->GetSubsystem<UEOSKitSubsystem>();
	if (!EOSSubsystem || !EOSSubsystem->GetPlatformHandle())
	{
		OnFail.Broadcast(TEXT("EOSKit Subsystem not initialized"));
		SetReadyToDestroy();
		return;
	}

	EOS_HPlatform PlatformHandle = EOSSubsystem->GetPlatformHandle();
	EOS_HEcom EcomHandle = EOS_Platform_GetEcomInterface(PlatformHandle);

	if (!EcomHandle)
	{
		OnFail.Broadcast(TEXT("Failed to get Ecom interface"));
		SetReadyToDestroy();
		return;
	}

	// Convert user ID string to EOS type
	EOS_EpicAccountId EpicAccountId = EOS_EpicAccountId_FromString(TCHAR_TO_UTF8(*UserId));
	if (!EpicAccountId)
	{
		OnFail.Broadcast(TEXT("Invalid Epic Account ID"));
		SetReadyToDestroy();
		return;
	}

	// Setup query options
	EOS_Ecom_QueryOffersOptions QueryOptions = {};
	QueryOptions.ApiVersion = EOS_ECOM_QUERYOFFERS_API_LATEST;
	QueryOptions.LocalUserId = EpicAccountId;

	UE_LOG(LogTemp, Log, TEXT("EOSKitWeb: Querying Ecom offers for user: %s"), *UserId);

	// Perform query - use lambda wrapper to avoid friend function issues
	EOS_Ecom_QueryOffers(EcomHandle, &QueryOptions, this, [](const EOS_Ecom_QueryOffersCallbackInfo* Data)
	{
		if (!Data || !Data->ClientData)
		{
			return;
		}

		UEOSEcomQueryOffersAsync* Self = static_cast<UEOSEcomQueryOffersAsync*>(Data->ClientData);

		AsyncTask(ENamedThreads::GameThread, [Self, Data]()
		{
			if (Data->ResultCode == EOS_EResult::EOS_Success)
			{
				UGameInstance* GameInstance = UGameplayStatics::GetGameInstance(Self->WorldContextObject);
				if (GameInstance)
				{
					UEOSKitSubsystem* EOSSubsystem = GameInstance->GetSubsystem<UEOSKitSubsystem>();
					if (EOSSubsystem && EOSSubsystem->GetPlatformHandle())
					{
						EOS_HEcom EcomHandle = EOS_Platform_GetEcomInterface(EOSSubsystem->GetPlatformHandle());
						if (EcomHandle)
						{
							// Get offer count
							EOS_Ecom_GetOfferCountOptions CountOptions = {};
							CountOptions.ApiVersion = EOS_ECOM_GETOFFERCOUNT_API_LATEST;
							CountOptions.LocalUserId = Data->LocalUserId;

							uint32_t OfferCount = EOS_Ecom_GetOfferCount(EcomHandle, &CountOptions);
							
							TArray<FEOSOfferInfo> Offers;
							Offers.Reserve(OfferCount);

							// Get each offer
							for (uint32_t i = 0; i < OfferCount; i++)
							{
								EOS_Ecom_CopyOfferByIndexOptions CopyOptions = {};
								CopyOptions.ApiVersion = EOS_ECOM_COPYOFFERBYINDEX_API_LATEST;
								CopyOptions.LocalUserId = Data->LocalUserId;
								CopyOptions.OfferIndex = i;

								EOS_Ecom_CatalogOffer* Offer = nullptr;
								EOS_EResult CopyResult = EOS_Ecom_CopyOfferByIndex(EcomHandle, &CopyOptions, &Offer);

								if (CopyResult == EOS_EResult::EOS_Success && Offer)
								{
									FEOSOfferInfo OfferInfo;
									
									if (Offer->Id)
									{
										OfferInfo.OfferId = UTF8_TO_TCHAR(Offer->Id);
									}
									if (Offer->TitleText)
									{
										OfferInfo.Title = UTF8_TO_TCHAR(Offer->TitleText);
									}
									if (Offer->DescriptionText)
									{
										OfferInfo.Description = UTF8_TO_TCHAR(Offer->DescriptionText);
									}
									if (Offer->LongDescriptionText)
									{
										OfferInfo.LongDescription = UTF8_TO_TCHAR(Offer->LongDescriptionText);
									}
									if (Offer->CurrencyCode)
									{
										OfferInfo.CurrencyCode = UTF8_TO_TCHAR(Offer->CurrencyCode);
									}
									
									OfferInfo.Price = Offer->CurrentPrice64;
									OfferInfo.OriginalPrice = Offer->OriginalPrice64;
									OfferInfo.ExpirationTimestamp = FDateTime::FromUnixTimestamp(Offer->ExpirationTimestamp);

									Offers.Add(OfferInfo);

									// Release the offer
									EOS_Ecom_CatalogOffer_Release(Offer);
								}
							}

							UE_LOG(LogTemp, Log, TEXT("EOSKitWeb: Query offers successful. Found %d offers"), Offers.Num());
							Self->OnSuccess.Broadcast(Offers);
							Self->SetReadyToDestroy();
							return;
						}
					}
				}

				Self->OnFail.Broadcast(TEXT("Failed to retrieve offers"));
				Self->SetReadyToDestroy();
			}
			else
			{
				FString ErrorMessage = FString::Printf(TEXT("Query offers failed: %s"), 
					UTF8_TO_TCHAR(EOS_EResult_ToString(Data->ResultCode)));
				UE_LOG(LogTemp, Error, TEXT("EOSKitWeb: %s"), *ErrorMessage);
				Self->OnFail.Broadcast(ErrorMessage);
				Self->SetReadyToDestroy();
			}
		});
	});
}
