// Copyright (c) 2023 Betide Studio. All Rights Reserved.


#include "EOK_EcomSubsystem.h"

#include "OnlineSubsystemEOS.h"
#include "OnlineSubsystemEOK/SdkFunctions/ConnectInterface/EOK_ConnectSubsystem.h"

void UEOK_EcomSubsystem::EOK_Ecom_CatalogItem_Release(FEOK_Ecom_CatalogItem CatalogItem)
{
	EOS_Ecom_CatalogItem_Release(&CatalogItem.Ref);
}

void UEOK_EcomSubsystem::EOK_Ecom_CatalogOffer_Release(FEOK_Ecom_CatalogOffer CatalogOffer)
{
	EOS_Ecom_CatalogOffer_Release(&CatalogOffer.Ref);
}

void UEOK_EcomSubsystem::EOK_Ecom_CatalogRelease_Release(FEOK_Ecom_CatalogRelease CatalogRelease)
{
	EOS_Ecom_CatalogRelease_Release(&CatalogRelease.Ref);
}

TEnumAsByte<EEOK_Result> UEOK_EcomSubsystem::EOK_Ecom_CopyEntitlementById(FEOK_EpicAccountId LocalUserId,
	const FString& EntitlementId, FEOK_Ecom_Entitlement& OutEntitlement)
{
	if(	IOnlineSubsystem* OnlineSub = IOnlineSubsystem::Get("EOK"))
	{
		if (FOnlineSubsystemEOS* EOSRef = static_cast<FOnlineSubsystemEOS*>(OnlineSub))
		{
			EOS_Ecom_CopyEntitlementByIdOptions CopyEntitlementByIdOptions = { };
			CopyEntitlementByIdOptions.ApiVersion = EOS_ECOM_COPYENTITLEMENTBYID_API_LATEST;
			CopyEntitlementByIdOptions.LocalUserId = LocalUserId.GetValueAsEosType();
			CopyEntitlementByIdOptions.EntitlementId = TCHAR_TO_ANSI(*EntitlementId);
			EOS_Ecom_Entitlement* LocalRef;
			auto Result = EOS_Ecom_CopyEntitlementById(EOSRef->EcomHandle, &CopyEntitlementByIdOptions, &LocalRef);
			OutEntitlement = FEOK_Ecom_Entitlement(*LocalRef);
			return static_cast<EEOK_Result>(Result);
		}
	}
	UE_LOG(LogEOK, Error, TEXT("EOK_Ecom_CopyEntitlementById: OnlineSubsystemEOK is not available"));
	return EEOK_Result::EOS_ServiceFailure;
}


TEnumAsByte<EEOK_Result> UEOK_EcomSubsystem::EOK_Ecom_CopyEntitlementByIndex(FEOK_EpicAccountId LocalUserId,
                                                                             int32 EntitlementIndex, FEOK_Ecom_Entitlement& OutEntitlement)
{
	if(	IOnlineSubsystem* OnlineSub = IOnlineSubsystem::Get("EOK"))
	{
		if (FOnlineSubsystemEOS* EOSRef = static_cast<FOnlineSubsystemEOS*>(OnlineSub))
		{
			EOS_Ecom_CopyEntitlementByIndexOptions CopyEntitlementByIndexOptions = { };
			CopyEntitlementByIndexOptions.ApiVersion = EOS_ECOM_COPYENTITLEMENTBYINDEX_API_LATEST;
			CopyEntitlementByIndexOptions.LocalUserId = LocalUserId.GetValueAsEosType();
			CopyEntitlementByIndexOptions.EntitlementIndex = EntitlementIndex;
			EOS_Ecom_Entitlement* LocalRef;
			auto Result = EOS_Ecom_CopyEntitlementByIndex(EOSRef->EcomHandle, &CopyEntitlementByIndexOptions, &LocalRef);
			OutEntitlement = FEOK_Ecom_Entitlement(*LocalRef);
			return static_cast<EEOK_Result>(Result);
		}
	}
	UE_LOG(LogEOK, Error, TEXT("EOK_Ecom_CopyEntitlementByIndex: OnlineSubsystemEOK is not available"));
	return EEOK_Result::EOS_ServiceFailure;
}

