// Copyright (C) 2024, All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "EOSKitP2PTypes.h"
#include "EOSKitSharedTypes.h"
#include "EOSKitSubsystem.h"

#if WITH_EOS_SDK
#include "Windows/AllowWindowsPlatformTypes.h"
#include "eos_p2p.h"
#include "eos_p2p_types.h"
#include "Windows/HideWindowsPlatformTypes.h"
#endif

// .generated.h must always be the last include
#include "EOSKitP2PSubsystem.generated.h"

/**
 * Delegate for peer connection request (multicast for Blueprint assignment)
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnEOSPeerConnectionRequest,
	const FEOSKitProductUserId&, RemoteUserId,
	const FEOSKitP2PSocketId&, SocketId,
	const FEOSKitProductUserId&, LocalUserId);

/**
 * Delegate for peer connection request (regular for function parameters)
 */
DECLARE_DYNAMIC_DELEGATE_ThreeParams(FOnEOSPeerConnectionRequestDelegate,
	const FEOSKitProductUserId&, RemoteUserId,
	const FEOSKitP2PSocketId&, SocketId,
	const FEOSKitProductUserId&, LocalUserId);

/**
 * P2P subsystem for managing EOS P2P interface
 */
UCLASS()
class EOSKITP2P_API UEOSKitP2PSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	// USubsystem interface
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	// ========================================
	// SDK Function Wrappers
	// ========================================

	/**
	 * Query NAT type
	 * @return Result code (Note: This is async, returns immediately)
	 */
	UFUNCTION(BlueprintCallable, Category = "EOSKit|P2P")
	EEOSResult QueryNATType();

	/**
	 * Get NAT type (from cached result)
	 * @param OutNATType - Output NAT type
	 * @return True if successful
	 */
	UFUNCTION(BlueprintCallable, Category = "EOSKit|P2P")
	bool GetNATType(EEOSKitNATType& OutNATType);

	/**
	 * Add notification for peer connection requests
	 * @param LocalUserId - Product User ID of the local user
	 * @param SocketId - Optional socket ID filter (empty to listen for all)
	 * @param Callback - Callback delegate
	 * @return Notification ID (use with RemoveNotifyPeerConnectionRequest)
	 */
	UFUNCTION(BlueprintCallable, Category = "EOSKit|P2P")
	int32 AddNotifyPeerConnectionRequest(const FEOSKitProductUserId& LocalUserId, const FEOSKitP2PSocketId& SocketId, const FOnEOSPeerConnectionRequestDelegate& Callback);

	/**
	 * Remove notification for peer connection requests
	 * @param NotificationId - Notification ID returned from AddNotifyPeerConnectionRequest
	 */
	UFUNCTION(BlueprintCallable, Category = "EOSKit|P2P")
	void RemoveNotifyPeerConnectionRequest(int32 NotificationId);

	/**
	 * Accept connection
	 * @param LocalUserId - Product User ID of the local user
	 * @param RemoteUserId - Product User ID of the remote user
	 * @param SocketId - Socket ID of the connection
	 * @return Result code
	 */
	UFUNCTION(BlueprintCallable, Category = "EOSKit|P2P")
	EEOSResult AcceptConnection(const FEOSKitProductUserId& LocalUserId, const FEOSKitProductUserId& RemoteUserId, const FEOSKitP2PSocketId& SocketId);

	/**
	 * Close connection
	 * @param LocalUserId - Product User ID of the local user
	 * @param RemoteUserId - Product User ID of the remote user
	 * @param SocketId - Socket ID of the connection (empty to close all)
	 * @return Result code
	 */
	UFUNCTION(BlueprintCallable, Category = "EOSKit|P2P")
	EEOSResult CloseConnection(const FEOSKitProductUserId& LocalUserId, const FEOSKitProductUserId& RemoteUserId, const FEOSKitP2PSocketId& SocketId);

	/**
	 * Send packet
	 * @param LocalUserId - Product User ID of the local user
	 * @param RemoteUserId - Product User ID of the remote user
	 * @param SocketId - Socket ID
	 * @param Channel - Channel number (0-255)
	 * @param Data - Data to send
	 * @param bAllowDelayedDelivery - Allow delayed delivery if connection not established
	 * @param bDisableAutoAcceptConnection - Disable auto-accept connection
	 * @param Reliability - Packet reliability mode
	 * @return Result code
	 */
	UFUNCTION(BlueprintCallable, Category = "EOSKit|P2P")
	EEOSResult SendPacket(const FEOSKitProductUserId& LocalUserId, 
		const FEOSKitProductUserId& RemoteUserId, 
		const FEOSKitP2PSocketId& SocketId, 
		int32 Channel, 
		const TArray<uint8>& Data, 
		bool bAllowDelayedDelivery = true, 
		bool bDisableAutoAcceptConnection = false, 
		EEOSKitPacketReliability Reliability = EEOSKitPacketReliability::ReliableUnordered);

	/**
	 * Get next received packet size
	 * @param LocalUserId - Product User ID of the local user
	 * @param RequestedChannel - Optional channel filter (nullptr for any channel)
	 * @param OutPacketSizeBytes - Output packet size in bytes
	 * @return True if successful
	 */
	UFUNCTION(BlueprintCallable, Category = "EOSKit|P2P")
	bool GetNextReceivedPacketSize(const FEOSKitProductUserId& LocalUserId, int32 RequestedChannel, int32& OutPacketSizeBytes);

	/**
	 * Receive packet
	 * @param LocalUserId - Product User ID of the local user
	 * @param MaxDataSizeBytes - Maximum data size to receive
	 * @param RequestedChannel - Optional channel filter (nullptr for any channel)
	 * @param OutPeerId - Output peer ID who sent the packet
	 * @param OutSocketId - Output socket ID
	 * @param OutChannel - Output channel number
	 * @param OutData - Output data
	 * @param OutBytesWritten - Output bytes written
	 * @return True if successful
	 */
	UFUNCTION(BlueprintCallable, Category = "EOSKit|P2P")
	bool ReceivePacket(const FEOSKitProductUserId& LocalUserId, 
		int32 MaxDataSizeBytes, 
		int32 RequestedChannel, 
		FEOSKitProductUserId& OutPeerId, 
		FEOSKitP2PSocketId& OutSocketId, 
		int32& OutChannel, 
		TArray<uint8>& OutData, 
		int32& OutBytesWritten);

	/**
	 * Get relay control count
	 * @return Number of relay control settings
	 */
	UFUNCTION(BlueprintCallable, Category = "EOSKit|P2P")
	int32 GetRelayControlCount();

	/**
	 * Set relay control
	 * @param RelayControl - Relay control setting (0=NoRelays, 1=AllowRelays, 2=ForceRelays)
	 * @return Result code
	 */
	UFUNCTION(BlueprintCallable, Category = "EOSKit|P2P")
	EEOSResult SetRelayControl(int32 RelayControl);

	/**
	 * Get relay control
	 * @param OutRelayControl - Output relay control setting
	 * @return True if successful
	 */
	UFUNCTION(BlueprintCallable, Category = "EOSKit|P2P")
	bool GetRelayControl(int32& OutRelayControl);

	UPROPERTY(BlueprintAssignable, Category = "EOSKit|P2P")
	FOnEOSPeerConnectionRequest OnPeerConnectionRequest;

private:
	UEOSKitSubsystem* GetEOSKitSubsystem() const;
	EOS_HP2P GetP2PHandle() const;

	static void EOS_CALL OnPeerConnectionRequestCallback(const EOS_P2P_OnIncomingConnectionRequestInfo* Data);

	TMap<int32, EOS_NotificationId> NotificationIdMap;
	TMap<EOS_NotificationId, FOnEOSPeerConnectionRequestDelegate> PeerConnectionRequestCallbacks;
	int32 NextNotificationId;
};

