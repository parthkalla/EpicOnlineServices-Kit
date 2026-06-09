// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Runtime/Launch/Resources/Version.h"
#include "Kismet/BlueprintAsyncActionBase.h"
#include "EOK_Logout_AsyncFunction.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FLogout_Delegate);

/**
 * 
 */
UCLASS()
class OnlineSubsystemEOK_API UEOK_Logout_AsyncFunction : public UBlueprintAsyncActionBase
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintAssignable, DisplayName="Success")
	FLogout_Delegate OnSuccess;
	UPROPERTY(BlueprintAssignable, DisplayName="Failure")
	FLogout_Delegate OnFail;
	/*
	This C++ method logs out in a user to an online subsystem using the selected method and sets up a callback function to handle the login response.
	Documentation link: https://betide-studio.gitbook.io/eos-integration-kit/authentication/
	*/
	UFUNCTION(BlueprintCallable, DisplayName="Logout EOK",meta = (BlueprintInternalUseOnly = "true"), Category="Epic Online Services-Kit V2 || Login")
	static UEOK_Logout_AsyncFunction* LogoutUsingEOK();

	void Activate() override;

	void LogoutPlayer();

	void LogoutCallback(int32 LocalUserNum, bool bWasSuccess);

};
