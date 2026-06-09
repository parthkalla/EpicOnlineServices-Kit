// Copyright (c) 2023 Betide Studio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintAsyncActionBase.h"
#include "OnlineSubsystemEOK/SdkFunctions/EOK_SharedFunctionFile.h"
#include "EOK_Ecom_QueryOwnershipBySandboxIds.generated.h"

USTRUCT(BlueprintType)
struct FEOK_Ecom_QueryOwnershipBySandboxIdsOptions
{
	GENERATED_BODY()

	//The Epic Account ID of the local user whose ownership to query.
	UPROPERTY(BlueprintReadWrite, Category = "Epic Online Services-Kit V2 | SDK Functions | Ecom Interface")
	FEOK_EpicAccountId LocalUserId;

	//The array of Sandbox IDs to check for ownership.
	UPROPERTY(BlueprintReadWrite, Category = "Epic Online Services-Kit V2 | SDK Functions | Ecom Interface")
	TArray<FEOK_Ecom_SandboxId> SandboxIds;

	FEOK_Ecom_QueryOwnershipBySandboxIdsOptions()
	{
		LocalUserId = FEOK_EpicAccountId();
		SandboxIds = TArray<FEOK_Ecom_SandboxId>();
	}
	EOS_Ecom_QueryOwnershipBySandboxIdsOptions ToEOS_Ecom_QueryOwnershipBySandboxIdsOptions()
	{
		EOS_Ecom_QueryOwnershipBySandboxIdsOptions Options;
		Options.ApiVersion = EOS_ECOM_QUERYOWNERSHIPBYSANDBOXIDSOPTIONS_API_LATEST;
		Options.LocalUserId = LocalUserId.GetValueAsEosType();
		Options.SandboxIdsCount = SandboxIds.Num();
		Options.SandboxIds = new const char*[SandboxIds.Num()];
		for (int i = 0; i < SandboxIds.Num(); i++)
		{
			Options.SandboxIds[i] = SandboxIds[i].Ref;
		}
		return Options;
	}
};

USTRUCT(BlueprintType)
struct FEOK_Ecom_QueryOwnershipBySandboxIdsCallbackInfo
{
	GENERATED_BODY()

	//The EOS_EResult code for the operation. EOS_Success indicates that the operation succeeded; other codes indicate errors.
	UPROPERTY(BlueprintReadOnly, Category = "Epic Online Services-Kit V2 | SDK Functions | Ecom Interface")
	TEnumAsByte<EEOK_Result> ResultCode;

	//The Epic Account ID of the local user whose ownership was queried
	UPROPERTY(BlueprintReadOnly, Category = "Epic Online Services-Kit V2 | SDK Functions | Ecom Interface")
	FEOK_EpicAccountId LocalUserId;

	//List of SandboxIds and their corresponding owned catalog item Ids. If there are no ownership items, the OwnedCatalogItemIdsCount is 0 and OwnedCatalogItemIds is null.
	UPROPERTY(BlueprintReadOnly, Category = "Epic Online Services-Kit V2 | SDK Functions | Ecom Interface")
	TArray<FEOK_Ecom_SandboxIdItemOwnership> SandboxIdItemOwnerships;

	FEOK_Ecom_QueryOwnershipBySandboxIdsCallbackInfo()
	{
		ResultCode = EEOK_Result::EOS_ServiceFailure;
		LocalUserId = FEOK_EpicAccountId();
		SandboxIdItemOwnerships = TArray<FEOK_Ecom_SandboxIdItemOwnership>();
	}
	FEOK_Ecom_QueryOwnershipBySandboxIdsCallbackInfo(const EOS_Ecom_QueryOwnershipBySandboxIdsCallbackInfo& Data)
	{
		ResultCode = static_cast<EEOK_Result>(Data.ResultCode);
		LocalUserId = Data.LocalUserId;
		int32 SandboxIdItemOwnershipsCount = Data.SandboxIdItemOwnershipsCount;
		for (int i = 0; i < SandboxIdItemOwnershipsCount; i++)
		{
			SandboxIdItemOwnerships.Add(Data.SandboxIdItemOwnerships[i]);
		}
	}
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FEOK_Ecom_QueryOwnershipBySandboxIdsCallback, const FEOK_Ecom_QueryOwnershipBySandboxIdsCallbackInfo&, Data);
UCLASS()
class OnlineSubsystemEOK_API UEOK_Ecom_QueryOwnershipBySandboxIds : public UBlueprintAsyncActionBase
{
	GENERATED_BODY()

public:

	//Query the ownership status of all catalog item IDs under the given list of Sandbox IDs defined with Epic Online Services. This data will be cached for a limited time and retrieved again from the backend when necessary.
	UFUNCTION(BlueprintCallable, Category = "Epic Online Services-Kit V2 | SDK Functions | Ecom Interface", DisplayName = "EOS_Ecom_QueryOwnershipBySandboxIds")
	static UEOK_Ecom_QueryOwnershipBySandboxIds* EOK_Ecom_QueryOwnershipBySandboxIds(FEOK_Ecom_QueryOwnershipBySandboxIdsOptions QueryOwnershipBySandboxIdsOptions);

	UPROPERTY(BlueprintAssignable, Category = "Epic Online Services-Kit V2 | SDK Functions | Ecom Interface")
	FEOK_Ecom_QueryOwnershipBySandboxIdsCallback OnCallback;

private:
	static void EOS_CALL OnQueryOwnershipBySandboxIdsCallback(const EOS_Ecom_QueryOwnershipBySandboxIdsCallbackInfo* Data);
	virtual void Activate() override;
	FEOK_Ecom_QueryOwnershipBySandboxIdsOptions Var_QueryOwnershipBySandboxIdsOptions;
};
