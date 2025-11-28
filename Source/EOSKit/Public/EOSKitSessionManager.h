#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "EOSKitSessionStructs.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "EOSKitSessionManager.generated.h"

// Delegates for session operations
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnEOSKitCreateSessionComplete, bool, bWasSuccessful);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnEOSKitCreateLobbyComplete, FString, LobbyId);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnEOSKitCreateLobbyFailed);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnEOSKitFindSessionsComplete, bool, bWasSuccessful, const TArray<FEOSKitSessionResult>&, SessionResults);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnEOSKitJoinSessionComplete, bool, bWasSuccessful);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnEOSKitDestroySessionComplete, bool, bWasSuccessful);

/**
 * Session Manager for EOSKit
 * Handles all session and lobby operations
 */
UCLASS()
class EOSKIT_API UEOSKitSessionManager : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	UEOSKitSessionManager();

	// Subsystem interface
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	// ========== SESSION CREATION ==========

	/**
	 * Create an EOS Session (Matchmaking)
	 * @param SessionSettings - Settings for the session to create
	 * @param DedicatedServerSettings - Optional dedicated server settings
	 * @param ExtraSettings - Additional key-value pairs for session attributes
	 */
	UFUNCTION(BlueprintCallable, Category = "EOSKit|Session", meta = (AutoCreateRefTerm = "ExtraSettings"))
	void CreateEOSKitSession(
		const FEOSKitCreateSessionSettings& SessionSettings,
		const FEOSKitDedicatedServerSettings& DedicatedServerSettings,
		const TMap<FString, FEOSKitAttribute>& ExtraSettings
	);

	/**
	 * Create an EOS Lobby
	 * @param SessionSettings - Basic session settings
	 * @param MemberSettings - Lobby member settings
	 * @param ExtraSettings - Additional key-value pairs for lobby attributes
	 */
	UFUNCTION(BlueprintCallable, Category = "EOSKit|Lobby", meta = (AutoCreateRefTerm = "ExtraSettings"))
	void CreateEOSKitLobby(
		const FEOSKitCreateLobbySettings& SessionSettings,
		const FEOSKitLobbyMemberSettings& MemberSettings,
		const TMap<FString, FEOSKitAttribute>& ExtraSettings
	);

	// ========== SESSION FINDING ==========

	/**
	 * Find EOS Sessions
	 * @param SessionSettings - Search criteria for finding sessions
	 */
	UFUNCTION(BlueprintCallable, Category = "EOSKit|Session")
	void FindEOSKitSessions(const FEOSKitSessionSearchSettings& SessionSettings);

	/**
	 * Find EOS Lobbies
	 * @param SessionSettings - Search criteria for finding lobbies
	 */
	UFUNCTION(BlueprintCallable, Category = "EOSKit|Lobby")
	void FindEOSKitLobbies(const FEOSKitSessionSearchSettings& SessionSettings);

	// ========== SESSION JOINING ==========

	/**
	 * Join an EOS Session
	 * @param SessionResult - The session to join (from search results)
	 */
	UFUNCTION(BlueprintCallable, Category = "EOSKit|Session")
	void JoinEOSKitSession(const FEOSKitSessionResult& SessionResult);

	/**
	 * Join an EOS Lobby
	 * @param SessionResult - The lobby to join (from search results)
	 */
	UFUNCTION(BlueprintCallable, Category = "EOSKit|Lobby")
	void JoinEOSKitLobby(const FEOSKitSessionResult& SessionResult);

	/**
	 * Join an EOS Lobby by ID
	 * @param LobbyId - The lobby ID to join directly
	 */
	UFUNCTION(BlueprintCallable, Category = "EOSKit|Lobby")
	void JoinEOSKitLobbyById(const FString& LobbyId);

	// ========== SESSION MANAGEMENT ==========

	/**
	 * Destroy the current session
	 */
	UFUNCTION(BlueprintCallable, Category = "EOSKit|Session")
	void DestroyEOSKitSession();

	/**
	 * Leave the current lobby
	 */
	UFUNCTION(BlueprintCallable, Category = "EOSKit|Lobby")
	void LeaveEOSKitLobby();

	/**
	 * Update session settings
	 * @param NewSettings - Updated session settings
	 */
	UFUNCTION(BlueprintCallable, Category = "EOSKit|Session")
	void UpdateEOSKitSession(const TMap<FString, FEOSKitAttribute>& NewSettings);

	/**
	 * Start the session (mark as in progress)
	 */
	UFUNCTION(BlueprintCallable, Category = "EOSKit|Session")
	void StartEOSKitSession();

	/**
	 * End the session (mark as complete)
	 */
	UFUNCTION(BlueprintCallable, Category = "EOSKit|Session")
	void EndEOSKitSession();

	// ========== UTILITY FUNCTIONS ==========

	/**
	 * Get the current session ID
	 */
	UFUNCTION(BlueprintPure, Category = "EOSKit|Session")
	FString GetCurrentSessionId() const;

	/**
	 * Check if currently in a session
	 */
	UFUNCTION(BlueprintPure, Category = "EOSKit|Session")
	bool IsInSession() const;

	/**
	 * Get the current lobby ID
	 */
	UFUNCTION(BlueprintPure, Category = "EOSKit|Lobby")
	FString GetCurrentLobbyId() const;

	/**
	 * Check if currently in a lobby
	 */
	UFUNCTION(BlueprintPure, Category = "EOSKit|Lobby")
	bool IsInLobby() const;

	/**
	 * Make an EOSKit Attribute (helper for creating attributes in Blueprint)
	 */
	UFUNCTION(BlueprintPure, Category = "EOSKit|Session", meta = (DisplayName = "Make EOSKit Attribute"))
	static FEOSKitAttribute MakeEOSKitAttribute_String(const FString& Key, const FString& StringValue);

	UFUNCTION(BlueprintPure, Category = "EOSKit|Session", meta = (DisplayName = "Make EOSKit Attribute (Bool)"))
	static FEOSKitAttribute MakeEOSKitAttribute_Bool(const FString& Key, bool BoolValue);

	UFUNCTION(BlueprintPure, Category = "EOSKit|Session", meta = (DisplayName = "Make EOSKit Attribute (Int)"))
	static FEOSKitAttribute MakeEOSKitAttribute_Int(const FString& Key, int32 IntValue);

	/**
	 * Break Session Find Struct - Get individual components from session result
	 */
	UFUNCTION(BlueprintPure, Category = "EOSKit|Session", meta = (DisplayName = "Break Session Find Struct"))
	static void BreakSessionFindStruct(
		const FEOSKitSessionResult& SessionResult,
		FString& SessionName,
		int32& CurrentNumberOfPlayers,
		int32& MaxNumberOfPlayers,
		bool& bIsDedicatedServer,
		TMap<FString, FEOSKitAttribute>& SessionSettings
	);

	/**
	 * Make Create Session Extra Settings - Helper for creating extra settings
	 */
	UFUNCTION(BlueprintPure, Category = "EOSKit|Session", meta = (DisplayName = "Make Create Session Extra Settings"))
	static FEOSKitCreateSessionSettings MakeCreateSessionExtraSettings(
		bool bIsLanMatch,
		int32 NumberOfPrivateConnections,
		bool bShouldAdvertise,
		bool bAllowJoinInProgress,
		EEOSKitRegion Region,
		bool bUsePresence,
		bool bAllowJoinViaPresence,
		bool bAllowJoinViaPresenceFriendsOnly,
		bool bEnforceSanctions
	);

	/**
	 * Make Create Lobby Settings - Helper for creating lobby settings
	 */
	UFUNCTION(BlueprintPure, Category = "EOSKit|Lobby", meta = (DisplayName = "Make Create Lobby Settings"))
	static FEOSKitCreateLobbySettings MakeCreateLobbySettings(
		bool bIsLanMatch,
		bool bAllowInvites,
		int32 NumberOfPrivateConnections,
		bool bShouldAdvertise,
		bool bAllowJoinInProgress,
		EEOSKitRegion Region,
		bool bUseVoiceChat,
		bool bUsePresence,
		const FString& BucketId,
		bool bSupportHostMigration,
		bool bEnableJoinViaId,
		const FString& LobbyIdOverride
	);

	// ========== DELEGATES ==========

	UPROPERTY(BlueprintAssignable, Category = "EOSKit|Session")
	FOnEOSKitCreateSessionComplete OnCreateSessionComplete;

	UPROPERTY(BlueprintAssignable, Category = "EOSKit|Lobby")
	FOnEOSKitCreateLobbyComplete OnCreateLobbySuccess;

	UPROPERTY(BlueprintAssignable, Category = "EOSKit|Lobby")
	FOnEOSKitCreateLobbyFailed OnCreateLobbyFailure;

	UPROPERTY(BlueprintAssignable, Category = "EOSKit|Session")
	FOnEOSKitFindSessionsComplete OnFindSessionsComplete;

	UPROPERTY(BlueprintAssignable, Category = "EOSKit|Session")
	FOnEOSKitJoinSessionComplete OnJoinSessionComplete;

	UPROPERTY(BlueprintAssignable, Category = "EOSKit|Session")
	FOnEOSKitDestroySessionComplete OnDestroySessionComplete;

