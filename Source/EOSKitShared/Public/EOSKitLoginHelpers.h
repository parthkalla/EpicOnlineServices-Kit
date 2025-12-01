// Copyright (C) 2024, All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "EOSKitLoginHelpers.generated.h"

/**
 * Helper library for checking EOS login status and providing user-friendly feedback
 */
UCLASS()
class EOSKITSHARED_API UEOSKitLoginHelpers : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	/**
	 * Check if a user is logged in to EOS (has valid ProductUserId)
	 * @param WorldContextObject World context
	 * @return True if user is logged in and can use EOS services
	 */
	UFUNCTION(BlueprintCallable, Category = "EOSKit|Login", meta = (WorldContext = "WorldContextObject"))
	static bool IsLoggedInToEOS(UObject* WorldContextObject);

	/**
	 * Get the current ProductUserId as a string (for debugging/display)
	 * @param WorldContextObject World context
	 * @return ProductUserId string, or empty if not logged in
	 */
	UFUNCTION(BlueprintCallable, Category = "EOSKit|Login", meta = (WorldContext = "WorldContextObject"))
	static FString GetProductUserIdString(UObject* WorldContextObject);

	/**
	 * Check if user is logged in, and log a warning if not
	 * Use this before session operations to provide better feedback
	 * @param WorldContextObject World context
	 * @param OperationName Name of the operation being attempted (e.g., "Create Session")
	 * @return True if logged in, False if not
	 */
	UFUNCTION(BlueprintCallable, Category = "EOSKit|Login", meta = (WorldContext = "WorldContextObject"))
	static bool CheckLoginStatusWithWarning(UObject* WorldContextObject, const FString& OperationName);

	/**
	 * Get a user-friendly error message about login requirements
	 * @return Error message string explaining how to login
	 */
	UFUNCTION(BlueprintPure, Category = "EOSKit|Login")
	static FString GetLoginRequiredMessage();

	/**
	 * Check if auto-login is enabled in project settings
	 * @return True if auto-login is configured
	 */
	UFUNCTION(BlueprintPure, Category = "EOSKit|Login")
	static bool IsAutoLoginEnabled();
};
