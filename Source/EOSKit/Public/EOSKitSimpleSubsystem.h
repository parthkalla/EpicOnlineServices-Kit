// Copyright (C) 2024, All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "EOSKitSessionStructs.h"
#include "EOSKitSimpleSubsystem.generated.h"

// ========================================
// Delegates for Blueprint Callbacks
// ========================================

DECLARE_DYNAMIC_DELEGATE_TwoParams(FEOSKit_Login_Callback, bool, bWasSuccess, const FString&, Error);
DECLARE_DYNAMIC_DELEGATE_OneParam(FEOSKit_Logout_Callback, bool, bWasSuccess);
DECLARE_DYNAMIC_DELEGATE_TwoParams(FEOSKit_CreateSession_Callback, bool, bWasSuccess, const FName&, SessionName);
DECLARE_DYNAMIC_DELEGATE_TwoParams(FEOSKit_CreateLobby_Callback, bool, bWasSuccess, const FName&, SessionName);
DECLARE_DYNAMIC_DELEGATE_OneParam(FEOSKit_DestroySession_Callback, bool, bWasSuccess);
DECLARE_DYNAMIC_DELEGATE_OneParam(FEOSKit_JoinSession_Callback, bool, bWasSuccess);
DECLARE_DYNAMIC_DELEGATE_TwoParams(FEOSKit_FindSession_Callback, bool, bWasSuccess, const TArray<FString>&, SessionIds);

// ========================================
// EOSKit Simple Subsystem
// ========================================

/**
 * EOSKit Simple Subsystem - Simplified interface for EOS operations
 * 
 * This subsystem provides easy-to-use functions similar to EIK_Subsystem,
 * but uses the EOS SDK directly. All functions use simple callback delegates
 * for Blueprint-friendly usage.
 */
