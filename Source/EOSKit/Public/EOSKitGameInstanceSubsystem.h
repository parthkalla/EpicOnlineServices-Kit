// Copyright (C) 2024, All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "EOSKitSessionStructs.h"
#include "EOSKitGameInstanceSubsystem.generated.h"

/**
 * EOSKit Game Instance Subsystem - Main subsystem for EOSKit functionality
 * 
 * This subsystem provides:
 * - Centralized access to EOSKit features
 * - Helper functions for common EOS operations
 * - Session management utilities
 * - User information access
 */
UCLASS()
class EOSKIT_API UEOSKitGameInstanceSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	/** Initialize the subsystem */
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	/** Deinitialize the subsystem */
	virtual void Deinitialize() override;

	// ========================================
	// User Information
	// ========================================

	/**
	 * Get the player's display name/nickname
	 * @param LocalUserNum - Local user index
	 * @return Player's nickname or empty string if not available
	 */
	UFUNCTION(BlueprintPure, Category="EOSKit|User")
	static FString GetPlayerNickname(int32 LocalUserNum = 0);

	/**
	 * Check if a player is logged in
	 * @param LocalUserNum - Local user index
	 * @return True if the player is logged in
	 */
	UFUNCTION(BlueprintPure, Category="EOSKit|User")
	static bool IsPlayerLoggedIn(int32 LocalUserNum = 0);

	/**
	 * Get the player's Product User ID
	 * @param LocalUserNum - Local user index
	 * @return Product User ID as a string
	 */
	UFUNCTION(BlueprintPure, Category="EOSKit|User")
	static FString GetProductUserId(int32 LocalUserNum = 0);

	/**
	 * Get the player's Epic Account ID
	 * @param LocalUserNum - Local user index
	 * @return Epic Account ID as a string
	 */
	UFUNCTION(BlueprintPure, Category="EOSKit|User")
	static FString GetEpicAccountId(int32 LocalUserNum = 0);

	// ========================================
	// Session Utilities
	// ========================================

	/**
	 * Generate a random session code
	 * Useful for creating lobby codes that players can share
	 * 
	 * @param CodeLength - Length of the code (default 6)
	 * @return Random alphanumeric code
	 */
	UFUNCTION(BlueprintPure, Category="EOSKit|Session")
	static FString GenerateSessionCode(int32 CodeLength = 6);

	/**
	 * Register the local player in a session
	 * Required for proper session functionality
	 * 
	 * @param SessionName - Name of the session
	 * @param bWasInvited - Whether the player was invited
	 */
	UFUNCTION(BlueprintCallable, Category="EOSKit|Session")
	void RegisterLocalPlayer(FName SessionName, bool bWasInvited = false);

	/**
	 * Unregister the local player from a session
	 * Should be called before leaving a session
	 * 
	 * @param SessionName - Name of the session
	 */
	UFUNCTION(BlueprintCallable, Category="EOSKit|Session")
	void UnregisterLocalPlayer(FName SessionName);

	/**
	 * Start a session
	 * Marks the session as in progress
	 * 
	 * @param SessionName - Name of the session to start
	 * @return True if the session was started successfully
	 */
	UFUNCTION(BlueprintCallable, Category="EOSKit|Session")
	bool StartSession(FName SessionName);

	/**
	 * End a session
	 * Marks the session as complete
	 * 
	 * @param SessionName - Name of the session to end
	 * @return True if the session was ended successfully
	 */
	UFUNCTION(BlueprintCallable, Category="EOSKit|Session")
	bool EndSession(FName SessionName);

	// ========================================
	// Social Features
	// ========================================

	/**
	 * Show the platform's friends interface
	 * Opens the EOS overlay friends list
	 * 
	 * @return True if the interface was shown successfully
	 */
	UFUNCTION(BlueprintCallable, Category="EOSKit|Social")
	bool ShowFriendsInterface();

	// ========================================
	// Helper Functions
	// ========================================

	/**
	 * Parse an EOS Unique Net ID to get the Product User ID
	 * Format: "EpicAccountId|ProductUserId"
	 * 
	 * @param UniqueNetIdString - The unique net ID string
	 * @return Product User ID portion
	 */
	UFUNCTION(BlueprintPure, Category="EOSKit|Utility")
	static FString ParseProductUserId(const FString& UniqueNetIdString);

	/**
	 * Parse an EOS Unique Net ID to get the Epic Account ID
	 * Format: "EpicAccountId|ProductUserId"
	 * 
	 * @param UniqueNetIdString - The unique net ID string
	 * @return Epic Account ID portion
	 */
	UFUNCTION(BlueprintPure, Category="EOSKit|Utility")
	static FString ParseEpicAccountId(const FString& UniqueNetIdString);

	/**
	 * Check if EOSKit is properly initialized
	 * @return True if EOSKit subsystem is ready
	 */
	UFUNCTION(BlueprintPure, Category="EOSKit|Status")
	static bool IsEOSKitInitialized();

private:
	/** Whether the subsystem has been initialized */
	bool bIsInitialized = false;
};
