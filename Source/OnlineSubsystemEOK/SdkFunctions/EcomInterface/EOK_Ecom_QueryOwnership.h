// Copyright (c) 2023 Betide Studio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintAsyncActionBase.h"
#include "OnlineSubsystemEOK/SdkFunctions/EOK_SharedFunctionFile.h"
#include "EOK_Ecom_QueryOwnership.generated.h"

USTRUCT(BlueprintType)
struct FEOK_Ecom_QueryOwnershipOptions
{
	GENERATED_BODY()

	//The Epic Account ID of the local user whose ownership to query
	UPROPERTY(BlueprintReadWrite, Category = "Epic Online Services-Kit V2 | SDK Functions | Ecom Interface")
	FEOK_EpicAccountId LocalUserId;

	//The array of Catalog Item IDs to check for ownership
	UPROPERTY(BlueprintReadWrite, Category = "Epic Online Services-Kit V2 | SDK Functions | Ecom Interface")
	TArray<FEOK_Ecom_CatalogItemId> CatalogItemIds;

	//Optional product namespace, if not the one specified during initialization
	UPROPERTY(BlueprintReadWrite, Category = "Epic Online Services-Kit V2 | SDK Functions | Ecom Interface")
	FString CatalogNamespace;
	
	FEOK_Ecom_QueryOwnershipOptions()
	{
		LocalUserId = FEOK_EpicAccountId();
		CatalogItemIds = TArray<FEOK_Ecom_CatalogItemId>();
		CatalogNamespace = "";
	}
	EOS_Ecom_QueryOwnershipOptions ToEOS_Ecom_QueryOwnershipOptions()
	{
		EOS_Ecom_QueryOwnershipOptions Options;
		Options.ApiVersion = EOS_ECOM_QUERYOWNERSHIP_API_LATEST;
		Options.LocalUserId = LocalUserId.GetValueAsEosType();
		Options.CatalogItemIdCount = CatalogItemIds.Num();
		Options.CatalogItemIds = new const char*[CatalogItemIds.Num()];
		for (int i = 0; i < CatalogItemIds.Num(); i++)
		{
			Options.CatalogItemIds[i] = CatalogItemIds[i].Ref;
		}
		Options.CatalogNamespace = TCHAR_TO_ANSI(*CatalogNamespace);
		return Options;
	}
};


USTRUCT(BlueprintType)
struct FEOK_Ecom_QueryOwnershipCallbackInfo
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Epic Online Services-Kit V2 | SDK Functions | Ecom Interface")
	FEOK_EpicAccountId LocalUserId;
	
	UPROPERTY(BlueprintReadOnly, Category = "Epic Online Services-Kit V2 | SDK Functions | Ecom Interface")
	TEnumAsByte<EEOK_Result> ResultCode;

	UPROPERTY(BlueprintReadOnly, Category = "Epic Online Services-Kit V2 | SDK Functions | Ecom Interface")
	TArray<FEOK_Ecom_ItemOwnership> ItemOwnership;
	
	FEOK_Ecom_QueryOwnershipCallbackInfo()
	{
		LocalUserId = FEOK_EpicAccountId();
		ResultCode = EEOK_Result::EOS_ServiceFailure;
		ItemOwnership = TArray<FEOK_Ecom_ItemOwnership>();
	}
	FEOK_Ecom_QueryOwnershipCallbackInfo(const EOS_Ecom_QueryOwnershipCallbackInfo* Data)
	{
		LocalUserId = Data->LocalUserId;
		ResultCode = static_cast<EEOK_Result>(Data->ResultCode);
		ItemOwnership = TArray<FEOK_Ecom_ItemOwnership>();
		int32 ItemOwnershipCount = Data->ItemOwnershipCount;
		for (int i = 0; i < ItemOwnershipCount; i++)
		{
			ItemOwnership.Add(Data->ItemOwnership[i]);
		}
	}
	
};
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FEOK_Ecom_QueryOwnershipCallback, const FEOK_Ecom_QueryOwnershipCallbackInfo&, Data);
UCLASS()
class OnlineSubsystemEOK_API UEOK_Ecom_QueryOwnership : public UBlueprintAsyncActionBase
{
	GENERATED_BODY()

public:

	//Query the ownership status for a given list of catalog item IDs defined with Epic Online Services. This data will be cached for a limited time and retrieved again from the backend when necessary Depending on the number of catalog item ids passed, the SDK splits the query into smaller batch requests to the backend and aggregates the result. Note: If one of the request batches fails, no data is cached and the entire query is marked as failed.
	UFUNCTION(BlueprintCallable, Category = "Epic Online Services-Kit V2 | SDK Functions | Ecom Interface", DisplayName = "EOS_Ecom_QueryOwnership")
	static UEOK_Ecom_QueryOwnership* EOK_Ecom_QueryOwnership(FEOK_Ecom_QueryOwnershipOptions QueryOwnershipOptions);

	UPROPERTY(BlueprintAssignable)
	FEOK_Ecom_QueryOwnershipCallback OnCallback;
private:
	FEOK_Ecom_QueryOwnershipOptions Var_QueryOwnershipOptions;
	static void EOS_CALL OnQueryOwnershipCallback(const EOS_Ecom_QueryOwnershipCallbackInfo* Data);
	virtual void Activate() override;
};
