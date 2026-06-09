// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintAsyncActionBase.h"
#include "Runtime/Launch/Resources/Version.h"
#include "EOK_PurchaseItem_AsyncFunction.generated.h"


DECLARE_DYNAMIC_MULTICAST_DELEGATE(FPurchaseItemResult);

UCLASS()
class OnlineSubsystemEOK_API UEOK_PurchaseItem_AsyncFunction : public UBlueprintAsyncActionBase
{
	GENERATED_BODY()

public:

	FString ItemID;

	UPROPERTY(BlueprintAssignable)
	FPurchaseItemResult OnSuccess;
	
	UPROPERTY(BlueprintAssignable)
	FPurchaseItemResult OnFail;

	bool bDelegateCalled = false;
	
	/*
	This C++ method purchases an item in the online subsystem using the selected method and sets up a callback function to handle the response.
	Documentation link: https://betide-studio.gitbook.io/eos-integration-kit/sessions/
	*/
	UFUNCTION(BlueprintCallable, DisplayName="Purchase EOK Item", meta = (BlueprintInternalUseOnly = "true"), Category="Epic Online Services-Kit V2 || Store")
	static UEOK_PurchaseItem_AsyncFunction* PurchaseItem(FString ItemID);

	 void Activate() override;

	void PurchaseItem();
	
	
};
