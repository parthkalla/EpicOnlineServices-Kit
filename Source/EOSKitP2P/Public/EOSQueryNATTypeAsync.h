// Copyright (C) 2024, All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintAsyncActionBase.h"
#include "EOSKitP2PTypes.h"
#include "EOSQueryNATTypeAsync.generated.h"

/**
 * Delegate for query NAT type completion
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnEOSQueryNATTypeComplete, EEOSKitNATType, NATType);

/**
 * Async node to query NAT type
 */
UCLASS()
class EOSKITP2P_API UEOSQueryNATTypeAsync : public UBlueprintAsyncActionBase
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintAssignable, Category = "EOSKit|P2P")
	FOnEOSQueryNATTypeComplete OnSuccess;

	UPROPERTY(BlueprintAssignable, Category = "EOSKit|P2P")
	FOnEOSQueryNATTypeComplete OnFailure;

	/**
	 * Query NAT type
	 * @param WorldContextObject - World context object
	 * @return Async node instance
	 */
	UFUNCTION(BlueprintCallable, DisplayName = "Query NAT Type",
		meta = (BlueprintInternalUseOnly = "true", WorldContext = "WorldContextObject",
		ToolTip = "Query the current NAT type of the connection."),
		Category = "EOSKit|P2P")
	static UEOSQueryNATTypeAsync* QueryNATType(UObject* WorldContextObject);

	virtual void Activate() override;

private:
	static void EOS_CALL OnQueryNATTypeComplete(const EOS_P2P_OnQueryNATTypeCompleteInfo* Data);

	UObject* WorldContextObject;
};