TEnumAsByte<EEOK_Result> UEOK_EcomSubsystem::EOK_Ecom_CopyEntitlementByNameAndIndex(
	FEOK_EpicAccountId LocalUserId, const FString& EntitlementName, int32 Index,
	FEOK_Ecom_Entitlement& OutEntitlement)
{
	if(	IOnlineSubsystem* OnlineSub = IOnlineSubsystem::Get("EOK"))
	{
		if (FOnlineSubsystemEOS* EOSRef = static_cast<FOnlineSubsystemEOS*>(OnlineSub))
		{
			EOS_Ecom_CopyEntitlementByNameAndIndexOptions CopyEntitlementByNameAndIndexOptions = { };
			CopyEntitlementByNameAndIndexOptions.ApiVersion = EOS_ECOM_COPYENTITLEMENTBYNAMEANDINDEX_API_LATEST;
			CopyEntitlementByNameAndIndexOptions.LocalUserId = LocalUserId.GetValueAsEosType();
			CopyEntitlementByNameAndIndexOptions.EntitlementName = TCHAR_TO_ANSI(*EntitlementName);
			CopyEntitlementByNameAndIndexOptions.Index = Index;
			EOS_Ecom_Entitlement* LocalRef;
			auto Result = EOS_Ecom_CopyEntitlementByNameAndIndex(EOSRef->EcomHandle, &CopyEntitlementByNameAndIndexOptions, &LocalRef);
			OutEntitlement = FEOK_Ecom_Entitlement(*LocalRef);
			return static_cast<EEOK_Result>(Result);
		}
	}
	UE_LOG(LogEOK, Error, TEXT("EOK_Ecom_CopyEntitlementByNameAndIndex: OnlineSubsystemEOK is not available"));
	return EEOK_Result::EOS_ServiceFailure;
}

TEnumAsByte<EEOK_Result> UEOK_EcomSubsystem::EOK_Ecom_CopyItemById(FEOK_EpicAccountId LocalUserId,
	const FString& ItemId, FEOK_Ecom_CatalogItem& OutCatalogItem)
{
	if(	IOnlineSubsystem* OnlineSub = IOnlineSubsystem::Get("EOK"))
	{
		if (FOnlineSubsystemEOS* EOSRef = static_cast<FOnlineSubsystemEOS*>(OnlineSub))
		{
			EOS_Ecom_CopyItemByIdOptions CopyItemByIdOptions = { };
			CopyItemByIdOptions.ApiVersion = EOS_ECOM_COPYITEMBYID_API_LATEST;
			CopyItemByIdOptions.LocalUserId = LocalUserId.GetValueAsEosType();
			CopyItemByIdOptions.ItemId = TCHAR_TO_ANSI(*ItemId);
			EOS_Ecom_CatalogItem* LocalRef;
			auto Result = EOS_Ecom_CopyItemById(EOSRef->EcomHandle, &CopyItemByIdOptions, &LocalRef);
			OutCatalogItem = FEOK_Ecom_CatalogItem(*LocalRef);
			return static_cast<EEOK_Result>(Result);
		}
	}
	UE_LOG(LogEOK, Error, TEXT("EOK_Ecom_CopyItemById: OnlineSubsystemEOK is not available"));
	return EEOK_Result::EOS_ServiceFailure;
}

TEnumAsByte<EEOK_Result> UEOK_EcomSubsystem::EOK_Ecom_CopyItemImageInfoByIndex(FEOK_EpicAccountId LocalUserId,
	FEOK_Ecom_CatalogItemId ItemId, int32 ImageInfoIndex, FEOK_Ecom_KeyImageInfo& OutKeyImageInfo)
{
	if(	IOnlineSubsystem* OnlineSub = IOnlineSubsystem::Get("EOK"))
	{
		if (FOnlineSubsystemEOS* EOSRef = static_cast<FOnlineSubsystemEOS*>(OnlineSub))
		{
			EOS_Ecom_CopyItemImageInfoByIndexOptions CopyItemImageInfoByIndexOptions = { };
			CopyItemImageInfoByIndexOptions.ApiVersion = EOS_ECOM_COPYITEMIMAGEINFOBYINDEX_API_LATEST;
			CopyItemImageInfoByIndexOptions.LocalUserId = LocalUserId.GetValueAsEosType();
			CopyItemImageInfoByIndexOptions.ItemId = ItemId.Ref;
			CopyItemImageInfoByIndexOptions.ImageInfoIndex = ImageInfoIndex;
			EOS_Ecom_KeyImageInfo* LocalRef;
			auto Result = EOS_Ecom_CopyItemImageInfoByIndex(EOSRef->EcomHandle, &CopyItemImageInfoByIndexOptions, &LocalRef);
			OutKeyImageInfo = FEOK_Ecom_KeyImageInfo(*LocalRef);
			return static_cast<EEOK_Result>(Result);
		}
	}
	UE_LOG(LogEOK, Error, TEXT("EOK_Ecom_CopyItemImageInfoByIndex: OnlineSubsystemEOK is not available"));
	return EEOK_Result::EOS_ServiceFailure;
}