UCLASS()
class EOSKIT_API UEOSKitSimpleSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	/** Initialize the subsystem */
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	/** Deinitialize the subsystem */
	virtual void Deinitialize() override;

	// ========================================
	// Authentication
	// ========================================

	/**
	 * Login with Device ID
	 * @param DisplayName - Display name for the device account
	 * @param DeviceName - Device identifier
	 * @param Result - Callback
	 */
	UFUNCTION(BlueprintCallable, Category = "EOSKit|Login")
	void LoginWithDeviceID(const FString& DisplayName, const FString& DeviceName, const FEOSKit_Login_Callback& Result);

	/**
	 * Login with Account Portal (Epic Account)
	 * @param Result - Callback
	 */
	UFUNCTION(BlueprintCallable, Category = "EOSKit|Login")
	void LoginWithAccountPortal(const FEOSKit_Login_Callback& Result);

	/**
	 * Login with Persistent Auth
	 * @param Result - Callback
	 */
	UFUNCTION(BlueprintCallable, Category = "EOSKit|Login")
	void LoginWithPersistentAuth(const FEOSKit_Login_Callback& Result);

	/**
	 * Logout the current user
	 * @param Result - Callback
	 */
	UFUNCTION(BlueprintCallable, Category = "EOSKit|Login")
	void Logout(const FEOSKit_Logout_Callback& Result);

	// ========================================
	// User Information
	// ========================================

	/**
	 * Get the player's display name/nickname
	 * @param LocalUserNum - Local user index (default 0)
	 * @return Player's nickname or empty string
	 */
	UFUNCTION(BlueprintPure, Category = "EOSKit|User")
	FString GetPlayerNickname(int32 LocalUserNum = 0) const;

	/**
	 * Check if a player is logged in
	 * @param LocalUserNum - Local user index (default 0)
	 * @return True if logged in
	 */
	UFUNCTION(BlueprintPure, Category = "EOSKit|User")
	bool GetLoginStatus(int32 LocalUserNum = 0) const;

	/**
	 * Get the player's Product User ID
	 * @param LocalUserNum - Local user index (default 0)
	 * @return Product User ID as string
	 */
	UFUNCTION(BlueprintPure, Category = "EOSKit|User")
	FString GetProductUserID(int32 LocalUserNum = 0) const;

	/**
	 * Get the player's Epic Account ID
	 * @param LocalUserNum - Local user index (default 0)
	 * @return Epic Account ID as string
	 */
	UFUNCTION(BlueprintPure, Category = "EOSKit|User")
	FString GetEpicID(int32 LocalUserNum = 0) const;

	// ========================================
	// Sessions
	// ========================================

	/**
	 * Create an EOS Session (P2P)
	 * @param CustomSettings - Custom session attributes
	 * @param SessionName - Name for the session
	 * @param bIsDedicatedServer - Whether this is a dedicated server
	 * @param bIsLan - Whether this is a LAN match (must be false for EOS)
	 * @param NumberOfPublicConnections - Max players
	 * @param Region - Region for the session
	 * @param Result - Callback
	 */
	UFUNCTION(BlueprintCallable, DisplayName = "Create EOS Session", Category = "EOSKit|Sessions", meta = (AutoCreateRefTerm = "CustomSettings"))
	void CreateEOSSession(
		const TMap<FString, FString>& CustomSettings,
		const FString& SessionName = TEXT("Modified_EOS_Session"),
		bool bIsDedicatedServer = false,
		bool bIsLan = false,
		int32 NumberOfPublicConnections = 4,
		EEOSKitRegion Region = EEOSKitRegion::NoSelection,
		const FEOSKit_CreateSession_Callback& Result
	);

	/**
	 * Create an EOS Lobby
	 * @param CustomSettings - Custom lobby attributes
	 * @param SessionName - Name for the lobby
	 * @param bUseVoiceChat - Enable voice chat
	 * @param bUsePresence - Enable presence (required for lobbies)
	 * @param bAllowInvites - Allow invites
	 * @param bAdvertise - Make lobby searchable
	 * @param bAllowJoinInProgress - Allow joining mid-game
	 * @param bIsLan - Whether this is LAN (must be false for EOS)
	 * @param NumberOfPublicConnections - Max public players
	 * @param NumberOfPrivateConnections - Max private players
	 * @param Result - Callback
	 */
	UFUNCTION(BlueprintCallable, DisplayName = "Create EOS Lobby", Category = "EOSKit|Lobbies", meta = (AutoCreateRefTerm = "CustomSettings"))
	void CreateEOSLobby(
		const TMap<FString, FString>& CustomSettings,
		const FString& SessionName = TEXT("Modified_EOS_Lobby"),
		bool bUseVoiceChat = false,
		bool bUsePresence = true,
		bool bAllowInvites = true,
		bool bAdvertise = true,
		bool bAllowJoinInProgress = true,
		bool bIsLan = false,
		int32 NumberOfPublicConnections = 4,
		int32 NumberOfPrivateConnections = 0,
		const FEOSKit_CreateLobby_Callback& Result
	);

	/**
	 * Find EOS Sessions or Lobbies
	 * @param SearchSettings - Search criteria
	 * @param MatchType - Type of match to search for
	 * @param RegionToSearch - Region to search in
	 * @param Result - Callback
	 */
	UFUNCTION(BlueprintCallable, DisplayName = "Find EOS Session", Category = "EOSKit|Sessions", meta = (AutoCreateRefTerm = "SearchSettings"))
	void FindEOSSession(
		const TMap<FString, FString>& SearchSettings,
		EEOSKitMatchType MatchType = EEOSKitMatchType::LobbySession,
		EEOSKitRegion RegionToSearch = EEOSKitRegion::NoSelection,
		const FEOSKit_FindSession_Callback& Result
	);

	/**
	 * Destroy an EOS Session
	 * @param SessionName - Name of the session to destroy
	 * @param Result - Callback
	 */
	UFUNCTION(BlueprintCallable, DisplayName = "Destroy EOS Session", Category = "EOSKit|Sessions")
	void DestroyEOSSession(const FName& SessionName, const FEOSKit_DestroySession_Callback& Result);

	/**
	 * Join an EOS Session
	 * @param SessionName - Name for the session
	 * @param SessionId - ID of the session to join
	 * @param bIsDedicatedServerSession - Whether joining a dedicated server
	 * @param Result - Callback
	 */
	UFUNCTION(BlueprintCallable, DisplayName = "Join EOS Session", Category = "EOSKit|Sessions")
	void JoinEOSSession(
		const FName& SessionName,
		const FString& SessionId,
		bool bIsDedicatedServerSession = false,
		const FEOSKit_JoinSession_Callback& Result
	);

	// ========================================
	// Session Management
	// ========================================

	/**
	 * Register the local player in a session
	 * @param SessionName - Name of the session
	 * @param bWasInvited - Whether the player was invited
	 */
	UFUNCTION(BlueprintCallable, Category = "EOSKit|Sessions")
	void RegisterPlayer(const FName& SessionName, bool bWasInvited = false);

	/**
	 * Unregister the local player from a session
	 * @param SessionName - Name of the session
	 */
	UFUNCTION(BlueprintCallable, Category = "EOSKit|Sessions")
	void UnregisterPlayer(const FName& SessionName);

	/**
	 * Start a session
	 * @param SessionName - Name of the session
	 * @return True if successful
	 */
	UFUNCTION(BlueprintCallable, Category = "EOSKit|Sessions")
	bool StartSession(const FName& SessionName);

	/**
	 * End a session
	 * @param SessionName - Name of the session
	 * @return True if successful
	 */
	UFUNCTION(BlueprintCallable, Category = "EOSKit|Sessions")
	bool EndSession(const FName& SessionName);

	// ========================================
	// Social Features
	// ========================================

	/**
	 * Show the friends interface
	 * @return True if successful
	 */
	UFUNCTION(BlueprintCallable, Category = "EOSKit|Social")
	bool ShowFriendUserInterface();

	// ========================================
	// Utility Functions
	// ========================================

	/**
	 * Generate a random session code
	 * @param CodeLength - Length of the code (default 9)
	 * @return Random alphanumeric code
	 */
	UFUNCTION(BlueprintPure, Category = "EOSKit|Utility")
	FString GenerateSessionCode(int32 CodeLength = 9) const;

	/**
	 * Check if EOSKit is initialized
	 * @return True if initialized
	 */
	UFUNCTION(BlueprintPure, Category = "EOSKit|Status")
	bool IsEOSKitInitialized() const;

private:
	/** Whether the subsystem has been initialized */
	bool bIsInitialized = false;

	/** Store active async nodes to prevent garbage collection */
	UPROPERTY()
	TArray<TObjectPtr<UObject>> ActiveAsyncNodes;
};

