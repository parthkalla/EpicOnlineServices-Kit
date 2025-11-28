// Copyright (C) 2024, All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "EOSKitSessionStructs.h"
#include "eos_sessions.h"
#include "eos_sessions_types.h"
#include "EOSKitSessionsSubsystem.generated.h"

// Forward declarations
class UEOSKitSubsystem;

/**
 * Delegate for session invite accepted notifications
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FEOSKitOnJoinSessionAccepted, FString, LocalUserId, FString, UiEventId);

/**
 * Delegate for leave session requested notifications
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FEOSKitOnLeaveSessionRequested, FString, LocalUserId, FString, SessionName);

/**
 * Delegate for session invite received notifications
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FEOSKitOnSessionInviteReceived, FString, LocalUserId, FString, InviteId, FString, TargetUserId);

/**
 * Delegate for session invite accepted notifications
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_FourParams(FEOSKitOnSessionInviteAccepted, FString, LocalUserId, FString, SessionId, FString, TargetUserId, FString, InviteId);

/**
 * Delegate for session invite rejected notifications
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_FourParams(FEOSKitOnSessionInviteRejected, FString, LocalUserId, FString, SessionId, FString, TargetUserId, FString, InviteId);

/**
 * EOSKit Sessions Subsystem
 * 
 * Manages EOS session handles, notifications, and provides helper functions
 * for session operations. Complements the async Blueprint nodes with
 * persistent handle storage and event management.
 */
UCLASS()
class EOSKITSESSIONS_API UEOSKitSessionsSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	// USubsystem implementation
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	// ========================================
	// Session Handle Management
	// ========================================

	/**
	 * Store a session details handle for later use
	 * @param SessionId - Identifier for this session
	 * @param Handle - EOS session details handle
	 */
	UFUNCTION(BlueprintCallable, Category = "EOSKit|Sessions|Subsystem")
	void StoreSessionDetailsHandle(const FString& SessionId, int64 Handle);

	/**
	 * Retrieve a stored session details handle
	 * @param SessionId - Identifier for the session
	 * @param OutHandle - Retrieved handle (0 if not found)
	 * @return True if handle was found
	 */
	UFUNCTION(BlueprintCallable, Category = "EOSKit|Sessions|Subsystem")
	bool GetSessionDetailsHandle(const FString& SessionId, int64& OutHandle);

	/**
	 * Remove and release a session details handle
	 * @param SessionId - Identifier for the session
	 */
	UFUNCTION(BlueprintCallable, Category = "EOSKit|Sessions|Subsystem")
	void ReleaseSessionDetailsHandle(const FString& SessionId);

	/**
	 * Clear all stored session handles
	 */
	UFUNCTION(BlueprintCallable, Category = "EOSKit|Sessions|Subsystem")
	void ClearAllSessionHandles();

	// ========================================
	// Active Session Management
	// ========================================

	/**
	 * Get active session handle by name
	 * @param SessionName - Name of the session
	 * @param OutHandle - Retrieved handle
	 * @return True if successful
	 */
	UFUNCTION(BlueprintCallable, Category = "EOSKit|Sessions|Subsystem")
	bool GetActiveSessionHandle(const FString& SessionName, int64& OutHandle);

	/**
	 * Get the current active session name (if any)
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "EOSKit|Sessions|Subsystem")
	FString GetCurrentSessionName() const { return CurrentSessionName; }

	/**
	 * Set the current active session name
	 */
	UFUNCTION(BlueprintCallable, Category = "EOSKit|Sessions|Subsystem")
	void SetCurrentSessionName(const FString& SessionName) { CurrentSessionName = SessionName; }

	// ========================================
	// Session Notifications
	// ========================================

	/**
	 * Register for join session accepted notifications
	 */
	UFUNCTION(BlueprintCallable, Category = "EOSKit|Sessions|Subsystem")
	bool RegisterJoinSessionAcceptedNotification();

	/**
	 * Register for leave session requested notifications
	 */
	UFUNCTION(BlueprintCallable, Category = "EOSKit|Sessions|Subsystem")
	bool RegisterLeaveSessionRequestedNotification();

	/**
	 * Register for session invite received notifications
	 */
	UFUNCTION(BlueprintCallable, Category = "EOSKit|Sessions|Subsystem")
	bool RegisterSessionInviteReceivedNotification();

	/**
	 * Register for session invite accepted notifications
	 */
	UFUNCTION(BlueprintCallable, Category = "EOSKit|Sessions|Subsystem")
	bool RegisterSessionInviteAcceptedNotification();

	/**
	 * Register for session invite rejected notifications
	 */
	UFUNCTION(BlueprintCallable, Category = "EOSKit|Sessions|Subsystem")
	bool RegisterSessionInviteRejectedNotification();

	/**
	 * Unregister from all session notifications
	 */
	UFUNCTION(BlueprintCallable, Category = "EOSKit|Sessions|Subsystem")
	void UnregisterAllNotifications();

	// ========================================
	// Session Utility Functions
	// ========================================

	/**
	 * Check if a user is in a specific session
	 */
	UFUNCTION(BlueprintCallable, Category = "EOSKit|Sessions|Subsystem")
	bool IsUserInSession(const FString& TargetUserId, const FString& SessionName);

	/**
	 * Get the number of pending invites for the local user
	 */
	UFUNCTION(BlueprintCallable, Category = "EOSKit|Sessions|Subsystem")
	int32 GetInviteCount();

	/**
	 * Get an invite ID by index
	 */
	UFUNCTION(BlueprintCallable, Category = "EOSKit|Sessions|Subsystem")
	FString GetInviteIdByIndex(int32 Index);

	/**
	 * Dump session state to logs (debug)
	 */
	UFUNCTION(BlueprintCallable, Category = "EOSKit|Sessions|Subsystem")
	void DumpSessionState(const FString& SessionName);

	// ========================================
	// Notification Delegates
	// ========================================

	UPROPERTY(BlueprintAssignable, Category = "EOSKit|Sessions|Notifications")
	FEOSKitOnJoinSessionAccepted OnJoinSessionAccepted;

	UPROPERTY(BlueprintAssignable, Category = "EOSKit|Sessions|Notifications")
	FEOSKitOnLeaveSessionRequested OnLeaveSessionRequested;

	UPROPERTY(BlueprintAssignable, Category = "EOSKit|Sessions|Notifications")
	FEOSKitOnSessionInviteReceived OnSessionInviteReceived;

	UPROPERTY(BlueprintAssignable, Category = "EOSKit|Sessions|Notifications")
	FEOSKitOnSessionInviteAccepted OnSessionInviteAccepted;

	UPROPERTY(BlueprintAssignable, Category = "EOSKit|Sessions|Notifications")
	FEOSKitOnSessionInviteRejected OnSessionInviteRejected;

