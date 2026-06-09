// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintAsyncActionBase.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "OnlineSubsystemEOK/Subsystem/EOK_Subsystem.h"
#include "EOK_FindSessionByID_AsyncFunction.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FFindSessionByID_Delegate, const FSessionFindStruct&, FoundResult);

/**
 * 
 */
UCLASS()
class OnlineSubsystemEOK_API UEOK_FindSessionByID_AsyncFunction : public UBlueprintAsyncActionBase
{
	GENERATED_BODY()

public:

	UPROPERTY(BlueprintAssignable, DisplayName="Success")
	FFindSessionByID_Delegate OnSuccess;
	UPROPERTY(BlueprintAssignable, DisplayName="Failure")
	FFindSessionByID_Delegate OnFail;
	/*
	This function is used to find a session by ID and return the result. The SessionID can be obtained from the CreateEOKSession function or CreateEOKLobby function.
	@param SessionID - The ID of the session to find.
	 
	 Documentation link: https://EOK.betide.studio/multiplayer/sessions/lobbies/
	*/
	UFUNCTION(BlueprintCallable, DisplayName="Find EOK Session By ID",meta = (BlueprintInternalUseOnly = "true"), Category="Epic Online Services-Kit V2 || Sessions")
	static UEOK_FindSessionByID_AsyncFunction* FindEOKSessionByID(FString SessionID);


	virtual void Activate() override;

	void OnFindSessionCompleted(int I, bool bArg, const FOnlineSessionSearchResult& OnlineSessionSearchResult);
	void FindSession();
	
	FString Var_SessionID;

	bool bDelegateCalled = false;

};
