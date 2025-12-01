// Copyright (C) 2024, All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintAsyncActionBase.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "FindSessionsCallbackProxy.h"
#include "EOSJoinLobbyAsync.generated.h"

UENUM(BlueprintType)
enum class EEOSKitJoinResult : uint8
{
	Success UMETA(DisplayName = "Success"),
	SessionIsFull UMETA(DisplayName = "Session Is Full"),
	SessionDoesNotExist UMETA(DisplayName = "Session Does Not Exist"),
	CouldNotRetrieveAddress UMETA(DisplayName = "Could Not Retrieve Address"),
	AlreadyInSession UMETA(DisplayName = "Already In Session"),
	UnknownError UMETA(DisplayName = "Unknown Error")
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnJoinEOSKitSessionComplete, EEOSKitJoinResult, Result, FString, SessionJoinAddress);

/**
 * Join EOK Session - Async Blueprint node for joining EOS sessions
 */
UCLASS()
class EOSKITSESSIONS_API UEOSJoinLobbyAsync : public UBlueprintAsyncActionBase
{
	GENERATED_BODY()

public:
	bool bDelegateCalled = false;
	FName Var_SessionName;

	UPROPERTY(BlueprintAssignable)
	FOnJoinEOSKitSessionComplete Success;

	UPROPERTY(BlueprintAssignable)
	FOnJoinEOSKitSessionComplete Failure;

	UPROPERTY(BlueprintAssignable)
	FOnJoinEOSKitSessionComplete Result;

	/**
	 * Join an EOS Kit Session
	 * This method joins a session using the EOS SDK
	 * @param WorldContextObject - World context
	 * @param SessionName - Name to give the session locally
	 * @param SessionToJoin - The session result from Find Sessions
	 * @param bLanSession - Is this a LAN session?
	 */
	UFUNCTION(BlueprintCallable, DisplayName = "Join EOK Session", meta = (BlueprintInternalUseOnly = "true", WorldContext = "WorldContextObject"), Category = "EOSKit|Sessions")
	static UEOSJoinLobbyAsync* JoinEOSKitSession(
		UObject* WorldContextObject,
		FName SessionName,
		FBlueprintSessionResult SessionToJoin,
		bool bLanSession = false
	);

	FBlueprintSessionResult Var_SessionToJoin;

	UPROPERTY()
	UObject* Var_WorldContextObject;

	virtual void Activate() override;

private:
	void JoinSession();
	void OnJoinSessionCompleted(FName SessionName, EOnJoinSessionCompleteResult::Type Result);
};