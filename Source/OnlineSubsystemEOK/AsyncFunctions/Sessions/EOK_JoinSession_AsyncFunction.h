// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintAsyncActionBase.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "OnlineSubsystemEOK/Subsystem/EOK_Subsystem.h"
#include "EOK_JoinSession_AsyncFunction.generated.h"


UENUM(BlueprintType)
enum class EEOKJoinResult : uint8 {
	Success       UMETA(DisplayName="Success"),
	SessionIsFull               UMETA(DisplayName="Session Is Full"),
	SessionDoesNotExist        UMETA(DisplayName="Session Does Not Exist"),
	CouldNotRetrieveAddress        UMETA(DisplayName="Could Not Retrieve Address"),
	AlreadyInSession              UMETA(DisplayName="Already In Session"),
	UnknownError              UMETA(DisplayName="UnknownError"),
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FJoinSession_Delegate, EEOKJoinResult, Result, FString, SessionJoinAddress);
/**
 * 
 */
UCLASS()
class OnlineSubsystemEOK_API UEOK_JoinSession_AsyncFunction : public UBlueprintAsyncActionBase
{
	GENERATED_BODY()

public:

	bool bDelegateCalled = false;
	FName Var_SessionName;
	
	UPROPERTY(BlueprintAssignable, DisplayName="Success")
	FJoinSession_Delegate OnSuccess;
	
	UPROPERTY(BlueprintAssignable, DisplayName="Failure")
	FJoinSession_Delegate OnFail;
	
	/*
	This C++ method joins a session using the EOS SDK and sets up a callback function to handle the join session response.
	Documentation link: https://betide-studio.gitbook.io/eos-integration-kit/sessions/
	For Input Parameters, please refer to the documentation link above.
	*/
	UFUNCTION(BlueprintCallable, DisplayName="Join EOK Session",meta = (BlueprintInternalUseOnly = "true",  WorldContext = "WorldContextObject"), Category="Epic Online Services-Kit V2 || Sessions")
	static UEOK_JoinSession_AsyncFunction* JoinEOKSessions(UObject* WorldContextObject,
		FName SessionName,
		FSessionFindStruct SessionToJoin,
		bool bLanSession = false);

	FSessionFindStruct Var_SessionToJoin;
	
	UPROPERTY()
	UObject* Var_WorldContextObject;


	void Activate() override;
	void JoinSession();
	void OnJoinSessionCompleted(FName SessionName, EOnJoinSessionCompleteResult::Type Result);
};
