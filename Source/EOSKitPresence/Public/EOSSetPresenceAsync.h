// Copyright (C) 2024, All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintAsyncActionBase.h"
#include "EOSKitPresenceTypes.h"
#include "EOSSetPresenceAsync.generated.h"

#if WITH_EOS_SDK
#include "Windows/AllowWindowsPlatformTypes.h"
#include "eos_presence_types.h"
#include "Windows/HideWindowsPlatformTypes.h"
#endif

/**
 * Delegate for set presence completion
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnEOSSetPresenceComplete, const FString&, RichPresence, EEOSKitPresenceStatus, PresenceStatus);

/**
 * Async node to set presence status and rich text
 */
UCLASS()
class EOSKITPRESENCE_API UEOSSetPresenceAsync : public UBlueprintAsyncActionBase
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintAssignable, Category = "EOSKit|Presence")
	FOnEOSSetPresenceComplete OnSuccess;

	UPROPERTY(BlueprintAssignable, Category = "EOSKit|Presence")
	FOnEOSSetPresenceComplete OnFailure;

	/**
	 * Set presence status and rich text for the logged-in user
	 * @param WorldContextObject - World context object
	 * @param RichPresence - Rich text presence string
	 * @param PresenceStatus - Presence status enum
	 * @return Async node instance
	 */
	UFUNCTION(BlueprintCallable, DisplayName = "Set EOS Presence",
		meta = (BlueprintInternalUseOnly = "true", WorldContext = "WorldContextObject",
		ToolTip = "Set presence status and rich text for the logged-in user"),
		Category = "EOSKit|Presence")
	static UEOSSetPresenceAsync* SetPresence(UObject* WorldContextObject, 
		const FString& RichPresence, 
		EEOSKitPresenceStatus PresenceStatus);

	virtual void Activate() override;

private:
	static void EOS_CALL OnSetPresenceComplete(const EOS_Presence_SetPresenceCallbackInfo* Data);

	UObject* WorldContextObject;
	FString RichPresenceString;
	EEOSKitPresenceStatus PresenceStatusEnum;
};

