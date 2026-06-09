// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintAsyncActionBase.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "OnlineSubsystemEOK/Subsystem/EOK_Subsystem.h"
#include "EOK_DestroySession_AsyncFunction.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FDestroySession_Delegate);

UCLASS()
class OnlineSubsystemEOK_API UEOK_DestroySession_AsyncFunction : public UBlueprintAsyncActionBase
{
	GENERATED_BODY()
public:

	bool bDelegateCalled = false;
	FName Var_SessionName;
	
	UPROPERTY(BlueprintAssignable, DisplayName="Success")
	FDestroySession_Delegate OnSuccess;
	
	UPROPERTY(BlueprintAssignable, DisplayName="Failure")
	FDestroySession_Delegate OnFail;

	/*
	 *This C++ method destroys a session using the EOS SDK and sets up a callback function to handle the destroy session response.
	 *Documentation link: https://EOK.betide.studio/multiplayer/sessions/matchmaking-sessions/
	 *For Input Parameters, please refer to the documentation link above.
	 */
	UFUNCTION(BlueprintCallable, DisplayName="Destroy EOK Session",meta = (BlueprintInternalUseOnly = "true"), Category="Epic Online Services-Kit V2 || Sessions")
	static UEOK_DestroySession_AsyncFunction* DestroyEOKSessions(FName SessionName);


	virtual void Activate() override;

	void DestroySession();

	void OnDestroySessionCompleted(FName SessionName, bool bWasSuccess);
};
