// Copyright (C) 2024, All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintAsyncActionBase.h"
#include "EOSKitSessionStructs.h"
#include "EOSUpdateLobbyAsync.generated.h"

/**
 * Lobby Modification Handle - Opaque handle for modifying lobby settings
 * 
 * This handle is created by EOS_Lobby_UpdateLobbyModification and must be released after use
 */
USTRUCT(BlueprintType)
struct FEOSKitLobbyModificationHandle
{
	GENERATED_BODY()

	/** Internal handle pointer - do not access directly */
	void* Handle = nullptr;

	FEOSKitLobbyModificationHandle() : Handle(nullptr) {}
	explicit FEOSKitLobbyModificationHandle(void* InHandle) : Handle(InHandle) {}

	bool IsValid() const { return Handle != nullptr; }
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FUpdateLobby_Delegate, bool, bSuccess, FString, LobbyId);

/**
 * Update Lobby - Apply modifications to an existing lobby
 * 
 * Requires a lobby modification handle created by UpdateLobbyModification
 */
UCLASS()
class EOSKITSESSIONS_API UEOSUpdateLobbyAsync : public UBlueprintAsyncActionBase
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintAssignable, DisplayName="On Complete")
	FUpdateLobby_Delegate OnComplete;

	/**
	 * Update a lobby with the changes in the modification handle
	 * 
	 * Important:
	 * - Only the lobby owner can update lobby settings
	 * - Must create modification handle first via UpdateLobbyModification
	 * - Handle is automatically released after this call
	 * - All lobby members will be notified of changes
	 * 
	 * Workflow:
	 * 1. Create modification handle: EOS_Lobby_UpdateLobbyModification
	 * 2. Set changes: EOS_LobbyModification_SetPermissionLevel, etc.
	 * 3. Apply changes: This function
	 * 
	 * @param LobbyModificationHandle - Handle containing the changes to apply
	 * 
	 * Note: This is a low-level API. Consider using helper functions for common updates.
	 */
	UFUNCTION(BlueprintCallable, DisplayName="Update Lobby", meta = (BlueprintInternalUseOnly = "true"), Category="EOSKit|Lobby")
	static UEOSUpdateLobbyAsync* UpdateLobby(FEOSKitLobbyModificationHandle LobbyModificationHandle);

	virtual void Activate() override;

private:
	void PerformUpdate();
	
	FEOSKitLobbyModificationHandle VarModificationHandle;
};
