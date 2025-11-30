// Copyright (C) 2024, All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "EOSKitSessionStructs.h"
#include "EOSKitLobbySubsystem.generated.h"

// Forward declarations
class UEOSKitSubsystem;

/**
 * Delegate for join lobby accepted notifications
 */
DECLARE_DYNAMIC_DELEGATE_TwoParams(FEOSKitOnJoinLobbyAccepted, FString, ProductUserId, FString, UiEventId);

/**
 * Delegate for leave lobby requested notifications
 */
DECLARE_DYNAMIC_DELEGATE_TwoParams(FEOSKitOnLeaveLobbyRequested, FString, ProductUserId, FString, LobbyId);

/**
 * Delegate for lobby invite accepted notifications
 */
DECLARE_DYNAMIC_DELEGATE_FourParams(FEOSKitOnLobbyInviteAccepted, FString, LocalUserId, FString, TargetUserId, FString, LobbyId, FString, InviteId);

/**
 * Delegate for lobby invite received notifications
 */
DECLARE_DYNAMIC_DELEGATE_ThreeParams(FEOSKitOnLobbyInviteReceived, FString, LocalUserId, FString, TargetUserId, FString, InviteId);

/**
 * Delegate for lobby member status received notifications
 */
DECLARE_DYNAMIC_DELEGATE_ThreeParams(FEOSKitOnLobbyMemberStatusReceived, FString, TargetUserId, FString, LobbyId, int32, CurrentStatus);

	/**
	 * Delegate for lobby update received notifications
	 */
	DECLARE_DYNAMIC_DELEGATE_OneParam(FEOSKitOnLobbyUpdateReceived, FString, LobbyId);

	/**
	 * Delegate for lobby member update received notifications
	 */
	DECLARE_DYNAMIC_DELEGATE_TwoParams(FEOSKitOnLobbyMemberUpdateReceived, FString, LobbyId, FString, TargetUserId);

	/**
	 * Delegate for lobby invite rejected notifications
	 */
	DECLARE_DYNAMIC_DELEGATE_FourParams(FEOSKitOnLobbyInviteRejected, FString, LocalUserId, FString, TargetUserId, FString, LobbyId, FString, InviteId);

/**
 * EOSKit Lobby Subsystem
 * 
 * Manages EOS Lobby interface operations including:
 * - Lobby notifications (join, leave, invite, etc.)
 * - Lobby details access
 * - Lobby search operations
 * - Lobby modification
 */