TEnumAsByte<EEOK_Result> UEOK_EcomSubsystem::EOK_Ecom_CopyItemReleaseByIndex(FEOK_EpicAccountId LocalUserId,
	FEOK_Ecom_CatalogItemId ItemId, int32 ReleaseIndex, FEOK_Ecom_CatalogRelease& OutRelease)
{
	if(	IOnlineSubsystem* OnlineSub = IOnlineSubsystem::Get("EOK"))
	{
		if (FOnlineSubsystemEOS* EOSRef = static_cast<FOnlineSubsystemEOS*>(OnlineSub))
		{
			EOS_Ecom_CopyItemReleaseByIndexOptions CopyItemReleaseByIndexOptions = { };
			CopyItemReleaseByIndexOptions.ApiVersion = EOS_ECOM_COPYITEMRELEASEBYINDEX_API_LATEST;
			CopyItemReleaseByIndexOptions.LocalUserId = LocalUserId.GetValueAsEosType();
			CopyItemReleaseByIndexOptions.ItemId = ItemId.Ref;
			CopyItemReleaseByIndexOptions.ReleaseIndex = ReleaseIndex;
			EOS_Ecom_CatalogRelease* LocalRef;
			auto Result = EOS_Ecom_CopyItemReleaseByIndex(EOSRef->EcomHandle, &CopyItemReleaseByIndexOptions, &LocalRef);
			OutRelease = FEOK_Ecom_CatalogRelease(*LocalRef);
			return static_cast<EEOK_Result>(Result);
		}
	}
	UE_LOG(LogEOK, Error, TEXT("EOK_Ecom_CopyItemReleaseByIndex: OnlineSubsystemEOK is not available"));
	return EEOK_Result::EOS_ServiceFailure;
}

TEnumAsByte<EEOK_Result> UEOK_EcomSubsystem::EOK_Ecom_CopyLastRedeemedEntitlementByIndex(
	FEOK_EpicAccountId LocalUserId, int32 RedeemedEntitlementIndex, FString& OutRedeemedEntitlementId)
{
	if(	IOnlineSubsystem* OnlineSub = IOnlineSubsystem::Get("EOK"))
	{
		if (FOnlineSubsystemEOS* EOSRef = static_cast<FOnlineSubsystemEOS*>(OnlineSub))
		{
			EOS_Ecom_CopyLastRedeemedEntitlementByIndexOptions CopyLastRedeemedEntitlementByIndexOptions = { };
			CopyLastRedeemedEntitlementByIndexOptions.ApiVersion = EOS_ECOM_COPYLASTREDEEMEDENTITLEMENTBYINDEX_API_LATEST;
			CopyLastRedeemedEntitlementByIndexOptions.LocalUserId = LocalUserId.GetValueAsEosType();
			CopyLastRedeemedEntitlementByIndexOptions.RedeemedEntitlementIndex = RedeemedEntitlementIndex;
			char OutRedeemedEntitlementIdLocal[EOS_ECOM_ENTITLEMENTID_MAX_LENGTH];
			int32_t* InOutRedeemedEntitlementIdLength = nullptr;
			auto Result = EOS_Ecom_CopyLastRedeemedEntitlementByIndex(EOSRef->EcomHandle, &CopyLastRedeemedEntitlementByIndexOptions, OutRedeemedEntitlementIdLocal, InOutRedeemedEntitlementIdLength);
			OutRedeemedEntitlementId = OutRedeemedEntitlementIdLocal;
			return static_cast<EEOK_Result>(Result);
		}
	}
	UE_LOG(LogEOK, Error, TEXT("EOK_Ecom_CopyLastRedeemedEntitlementByIndex: OnlineSubsystemEOK is not available"));
	return EEOK_Result::EOS_ServiceFailure;
}

TEnumAsByte<EEOK_Result> UEOK_EcomSubsystem::EOK_Ecom_CopyOfferById(FEOK_EpicAccountId LocalUserId,
	const FEOK_Ecom_CatalogOfferId& OfferId, FEOK_Ecom_CatalogOffer& OutCatalogOffer)
{
	if(	IOnlineSubsystem* OnlineSub = IOnlineSubsystem::Get("EOK"))
	{
		if (FOnlineSubsystemEOS* EOSRef = static_cast<FOnlineSubsystemEOS*>(OnlineSub))
		{
			EOS_Ecom_CopyOfferByIdOptions CopyOfferByIdOptions = { };
			CopyOfferByIdOptions.ApiVersion = EOS_ECOM_COPYOFFERBYID_API_LATEST;
			CopyOfferByIdOptions.LocalUserId = LocalUserId.GetValueAsEosType();
			CopyOfferByIdOptions.OfferId = OfferId.CatalogOfferId;
			EOS_Ecom_CatalogOffer* LocalRef;
			auto Result = EOS_Ecom_CopyOfferById(EOSRef->EcomHandle, &CopyOfferByIdOptions, &LocalRef);
			OutCatalogOffer = *LocalRef;
			return static_cast<EEOK_Result>(Result);
		}
	}
	UE_LOG(LogEOK, Error, TEXT("EOK_Ecom_CopyOfferById: OnlineSubsystemEOK is not available"));
	return EEOK_Result::EOS_ServiceFailure;
}

