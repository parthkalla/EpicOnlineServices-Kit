// Copyright (C) 2024, All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "EOSKitP2PTypes.generated.h"

#if WITH_EOS_SDK
#include "Windows/AllowWindowsPlatformTypes.h"
#include "eos_p2p_types.h"
#include "Windows/HideWindowsPlatformTypes.h"
#endif

/**
 * P2P Socket ID wrapper
 */
USTRUCT(BlueprintType, Category = "EOSKit|P2P")
struct EOSKITSESSIONS_API FEOSKitP2PSocketId
{
	GENERATED_BODY()

	/** A name for the connection. Must be a NULL-terminated string of between 1-32 alpha-numeric characters */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "EOSKit|P2P")
	FString SocketName = TEXT("");

#if WITH_EOS_SDK
	EOS_P2P_SocketId GetAsEosData() const
	{
		EOS_P2P_SocketId SocketId = {};
		SocketId.ApiVersion = EOS_P2P_SOCKETID_API_LATEST;
		
		if (!SocketName.IsEmpty())
		{
			FTCHARToUTF8 Converter(*SocketName);
			FCStringAnsi::Strncpy(SocketId.SocketName, Converter.Get(), EOS_P2P_SOCKETID_SOCKETNAME_SIZE);
		}
		else
		{
			SocketId.SocketName[0] = '\0';
		}
		
		return SocketId;
	}
#endif
};

/**
 * NAT Type enumeration
 */
UENUM(BlueprintType)
enum class EEOSKitNATType : uint8
{
	Unknown = 0		UMETA(DisplayName = "Unknown"),
	Open = 1		UMETA(DisplayName = "Open"),
	Moderate = 2	UMETA(DisplayName = "Moderate"),
	Strict = 3		UMETA(DisplayName = "Strict")
};

/**
 * Packet Reliability enumeration
 */
UENUM(BlueprintType)
enum class EEOSKitPacketReliability : uint8
{
	UnreliableUnordered = 0		UMETA(DisplayName = "Unreliable Unordered"),
	ReliableUnordered = 1		UMETA(DisplayName = "Reliable Unordered"),
	ReliableOrdered = 2			UMETA(DisplayName = "Reliable Ordered")
};

/**
 * Connection Established Type enumeration
 */
UENUM(BlueprintType)
enum class EEOSKitConnectionEstablishedType : uint8
{
	NewConnection = 0		UMETA(DisplayName = "New Connection"),
	Reconnection = 1		UMETA(DisplayName = "Reconnection")
};

/**
 * Network Connection Type enumeration
 */
UENUM(BlueprintType)
enum class EEOSKitNetworkConnectionType : uint8
{
	NoConnection = 0			UMETA(DisplayName = "No Connection"),
	DirectConnection = 1		UMETA(DisplayName = "Direct Connection"),
	RelayedConnection = 2		UMETA(DisplayName = "Relayed Connection")
};

/**
 * Connection Closed Reason enumeration
 */
UENUM(BlueprintType)
enum class EEOSKitConnectionClosedReason : uint8
{
	Unknown = 0					UMETA(DisplayName = "Unknown"),
	ClosedByLocalUser = 1		UMETA(DisplayName = "Closed By Local User"),
	ClosedByPeer = 2			UMETA(DisplayName = "Closed By Peer"),
	TimedOut = 3				UMETA(DisplayName = "Timed Out"),
	TooManyConnections = 4		UMETA(DisplayName = "Too Many Connections"),
	InvalidMessage = 5			UMETA(DisplayName = "Invalid Message"),
	InvalidData = 6				UMETA(DisplayName = "Invalid Data"),
	ConnectionFailed = 7		UMETA(DisplayName = "Connection Failed"),
	ConnectionClosed = 8		UMETA(DisplayName = "Connection Closed"),
	NegotiationFailed = 9		UMETA(DisplayName = "Negotiation Failed"),
	UnexpectedError = 10		UMETA(DisplayName = "Unexpected Error"),
	ConnectionIgnored = 11		UMETA(DisplayName = "Connection Ignored")
};

