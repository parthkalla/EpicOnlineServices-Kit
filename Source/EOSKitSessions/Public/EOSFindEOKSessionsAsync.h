// Copyright (C) 2024, All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintAsyncActionBase.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "EOSKitSessionStructs.h"
#include "EOSFindEOKSessionsAsync.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FFindEOKSession_Delegate, const TArray<FEOSKitSessionFindResult>&, SessionResults);

/**
 * Find EOK Sessions - Async Blueprint node for searching EOS sessions
 */
UCLASS()
class EOSKITSESSIONS_API UEOSFindEOKSessionsAsync : public UBlueprintAsyncActionBase
{
	GENERATED_BODY()

public:
	// Store the world context object
	UPROPERTY()
	TObjectPtr<UObject> CachedWorldContextObject;

	UPROPERTY(BlueprintAssignable, DisplayName="Success")
	FFindEOKSession_Delegate OnSuccess;
	
	UPROPERTY(BlueprintAssignable, DisplayName="Failure")
	FFindEOKSession_Delegate OnFail;

	/**
	 * This C++ method searches for sessions in an online subsystem using the selected method and sets up a callback function to handle the search response.
	 * @param WorldContextObject - The world context object (usually 'self' in Blueprint).
	 * @param SessionSettings - A map of session settings to filter the search.
	 * @param MatchType - Type of match to search for (Matchmaking, Custom, or Lobby).
	 * @param MaxResults - Maximum number of search results to return.
	 * @param RegionToSearch - Region to search in.
	 * @param bLanSearch - Whether to search for LAN sessions.
	 * @param bIncludePartySessions - Whether to include party sessions in the search.
	 */
	UFUNCTION(BlueprintCallable, DisplayName="Find EOK Sessions", meta = (BlueprintInternalUseOnly = "true", WorldContext = "WorldContextObject", AutoCreateRefTerm="SessionSettings"), Category="EOSKit|Sessions")
	static UEOSFindEOKSessionsAsync* FindEOKSessions(
		UObject* WorldContextObject,
		TMap<FString, FEOSKitAttribute> SessionSettings,
		EEOSKitMatchType MatchType = EEOSKitMatchType::LobbySession,
		int32 MaxResults = 15,
		EEOSKitRegion RegionToSearch = EEOSKitRegion::NoSelection,
		bool bLanSearch = false,
		bool bIncludePartySessions = false
	);

	virtual void Activate() override;

	void FindSession();

	void OnFindSessionCompleted(bool bWasSuccess);

	// This is a C++ variable for storing a reference to an online session search.
	TSharedPtr<FOnlineSessionSearch> SessionSearch;

	TMap<FString, FEOSKitAttribute> SessionSettings;
	EEOSKitRegion E_RegionToSearch;
	EEOSKitMatchType E_MatchType;
	int32 I_MaxResults;
	bool B_bLanSearch;
	bool bIncludePartySessions = false;
	bool bDelegateCalled = false;
};