TEnumAsByte<EEOK_Result> UEOK_EcomSubsystem::EOK_Ecom_CopyOfferImageInfoByIndex(FEOK_EpicAccountId LocalUserId,
	const FEOK_Ecom_CatalogOfferId& OfferId, int32 ImageInfoIndex, FEOK_Ecom_KeyImageInfo& OutKeyImageInfo)
{
	if(	IOnlineSubsystem* OnlineSub = IOnlineSubsystem::Get("EOK"))
	{
		if (FOnlineSubsystemEOS* EOSRef = static_cast<FOnlineSubsystemEOS*>(OnlineSub))
		{
			EOS_Ecom_CopyOfferImageInfoByIndexOptions CopyOfferImageInfoByIndexOptions = { };
			CopyOfferImageInfoByIndexOptions.ApiVersion = EOS_ECOM_COPYOFFERIMAGEINFOBYINDEX_API_LATEST;
			CopyOfferImageInfoByIndexOptions.LocalUserId = LocalUserId.GetValueAsEosType();
			CopyOfferImageInfoByIndexOptions.OfferId = OfferId.CatalogOfferId;
			CopyOfferImageInfoByIndexOptions.ImageInfoIndex = ImageInfoIndex;
			EOS_Ecom_KeyImageInfo* LocalRef;
			auto Result = EOS_Ecom_CopyOfferImageInfoByIndex(EOSRef->EcomHandle, &CopyOfferImageInfoByIndexOptions, &LocalRef);
			OutKeyImageInfo = *LocalRef;
			return static_cast<EEOK_Result>(Result);
		}
	}
	UE_LOG(LogEOK, Error, TEXT("EOK_Ecom_CopyOfferImageInfoByIndex: OnlineSubsystemEOK is not available"));
	return EEOK_Result::EOS_ServiceFailure;
}

TEnumAsByte<EEOK_Result> UEOK_EcomSubsystem::EOK_Ecom_CopyOfferItemByIndex(FEOK_EpicAccountId LocalUserId,
	const FEOK_Ecom_CatalogOfferId& OfferId, int32 ItemIndex, FEOK_Ecom_CatalogItem& OutCatalogItem)
{
	if(	IOnlineSubsystem* OnlineSub = IOnlineSubsystem::Get("EOK"))
	{
		if (FOnlineSubsystemEOS* EOSRef = static_cast<FOnlineSubsystemEOS*>(OnlineSub))
		{
			EOS_Ecom_CopyOfferItemByIndexOptions CopyOfferItemByIndexOptions = { };
			CopyOfferItemByIndexOptions.ApiVersion = EOS_ECOM_COPYOFFERITEMBYINDEX_API_LATEST;
			CopyOfferItemByIndexOptions.LocalUserId = LocalUserId.GetValueAsEosType();
			CopyOfferItemByIndexOptions.OfferId = OfferId.CatalogOfferId;
			CopyOfferItemByIndexOptions.ItemIndex = ItemIndex;
			EOS_Ecom_CatalogItem* LocalRef;
			auto Result = EOS_Ecom_CopyOfferItemByIndex(EOSRef->EcomHandle, &CopyOfferItemByIndexOptions, &LocalRef);
			OutCatalogItem = *LocalRef;
			return static_cast<EEOK_Result>(Result);
		}
	}
	UE_LOG(LogEOK, Error, TEXT("EOK_Ecom_CopyOfferItemByIndex: OnlineSubsystemEOK is not available"));
	return EEOK_Result::EOS_ServiceFailure;
}