private:
	// Get EOS Sessions Handle
	EOS_HSessions GetSessionsHandle() const;

	// Get EOSKit Subsystem
	UEOSKitSubsystem* GetEOSKitSubsystem() const;

	// Static callback functions for EOS notifications
	static void EOS_CALL OnJoinSessionAcceptedCallback(const EOS_Sessions_JoinSessionAcceptedCallbackInfo* Data);
	static void EOS_CALL OnLeaveSessionRequestedCallback(const EOS_Sessions_LeaveSessionRequestedCallbackInfo* Data);
	static void EOS_CALL OnSessionInviteReceivedCallback(const EOS_Sessions_SessionInviteReceivedCallbackInfo* Data);
	static void EOS_CALL OnSessionInviteAcceptedCallback(const EOS_Sessions_SessionInviteAcceptedCallbackInfo* Data);
	static void EOS_CALL OnSessionInviteRejectedCallback(const EOS_Sessions_SessionInviteRejectedCallbackInfo* Data);

	// Session handle storage
	TMap<FString, EOS_HSessionDetails> SessionDetailsHandles;
	
	// Current active session
	FString CurrentSessionName;

	// Notification IDs for cleanup
	EOS_NotificationId JoinSessionAcceptedNotificationId = EOS_INVALID_NOTIFICATIONID;
	EOS_NotificationId LeaveSessionRequestedNotificationId = EOS_INVALID_NOTIFICATIONID;
	EOS_NotificationId SessionInviteReceivedNotificationId = EOS_INVALID_NOTIFICATIONID;
	EOS_NotificationId SessionInviteAcceptedNotificationId = EOS_INVALID_NOTIFICATIONID;
	EOS_NotificationId SessionInviteRejectedNotificationId = EOS_INVALID_NOTIFICATIONID;
};
