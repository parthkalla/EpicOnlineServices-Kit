// Copyright (C) 2024, All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintAsyncActionBase.h"
#include "EOSKitSessionStructs.h"
#include "EOSGetSessionAttributeAsync.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FGetSessionAttribute_Delegate, bool, bSuccess, const FEOSKitAttribute&, AttributeValue);

/**
 * Get Session Attribute - Retrieve a specific session attribute value
 */
UCLASS()
class EOSKITSESSIONS_API UEOSGetSessionAttributeAsync : public UBlueprintAsyncActionBase
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintAssignable, DisplayName="On Complete")
	FGetSessionAttribute_Delegate OnComplete;

	/**
	 * Get a session attribute by key
	 * 
	 * @param SessionName - Name of the session
	 * @param AttributeKey - Key of the attribute to retrieve
	 */
	UFUNCTION(BlueprintCallable, DisplayName="Get Session Attribute", meta = (BlueprintInternalUseOnly = "true"), Category="EOSKit|Sessions")
	static UEOSGetSessionAttributeAsync* GetSessionAttribute(FName SessionName, FString AttributeKey);

	virtual void Activate() override;

private:
	void GetAttribute();
	
	FName VSessionName;
	FString AttributeKey;
};