TEnumAsByte<EEOK_Result> UEOK_EcomSubsystem::EOK_Ecom_CopyOfferByIndex(FEOK_EpicAccountId LocalUserId,
	int32 OfferIndex, FEOK_Ecom_CatalogOffer& OutCatalogOffer)
{
	if(	IOnlineSubsystem* OnlineSub = IOnlineSubsystem::Get("EOK"))
	{
		if (FOnlineSubsystemEOS* EOSRef = static_cast<FOnlineSubsystemEOS*>(OnlineSub))
		{
			EOS_Ecom_CopyOfferByIndexOptions CopyOfferByIndexOptions = { };
			CopyOfferByIndexOptions.ApiVersion = EOS_ECOM_COPYOFFERBYINDEX_API_LATEST;
			CopyOfferByIndexOptions.LocalUserId = LocalUserId.GetValueAsEosType();
			CopyOfferByIndexOptions.OfferIndex = OfferIndex;
			EOS_Ecom_CatalogOffer* LocalRef;
			auto Result = EOS_Ecom_CopyOfferByIndex(EOSRef->EcomHandle, &CopyOfferByIndexOptions, &LocalRef);
			OutCatalogOffer = *LocalRef;
			return static_cast<EEOK_Result>(Result);
		}
	}
	UE_LOG(LogEOK, Error, TEXT("EOK_Ecom_CopyOfferByIndex: OnlineSubsystemEOK is not available"));
	return EEOK_Result::EOS_ServiceFailure;
}

TEnumAsByte<EEOK_Result> UEOK_EcomSubsystem::EOK_Ecom_CopyTransactionById(FEOK_EpicAccountId LocalUserId,
                                                                          const FString& TransactionId, FEOK_Ecom_HTransaction& OutTransaction)
{
	if(	IOnlineSubsystem* OnlineSub = IOnlineSubsystem::Get("EOK"))
	{
		if (FOnlineSubsystemEOS* EOSRef = static_cast<FOnlineSubsystemEOS*>(OnlineSub))
		{
			EOS_Ecom_CopyTransactionByIdOptions CopyTransactionByIdOptions = { };
			CopyTransactionByIdOptions.ApiVersion = EOS_ECOM_COPYTRANSACTIONBYID_API_LATEST;
			CopyTransactionByIdOptions.LocalUserId = LocalUserId.GetValueAsEosType();
			CopyTransactionByIdOptions.TransactionId = TCHAR_TO_ANSI(*TransactionId);
			EOS_Ecom_HTransaction LocalRef;
			auto Result = EOS_Ecom_CopyTransactionById(EOSRef->EcomHandle, &CopyTransactionByIdOptions, &LocalRef);
			if(Result == EOS_EResult::EOS_Success)
			{
				OutTransaction = LocalRef;
			}
			return static_cast<EEOK_Result>(Result);
		}
	}
	UE_LOG(LogEOK, Error, TEXT("EOK_Ecom_CopyTransactionById: OnlineSubsystemEOK is not available"));
	return EEOK_Result::EOS_ServiceFailure;
}

TEnumAsByte<EEOK_Result> UEOK_EcomSubsystem::EOK_Ecom_CopyTransactionByIndex(FEOK_EpicAccountId LocalUserId,
	int32 TransactionIndex, FEOK_Ecom_HTransaction& OutTransaction)
{
	if(	IOnlineSubsystem* OnlineSub = IOnlineSubsystem::Get("EOK"))
	{
		if (FOnlineSubsystemEOS* EOSRef = static_cast<FOnlineSubsystemEOS*>(OnlineSub))
		{
			EOS_Ecom_CopyTransactionByIndexOptions CopyTransactionByIndexOptions = { };
			CopyTransactionByIndexOptions.ApiVersion = EOS_ECOM_COPYTRANSACTIONBYINDEX_API_LATEST;
			CopyTransactionByIndexOptions.LocalUserId = LocalUserId.GetValueAsEosType();
			CopyTransactionByIndexOptions.TransactionIndex = TransactionIndex;
			EOS_Ecom_HTransaction LocalRef;
			auto Result = EOS_Ecom_CopyTransactionByIndex(EOSRef->EcomHandle, &CopyTransactionByIndexOptions, &LocalRef);
			if(Result == EOS_EResult::EOS_Success)
			{
				OutTransaction = LocalRef;
			}
			return static_cast<EEOK_Result>(Result);
		}
	}
	UE_LOG(LogEOK, Error, TEXT("EOK_Ecom_CopyTransactionByIndex: OnlineSubsystemEOK is not available"));
	return EEOK_Result::EOS_ServiceFailure;
}

void UEOK_EcomSubsystem::EOK_Ecom_Entitlement_Release(FEOK_Ecom_Entitlement Entitlement)
{
	EOS_Ecom_Entitlement_Release(&Entitlement.Ref);
}

