// Copyright (C) 2024, All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintAsyncActionBase.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "EOSKitSessionStructs.h"
#include "EOSJoinEOKSessionAsync.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FJoinEOKSession_Delegate);

/**
 * Join EOK Session - Async Blueprint node for joining EOS sessions
 */
UCLASS()
class EOSKITSESSIONS_API UEOSJoinEOKSessionAsync : public UBlueprintAsyncActionBase
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintAssignable, DisplayName="Success")
	FJoinEOKSession_Delegate OnSuccess;
	
	UPROPERTY(BlueprintAssignable, DisplayName="Failure")
	FJoinEOKSession_Delegate OnFail;

	/**
	 * Join a session using OnlineSubsystem (like EIK)
	 * @param WorldContextObject - The world context object
	 * @param SessionName - Name of the local session to create
	 * @param SessionResult - The session search result to join
	 * @param bUsePresence - Whether to use presence when joining
	 */
	UFUNCTION(BlueprintCallable, DisplayName="Join EOK Session", meta = (BlueprintInternalUseOnly = "true", WorldContext = "WorldContextObject"), Category="EOSKit|Sessions")
	static UEOSJoinEOKSessionAsync* JoinEOKSession(
		UObject* WorldContextObject,
		FName SessionName,
		FEOSKitSessionFindResult SessionResult,
		bool bUsePresence = true
	);

	virtual void Activate() override;

	void JoinSession();
	void OnJoinSessionCompleted(FName SessionName, EOnJoinSessionCompleteResult::Type Result);

private:
	UPROPERTY()
	TObjectPtr<UObject> CachedWorldContextObject;
	
	FName VSessionName;
	FEOSKitSessionFindResult SessionResult;
	bool bUsePresence;
	bool bDelegateCalled = false;
};