UCLASS()
class EOSKITSESSIONS_API UEOSKitLobbySubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	// USubsystem implementation
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	// ========================================
	// Lobby Notifications
	// ========================================

	/**
	 * Register for join lobby accepted notifications
	 * Sent when user joins lobby via overlay
	 */
	UFUNCTION(BlueprintCallable, Category = "EOSKit|Lobby|Subsystem")
	bool RegisterJoinLobbyAcceptedNotification(const FEOSKitOnJoinLobbyAccepted& Callback);

	/**
	 * Unregister from join lobby accepted notifications
	 */
	UFUNCTION(BlueprintCallable, Category = "EOSKit|Lobby|Subsystem")
	void UnregisterJoinLobbyAcceptedNotification();

	/**
	 * Register for leave lobby requested notifications
	 * SDK does not automatically leave lobby - game must call LeaveLobby
	 */
	UFUNCTION(BlueprintCallable, Category = "EOSKit|Lobby|Subsystem")
	bool RegisterLeaveLobbyRequestedNotification(const FEOSKitOnLeaveLobbyRequested& Callback);

	/**
	 * Unregister from leave lobby requested notifications
	 */
	UFUNCTION(BlueprintCallable, Category = "EOSKit|Lobby|Subsystem")
	void UnregisterLeaveLobbyRequestedNotification();

	/**
	 * Register for lobby invite accepted notifications
	 */
	UFUNCTION(BlueprintCallable, Category = "EOSKit|Lobby|Subsystem")
	bool RegisterLobbyInviteAcceptedNotification(const FEOSKitOnLobbyInviteAccepted& Callback);

	/**
	 * Unregister from lobby invite accepted notifications
	 */
	UFUNCTION(BlueprintCallable, Category = "EOSKit|Lobby|Subsystem")
	void UnregisterLobbyInviteAcceptedNotification();

	/**
	 * Register for lobby invite received notifications
	 */
	UFUNCTION(BlueprintCallable, Category = "EOSKit|Lobby|Subsystem")
	bool RegisterLobbyInviteReceivedNotification(const FEOSKitOnLobbyInviteReceived& Callback);

	/**
	 * Unregister from lobby invite received notifications
	 */
	UFUNCTION(BlueprintCallable, Category = "EOSKit|Lobby|Subsystem")
	void UnregisterLobbyInviteReceivedNotification();

	/**
	 * Register for lobby member status received notifications
	 */
	UFUNCTION(BlueprintCallable, Category = "EOSKit|Lobby|Subsystem")
	bool RegisterLobbyMemberStatusReceivedNotification(const FEOSKitOnLobbyMemberStatusReceived& Callback);

	/**
	 * Unregister from lobby member status received notifications
	 */
	UFUNCTION(BlueprintCallable, Category = "EOSKit|Lobby|Subsystem")
	void UnregisterLobbyMemberStatusReceivedNotification();

	/**
	 * Register for lobby update received notifications
	 */
	UFUNCTION(BlueprintCallable, Category = "EOSKit|Lobby|Subsystem")
	bool RegisterLobbyUpdateReceivedNotification(const FEOSKitOnLobbyUpdateReceived& Callback);

	/**
	 * Unregister from lobby update received notifications
	 */
	UFUNCTION(BlueprintCallable, Category = "EOSKit|Lobby|Subsystem")
	void UnregisterLobbyUpdateReceivedNotification();

	/**
	 * Register for lobby member update received notifications
	 * Sent when a lobby member updates their attributes
	 */
	UFUNCTION(BlueprintCallable, Category = "EOSKit|Lobby|Subsystem")
	bool RegisterLobbyMemberUpdateReceivedNotification(const FEOSKitOnLobbyMemberUpdateReceived& Callback);

	/**
	 * Unregister from lobby member update received notifications
	 */
	UFUNCTION(BlueprintCallable, Category = "EOSKit|Lobby|Subsystem")
	void UnregisterLobbyMemberUpdateReceivedNotification();

	/**
	 * Register for lobby invite rejected notifications
	 * Sent when a user rejects a lobby invitation via the overlay
	 */
	UFUNCTION(BlueprintCallable, Category = "EOSKit|Lobby|Subsystem")
	bool RegisterLobbyInviteRejectedNotification(const FEOSKitOnLobbyInviteRejected& Callback);

	/**
	 * Unregister from lobby invite rejected notifications
	 */
	UFUNCTION(BlueprintCallable, Category = "EOSKit|Lobby|Subsystem")
	void UnregisterLobbyInviteRejectedNotification();

	/**
	 * Unregister from all lobby notifications
	 */
	UFUNCTION(BlueprintCallable, Category = "EOSKit|Lobby|Subsystem")
	void UnregisterAllNotifications();

private:
	// Get EOSKit Subsystem
	UEOSKitSubsystem* GetEOSKitSubsystem() const;

	// Static callback functions
	static void OnJoinLobbyAcceptedCallback(const void* Data);
	static void OnLeaveLobbyRequestedCallback(const void* Data);
	static void OnLobbyInviteAcceptedCallback(const void* Data);
	static void OnLobbyInviteReceivedCallback(const void* Data);
	static void OnLobbyMemberStatusReceivedCallback(const void* Data);
	static void OnLobbyUpdateReceivedCallback(const void* Data);
	static void OnLobbyMemberUpdateReceivedCallback(const void* Data);
	static void OnLobbyInviteRejectedCallback(const void* Data);

	// Notification delegates
	FEOSKitOnJoinLobbyAccepted OnJoinLobbyAcceptedDelegate;
	FEOSKitOnLeaveLobbyRequested OnLeaveLobbyRequestedDelegate;
	FEOSKitOnLobbyInviteAccepted OnLobbyInviteAcceptedDelegate;
	FEOSKitOnLobbyInviteReceived OnLobbyInviteReceivedDelegate;
	FEOSKitOnLobbyMemberStatusReceived OnLobbyMemberStatusReceivedDelegate;
	FEOSKitOnLobbyUpdateReceived OnLobbyUpdateReceivedDelegate;
	FEOSKitOnLobbyMemberUpdateReceived OnLobbyMemberUpdateReceivedDelegate;
	FEOSKitOnLobbyInviteRejected OnLobbyInviteRejectedDelegate;

	// Notification IDs
	uint64 JoinLobbyAcceptedNotificationId = 0;
	uint64 LeaveLobbyRequestedNotificationId = 0;
	uint64 LobbyInviteAcceptedNotificationId = 0;
	uint64 LobbyInviteReceivedNotificationId = 0;
	uint64 LobbyMemberStatusReceivedNotificationId = 0;
	uint64 LobbyUpdateReceivedNotificationId = 0;
	uint64 LobbyMemberUpdateReceivedNotificationId = 0;
	uint64 LobbyInviteRejectedNotificationId = 0;
};