int32 UEOK_EcomSubsystem::EOK_Ecom_GetEntitlementsByNameCount(FEOK_EpicAccountId LocalUserId,
	const FEOK_Ecom_EntitlementName& EntitlementName)
{
	if(	IOnlineSubsystem* OnlineSub = IOnlineSubsystem::Get("EOK"))
	{
		if (FOnlineSubsystemEOS* EOSRef = static_cast<FOnlineSubsystemEOS*>(OnlineSub))
		{
			EOS_Ecom_GetEntitlementsByNameCountOptions GetEntitlementsByNameCountOptions = { };
			GetEntitlementsByNameCountOptions.ApiVersion = EOS_ECOM_GETENTITLEMENTSBYNAMECOUNT_API_LATEST;
			GetEntitlementsByNameCountOptions.LocalUserId = LocalUserId.GetValueAsEosType();
			GetEntitlementsByNameCountOptions.EntitlementName = EntitlementName.Ref;
			return EOS_Ecom_GetEntitlementsByNameCount(EOSRef->EcomHandle, &GetEntitlementsByNameCountOptions);
		}
	}
	UE_LOG(LogEOK, Error, TEXT("EOK_Ecom_GetEntitlementsByNameCount: OnlineSubsystemEOK is not available"));
	return -1;
}

int32 UEOK_EcomSubsystem::EOK_Ecom_GetEntitlementsCount(FEOK_EpicAccountId LocalUserId)
{
	if(	IOnlineSubsystem* OnlineSub = IOnlineSubsystem::Get("EOK"))
	{
		if (FOnlineSubsystemEOS* EOSRef = static_cast<FOnlineSubsystemEOS*>(OnlineSub))
		{
			EOS_Ecom_GetEntitlementsCountOptions GetEntitlementsCountOptions = { };
			GetEntitlementsCountOptions.ApiVersion = EOS_ECOM_GETENTITLEMENTSCOUNT_API_LATEST;
			GetEntitlementsCountOptions.LocalUserId = LocalUserId.GetValueAsEosType();
			return EOS_Ecom_GetEntitlementsCount(EOSRef->EcomHandle, &GetEntitlementsCountOptions);
		}
	}
	UE_LOG(LogEOK, Error, TEXT("EOK_Ecom_GetEntitlementsCount: OnlineSubsystemEOK is not available"));
	return -1;
}

int32 UEOK_EcomSubsystem::EOK_Ecom_GetItemImageInfoCount(FEOK_EpicAccountId LocalUserId,
	FEOK_Ecom_CatalogItemId ItemId)
{
	if(	IOnlineSubsystem* OnlineSub = IOnlineSubsystem::Get("EOK"))
	{
		if (FOnlineSubsystemEOS* EOSRef = static_cast<FOnlineSubsystemEOS*>(OnlineSub))
		{
			EOS_Ecom_GetItemImageInfoCountOptions GetItemImageInfoCountOptions = { };
			GetItemImageInfoCountOptions.ApiVersion = EOS_ECOM_GETITEMIMAGEINFOCOUNT_API_LATEST;
			GetItemImageInfoCountOptions.LocalUserId = LocalUserId.GetValueAsEosType();
			GetItemImageInfoCountOptions.ItemId = ItemId.Ref;
			return EOS_Ecom_GetItemImageInfoCount(EOSRef->EcomHandle, &GetItemImageInfoCountOptions);
		}
	}
	UE_LOG(LogEOK, Error, TEXT("EOK_Ecom_GetItemImageInfoCount: OnlineSubsystemEOK is not available"));
	return -1;
}

int32 UEOK_EcomSubsystem::EOK_Ecom_GetItemReleaseCount(FEOK_EpicAccountId LocalUserId,
	FEOK_Ecom_CatalogItemId ItemId)
{
	if(	IOnlineSubsystem* OnlineSub = IOnlineSubsystem::Get("EOK"))
	{
		if (FOnlineSubsystemEOS* EOSRef = static_cast<FOnlineSubsystemEOS*>(OnlineSub))
		{
			EOS_Ecom_GetItemReleaseCountOptions GetItemReleaseCountOptions = { };
			GetItemReleaseCountOptions.ApiVersion = EOS_ECOM_GETITEMRELEASECOUNT_API_LATEST;
			GetItemReleaseCountOptions.LocalUserId = LocalUserId.GetValueAsEosType();
			GetItemReleaseCountOptions.ItemId = ItemId.Ref;
			return EOS_Ecom_GetItemReleaseCount(EOSRef->EcomHandle, &GetItemReleaseCountOptions);
		}
	}
	UE_LOG(LogEOK, Error, TEXT("EOK_Ecom_GetItemReleaseCount: OnlineSubsystemEOK is not available"));
	return -1;
}

