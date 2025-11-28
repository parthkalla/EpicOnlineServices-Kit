// Copyright (C) 2024, All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintAsyncActionBase.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "EOSKitSessionStructs.h"
#include "EOSFindEOKSessionByIDAsync.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FFindEOKSessionByID_Delegate, const FEOSKitSessionFindResult&, FoundResult);

/**
 * Find EOK Session By ID - Async Blueprint node for finding a session by its ID
 */
UCLASS()
class EOSKITSESSIONS_API UEOSFindEOKSessionByIDAsync : public UBlueprintAsyncActionBase
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintAssignable, DisplayName="Success")
	FFindEOKSessionByID_Delegate OnSuccess;
	
	UPROPERTY(BlueprintAssignable, DisplayName="Failure")
	FFindEOKSessionByID_Delegate OnFail;

	/**
	 * This function is used to find a session by ID and return the result. The SessionID can be obtained from the CreateEOKSession function or CreateEOKLobby function.
	 * @param SessionID - The ID of the session to find.
	 */
	UFUNCTION(BlueprintCallable, DisplayName="Find EOK Session By ID", meta = (BlueprintInternalUseOnly = "true"), Category="EOSKit|Sessions")
	static UEOSFindEOKSessionByIDAsync* FindEOKSessionByID(FString SessionID);

	virtual void Activate() override;

	void OnFindSessionCompleted(int I, bool bArg, const FOnlineSessionSearchResult& OnlineSessionSearchResult);
	void FindSession();
	
	FString Var_SessionID;
	bool bDelegateCalled = false;
};
