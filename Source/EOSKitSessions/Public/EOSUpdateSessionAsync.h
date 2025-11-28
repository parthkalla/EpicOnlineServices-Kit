// Copyright (C) 2024, All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintAsyncActionBase.h"
#include "EOSKitSessionStructs.h"
#include "EOSUpdateSessionAsync.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FUpdateSession_Delegate, const FString&, SessionId);

/**
 * Update Session - Async Blueprint node for updating session settings
 * 
 * IMPORTANT: This is a simplified implementation that recreates the session
 * with new settings. For production use, consider implementing a session
 * modification handle manager.
 */
UCLASS()
class EOSKITSESSIONS_API UEOSUpdateSessionAsync : public UBlueprintAsyncActionBase
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintAssignable, DisplayName="Success")
	FUpdateSession_Delegate OnSuccess;
	
	UPROPERTY(BlueprintAssignable, DisplayName="Failure")
	FUpdateSession_Delegate OnFail;

	/**
	 * Update a session's settings
	 * 
	 * NOTE: This implementation uses a simplified approach that modifies and updates
	 * the session in one operation. For more complex scenarios, you may need to
	 * implement a SessionModification handle management system.
	 * 
	 * @param SessionName - Name of the session to update
	 * @param NewSessionSettings - New session attributes
	 * @param NewMaxPlayers - New maximum number of players (0 = keep current)
	 * @param bNewAllowJoinInProgress - Allow new players to join after session starts
	 */
	UFUNCTION(BlueprintCallable, DisplayName="Update Session", meta = (BlueprintInternalUseOnly = "true", AutoCreateRefTerm="NewSessionSettings"), Category="EOSKit|Sessions")
	static UEOSUpdateSessionAsync* UpdateSession(
		FName SessionName,
		const TMap<FString, FEOSKitAttribute>& NewSessionSettings,
		int32 NewMaxPlayers = 0,
		bool bNewAllowJoinInProgress = true
	);

	virtual void Activate() override;

private:
	void UpdateGameSession();
	
	FName VSessionName;
	TMap<FString, FEOSKitAttribute> SessionSettings;
	int32 MaxPlayers;
	bool bAllowJoinInProgress;
	bool bDelegateCalled = false;
};