int32 UEOK_EcomSubsystem::EOK_Ecom_GetLastRedeemedEntitlementsCount(FEOK_EpicAccountId LocalUserId)
{
	if(	IOnlineSubsystem* OnlineSub = IOnlineSubsystem::Get("EOK"))
	{
		if (FOnlineSubsystemEOS* EOSRef = static_cast<FOnlineSubsystemEOS*>(OnlineSub))
		{
			EOS_Ecom_GetLastRedeemedEntitlementsCountOptions GetLastRedeemedEntitlementsCountOptions = { };
			GetLastRedeemedEntitlementsCountOptions.ApiVersion = EOS_ECOM_GETLASTREDEEMEDENTITLEMENTSCOUNT_API_LATEST;
			GetLastRedeemedEntitlementsCountOptions.LocalUserId = LocalUserId.GetValueAsEosType();
			return EOS_Ecom_GetLastRedeemedEntitlementsCount(EOSRef->EcomHandle, &GetLastRedeemedEntitlementsCountOptions);
		}
	}
	UE_LOG(LogEOK, Error, TEXT("EOK_Ecom_GetLastRedeemedEntitlementsCount: OnlineSubsystemEOK is not available"));
	return -1;
}

int32 UEOK_EcomSubsystem::EOK_Ecom_GetOfferCount(FEOK_EpicAccountId LocalUserId)
{
	if(	IOnlineSubsystem* OnlineSub = IOnlineSubsystem::Get("EOK"))
	{
		if (FOnlineSubsystemEOS* EOSRef = static_cast<FOnlineSubsystemEOS*>(OnlineSub))
		{
			EOS_Ecom_GetOfferCountOptions GetOfferCountOptions = { };
			GetOfferCountOptions.ApiVersion = EOS_ECOM_GETOFFERCOUNT_API_LATEST;
			GetOfferCountOptions.LocalUserId = LocalUserId.GetValueAsEosType();
			return EOS_Ecom_GetOfferCount(EOSRef->EcomHandle, &GetOfferCountOptions);
		}
	}
	UE_LOG(LogEOK, Error, TEXT("EOK_Ecom_GetOfferCount: OnlineSubsystemEOK is not available"));
	return -1;
}

int32 UEOK_EcomSubsystem::EOK_Ecom_GetOfferImageInfoCount(FEOK_EpicAccountId LocalUserId,
	const FEOK_Ecom_CatalogOfferId& OfferId)
{
	if(	IOnlineSubsystem* OnlineSub = IOnlineSubsystem::Get("EOK"))
	{
		if (FOnlineSubsystemEOS* EOSRef = static_cast<FOnlineSubsystemEOS*>(OnlineSub))
		{
			EOS_Ecom_GetOfferImageInfoCountOptions GetOfferImageInfoCountOptions = { };
			GetOfferImageInfoCountOptions.ApiVersion = EOS_ECOM_GETOFFERIMAGEINFOCOUNT_API_LATEST;
			GetOfferImageInfoCountOptions.LocalUserId = LocalUserId.GetValueAsEosType();
			GetOfferImageInfoCountOptions.OfferId = OfferId.CatalogOfferId;
			return EOS_Ecom_GetOfferImageInfoCount(EOSRef->EcomHandle, &GetOfferImageInfoCountOptions);
		}
	}
	UE_LOG(LogEOK, Error, TEXT("EOK_Ecom_GetOfferImageInfoCount: OnlineSubsystemEOK is not available"));
	return -1;
}

int32 UEOK_EcomSubsystem::EOK_Ecom_GetOfferItemCount(FEOK_EpicAccountId LocalUserId,
	const FEOK_Ecom_CatalogOfferId& OfferId)
{
	if(	IOnlineSubsystem* OnlineSub = IOnlineSubsystem::Get("EOK"))
	{
		if (FOnlineSubsystemEOS* EOSRef = static_cast<FOnlineSubsystemEOS*>(OnlineSub))
		{
			EOS_Ecom_GetOfferItemCountOptions GetOfferItemCountOptions = { };
			GetOfferItemCountOptions.ApiVersion = EOS_ECOM_GETOFFERITEMCOUNT_API_LATEST;
			GetOfferItemCountOptions.LocalUserId = LocalUserId.GetValueAsEosType();
			GetOfferItemCountOptions.OfferId = OfferId.CatalogOfferId;
			return EOS_Ecom_GetOfferItemCount(EOSRef->EcomHandle, &GetOfferItemCountOptions);
		}
	}
	UE_LOG(LogEOK, Error, TEXT("EOK_Ecom_GetOfferItemCount: OnlineSubsystemEOK is not available"));
	return -1;
}

