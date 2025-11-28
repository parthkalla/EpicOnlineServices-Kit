// Copyright (C) 2024, All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintAsyncActionBase.h"
#include "EOSEndSessionAsync.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FEndSession_Delegate);

/**
 * End Session - Async Blueprint node for marking a session as ended
 * Marks the session as completed and no longer active
 */
UCLASS()
class EOSKITSESSIONS_API UEOSEndSessionAsync : public UBlueprintAsyncActionBase
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintAssignable, DisplayName="Success")
	FEndSession_Delegate OnSuccess;
	
	UPROPERTY(BlueprintAssignable, DisplayName="Failure")
	FEndSession_Delegate OnFail;

	/**
	 * Mark a session as ended
	 * The session will no longer be active but still exists until destroyed
	 * 
	 * @param SessionName - Name of the session to end
	 */
	UFUNCTION(BlueprintCallable, DisplayName="End Session", meta = (BlueprintInternalUseOnly = "true"), Category="EOSKit|Sessions")
	static UEOSEndSessionAsync* EndSession(FName SessionName);

	virtual void Activate() override;

private:
	void EndGameSession();
	
	FName VSessionName;
	bool bDelegateCalled = false;
};