private:
	// Session interface
	IOnlineSessionPtr SessionInterface;

	// Current session name
	FName CurrentSessionName;

	// Current lobby ID
	FString CurrentLobbyId;

	// Search results
	TSharedPtr<class FOnlineSessionSearch> SessionSearch;

	// Callback handlers
	void OnCreateSessionCompleteInternal(FName SessionName, bool bWasSuccessful);
	void OnFindSessionsCompleteInternal(bool bWasSuccessful);
	void OnJoinSessionCompleteInternal(FName SessionName, EOnJoinSessionCompleteResult::Type Result);
	void OnDestroySessionCompleteInternal(FName SessionName, bool bWasSuccessful);
	void OnStartSessionCompleteInternal(FName SessionName, bool bWasSuccessful);
	void OnEndSessionCompleteInternal(FName SessionName, bool bWasSuccessful);

	// Delegate handles
	FDelegateHandle OnCreateSessionCompleteDelegateHandle;
	FDelegateHandle OnFindSessionsCompleteDelegateHandle;
	FDelegateHandle OnJoinSessionCompleteDelegateHandle;
	FDelegateHandle OnDestroySessionCompleteDelegateHandle;
	FDelegateHandle OnStartSessionCompleteDelegateHandle;
	FDelegateHandle OnEndSessionCompleteDelegateHandle;

	// Helper functions
	void SetupSessionSettings(FOnlineSessionSettings& OutSettings, const FEOSKitCreateSessionSettings& InSettings);
	void SetupLobbySettings(FOnlineSessionSettings& OutSettings, const FEOSKitCreateLobbySettings& InSettings);
	FString RegionToString(EEOSKitRegion Region) const;
	FEOSKitSessionResult ConvertToSessionResult(const FOnlineSessionSearchResult& SearchResult);
};
