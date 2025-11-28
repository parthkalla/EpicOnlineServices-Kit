// Copyright (C) 2024, All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintAsyncActionBase.h"
#include "EOSKitSessionStructs.h"
#include "EOSLobbySearchFindAsync.generated.h"

/**
 * Lobby Search Handle - Opaque handle for lobby search operations
 * 
 * This handle is created by EOS_Lobby_CreateLobbySearch and must be released after use
 */
USTRUCT(BlueprintType)
struct FEOSKitLobbySearchHandle
{
	GENERATED_BODY()

	/** Internal handle pointer - do not access directly */
	void* Handle = nullptr;

	FEOSKitLobbySearchHandle() : Handle(nullptr) {}
	explicit FEOSKitLobbySearchHandle(void* InHandle) : Handle(InHandle) {}

	bool IsValid() const { return Handle != nullptr; }
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FLobbySearchFind_Delegate, bool, bSuccess);

/**
 * Lobby Search Find - Execute a lobby search and retrieve results
 * 
 * Searches for lobbies matching the configured search criteria
 */
UCLASS()
class EOSKITSESSIONS_API UEOSLobbySearchFindAsync : public UBlueprintAsyncActionBase
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintAssignable, DisplayName="On Complete")
	FLobbySearchFind_Delegate OnComplete;

	/**
	 * Find lobbies matching the search criteria
	 * 
	 * Workflow:
	 * 1. Create search handle: EOS_Lobby_CreateLobbySearch
	 * 2. Set search parameters:
	 *    - EOS_LobbySearch_SetMaxResults
	 *    - EOS_LobbySearch_SetParameter (filters)
	 *    - EOS_LobbySearch_SetLobbyId (optional)
	 * 3. Execute search: This function
	 * 4. Get results:
	 *    - EOS_LobbySearch_GetSearchResultCount
	 *    - EOS_LobbySearch_CopySearchResultByIndex
	 * 5. Release handle: EOS_LobbySearch_Release
	 * 
	 * Important:
	 * - Handle must be configured before calling this
	 * - Results are stored in the handle after completion
	 * - Handle is automatically released after this call
	 * 
	 * @param SearchHandle - Configured lobby search handle
	 * @param LocalUserId - Product User ID of the local user
	 */
	UFUNCTION(BlueprintCallable, DisplayName="Lobby Search Find", meta = (BlueprintInternalUseOnly = "true"), Category="EOSKit|Lobby")
	static UEOSLobbySearchFindAsync* LobbySearchFind(FEOSKitLobbySearchHandle SearchHandle, FString LocalUserId);

	virtual void Activate() override;

private:
	void PerformSearch();
	
	FEOSKitLobbySearchHandle VarSearchHandle;
	FString VarLocalUserId;
};
