// Copyright (c) 2023 Betide Studio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintAsyncActionBase.h"
#include "OnlineSubsystemEOK/SdkFunctions/EOK_SharedFunctionFile.h"
#include "EOK_Ecom_QueryOwnershipToken.generated.h"

USTRUCT(BlueprintType)
struct FEOK_Ecom_QueryOwnershipTokenOptions
{
	GENERATED_BODY()

	//The Epic Account ID of the local user whose ownership token you want to query
	UPROPERTY(BlueprintReadWrite, Category = "Epic Online Services-Kit V2 | SDK Functions | Ecom Interface")
	FEOK_EpicAccountId LocalUserId;

	//The array of Catalog Item IDs to check for ownership, matching in number to the CatalogItemIdCount
	UPROPERTY(BlueprintReadWrite, Category = "Epic Online Services-Kit V2 | SDK Functions | Ecom Interface")
	TArray<FEOK_Ecom_CatalogItemId> CatalogItemIds;

	//Optional product namespace, if not the one specified during initialization
	UPROPERTY(BlueprintReadWrite, Category = "Epic Online Services-Kit V2 | SDK Functions | Ecom Interface")
	FString CatalogNamespace;

	FEOK_Ecom_QueryOwnershipTokenOptions()
	{
		LocalUserId = FEOK_EpicAccountId();
		CatalogItemIds = TArray<FEOK_Ecom_CatalogItemId>();
		CatalogNamespace = "";
	}
	EOS_Ecom_QueryOwnershipTokenOptions ToEOS_Ecom_QueryOwnershipOptions()
	{
		EOS_Ecom_QueryOwnershipTokenOptions Options;
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
struct FEOK_Ecom_QueryOwnershipTokenCallbackInfo
{
	GENERATED_BODY()

	//The EOS_EResult code for the operation. EOS_Success indicates that the operation succeeded; other codes indicate errors.
	UPROPERTY(BlueprintReadWrite, Category = "Epic Online Services-Kit V2 | SDK Functions | Ecom Interface")
	TEnumAsByte<EEOK_Result> ResultCode;

	//The Epic Account ID of the local user whose ownership token was queried
	UPROPERTY(BlueprintReadWrite, Category = "Epic Online Services-Kit V2 | SDK Functions | Ecom Interface")
	FEOK_EpicAccountId LocalUserId;

	//Ownership token containing details about the catalog items queried
	UPROPERTY(BlueprintReadWrite, Category = "Epic Online Services-Kit V2 | SDK Functions | Ecom Interface")
	FString OwnershipToken;

	FEOK_Ecom_QueryOwnershipTokenCallbackInfo()
	{
		ResultCode = EEOK_Result::EOS_ServiceFailure;
		LocalUserId = FEOK_EpicAccountId();
		OwnershipToken = "";
	}
	FEOK_Ecom_QueryOwnershipTokenCallbackInfo(const EOS_Ecom_QueryOwnershipTokenCallbackInfo& Data)
	{
		ResultCode = static_cast<EEOK_Result>(Data.ResultCode);
		LocalUserId = Data.LocalUserId;
		OwnershipToken = Data.OwnershipToken;
	}
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FEOK_Ecom_QueryOwnershipTokenCallback, const FEOK_Ecom_QueryOwnershipTokenCallbackInfo&, Data);

UCLASS()
class OnlineSubsystemEOK_API UEOK_Ecom_QueryOwnershipToken : public UBlueprintAsyncActionBase
{
	GENERATED_BODY()
public:

	//Query the ownership status for a given list of catalog item IDs defined with Epic Online Services. The data is return via the callback in the form of a signed JWT that should be verified by an external backend server using a public key for authenticity.
	UFUNCTION(BlueprintCallable, Category = "Epic Online Services-Kit V2 | SDK Functions | Ecom Interface", DisplayName = "EOS_Ecom_QueryOwnershipToken")
	static UEOK_Ecom_QueryOwnershipToken* EOK_Ecom_QueryOwnershipToken(FEOK_Ecom_QueryOwnershipTokenOptions QueryOwnershipTokenOptions);

	UPROPERTY(BlueprintAssignable, Category = "Epic Online Services-Kit V2 | SDK Functions | Ecom Interface")
	FEOK_Ecom_QueryOwnershipTokenCallback OnCallback;
private:
	FEOK_Ecom_QueryOwnershipTokenOptions Var_QueryOwnershipTokenOptions;
	static void EOS_CALL OnQueryOwnershipTokenCallback(const EOS_Ecom_QueryOwnershipTokenCallbackInfo* Data);
	virtual void Activate() override;
};
