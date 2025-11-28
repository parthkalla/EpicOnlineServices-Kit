// Copyright (C) 2024, All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintAsyncActionBase.h"
#include "EOSStartSessionAsync.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FStartSession_Delegate);

/**
 * Start Session - Async Blueprint node for starting a session
 * Marks the session as started, preventing new joins if "join in progress" is disabled
 */
UCLASS()
class EOSKITSESSIONS_API UEOSStartSessionAsync : public UBlueprintAsyncActionBase
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintAssignable, DisplayName="Success")
	FStartSession_Delegate OnSuccess;
	
	UPROPERTY(BlueprintAssignable, DisplayName="Failure")
	FStartSession_Delegate OnFail;

	/**
	 * Mark a session as started
	 * Makes it unable to be found if session properties indicate "join in progress" is not available
	 * 
	 * @param SessionName - Name of the session to start
	 */
	UFUNCTION(BlueprintCallable, DisplayName="Start Session", meta = (BlueprintInternalUseOnly = "true"), Category="EOSKit|Sessions")
	static UEOSStartSessionAsync* StartSession(FName SessionName);

	virtual void Activate() override;

private:
	void StartGameSession();
	
	FName VSessionName;
	bool bDelegateCalled = false;
};
