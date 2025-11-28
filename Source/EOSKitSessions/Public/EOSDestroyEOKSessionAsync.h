// Copyright (C) 2024, All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintAsyncActionBase.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "EOSDestroyEOKSessionAsync.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FDestroyEOKSession_Delegate);

/**
 * Destroy EOK Session - Async Blueprint node for destroying EOS sessions
 */
UCLASS()
class EOSKITSESSIONS_API UEOSDestroyEOKSessionAsync : public UBlueprintAsyncActionBase
{
	GENERATED_BODY()

public:
	bool bDelegateCalled = false;
	FName Var_SessionName;
	
	UPROPERTY(BlueprintAssignable, DisplayName="Success")
	FDestroyEOKSession_Delegate OnSuccess;
	
	UPROPERTY(BlueprintAssignable, DisplayName="Failure")
	FDestroyEOKSession_Delegate OnFail;

	/**
	 * This C++ method destroys a session using the EOS SDK and sets up a callback function to handle the destroy session response.
	 * @param SessionName - The name of the session to destroy.
	 */
	UFUNCTION(BlueprintCallable, DisplayName="Destroy EOK Session", meta = (BlueprintInternalUseOnly = "true"), Category="EOSKit|Sessions")
	static UEOSDestroyEOKSessionAsync* DestroyEOKSession(FName SessionName);

	virtual void Activate() override;

	void DestroySession();

	void OnDestroySessionCompleted(FName SessionName, bool bWasSuccess);
};