int32 UEOK_EcomSubsystem::EOK_Ecom_GetTransactionCount(FEOK_EpicAccountId LocalUserId)
{
	if(	IOnlineSubsystem* OnlineSub = IOnlineSubsystem::Get("EOK"))
	{
		if (FOnlineSubsystemEOS* EOSRef = static_cast<FOnlineSubsystemEOS*>(OnlineSub))
		{
			EOS_Ecom_GetTransactionCountOptions GetTransactionCountOptions = { };
			GetTransactionCountOptions.ApiVersion = EOS_ECOM_GETTRANSACTIONCOUNT_API_LATEST;
			GetTransactionCountOptions.LocalUserId = LocalUserId.GetValueAsEosType();
			return EOS_Ecom_GetTransactionCount(EOSRef->EcomHandle, &GetTransactionCountOptions);
		}
	}
	UE_LOG(LogEOK, Error, TEXT("EOK_Ecom_GetTransactionCount: OnlineSubsystemEOK is not available"));
	return -1;
}

void UEOK_EcomSubsystem::EOK_Ecom_KeyImageInfo_Release(FEOK_Ecom_KeyImageInfo KeyImageInfo)
{
	EOS_Ecom_KeyImageInfo_Release(&KeyImageInfo.Ref);
}

TEnumAsByte<EEOK_Result> UEOK_EcomSubsystem::EOK_Ecom_Transaction_CopyEntitlementByIndex(int32 EntitlementIndex,
	FEOK_Ecom_HTransaction Transaction, FEOK_Ecom_Entitlement& OutEntitlement)
{
	if(	IOnlineSubsystem* OnlineSub = IOnlineSubsystem::Get("EOK"))
	{
		if (FOnlineSubsystemEOS* EOSRef = static_cast<FOnlineSubsystemEOS*>(OnlineSub))
		{
			if(Transaction.Ref == nullptr)
			{
				UE_LOG(LogEOK, Error, TEXT("EOK_Ecom_Transaction_CopyEntitlementByIndex: Transaction is not valid"));
				return EEOK_Result::EOS_InvalidParameters;
			}
			EOS_Ecom_Transaction_CopyEntitlementByIndexOptions CopyEntitlementByIndexOptions = { };
			CopyEntitlementByIndexOptions.ApiVersion = EOS_ECOM_TRANSACTION_COPYENTITLEMENTBYINDEX_API_LATEST;
			CopyEntitlementByIndexOptions.EntitlementIndex = EntitlementIndex;
			EOS_Ecom_Entitlement* LocalRef;
			auto Result = EOS_Ecom_Transaction_CopyEntitlementByIndex(Transaction.Ref, &CopyEntitlementByIndexOptions, &LocalRef);
			if(Result == EOS_EResult::EOS_Success)
			{
				OutEntitlement = FEOK_Ecom_Entitlement(*LocalRef);
			}
			return static_cast<EEOK_Result>(Result);
		}
	}
	UE_LOG(LogEOK, Error, TEXT("EOK_Ecom_Transaction_CopyEntitlementByIndex: OnlineSubsystemEOK is not available"));
	return EEOK_Result::EOS_ServiceFailure;
}

int32 UEOK_EcomSubsystem::EOK_Ecom_Transaction_GetEntitlementsCount(FEOK_Ecom_HTransaction Transaction)
{
	if(Transaction.Ref == nullptr)
	{
		UE_LOG(LogEOK, Error, TEXT("EOK_Ecom_Transaction_GetEntitlementsCount: Transaction is not valid"));
		return -1;
	}
	EOS_Ecom_Transaction_GetEntitlementsCountOptions GetEntitlementsCountOptions = { };
	GetEntitlementsCountOptions.ApiVersion = EOS_ECOM_TRANSACTION_GETENTITLEMENTSCOUNT_API_LATEST;
	return EOS_Ecom_Transaction_GetEntitlementsCount(Transaction.Ref, &GetEntitlementsCountOptions);
}

FString UEOK_EcomSubsystem::EOK_Ecom_Transaction_GetTransactionId(FEOK_Ecom_HTransaction Transaction)
{
	if(Transaction.Ref == nullptr)
	{
		UE_LOG(LogEOK, Error, TEXT("EOK_Ecom_Transaction_GetTransactionId: Transaction is not valid"));
		return "";
	}
	char* OutBuffer = nullptr;
	int32_t* InOutBufferLength = nullptr;
	auto Result = EOS_Ecom_Transaction_GetTransactionId(Transaction.Ref, OutBuffer, InOutBufferLength);
	if(Result == EOS_EResult::EOS_Success)
	{
		FString OutString = FString(OutBuffer);
		EOS_Ecom_Transaction_Release(Transaction.Ref);
		return OutString;
	}
	return "";
}
