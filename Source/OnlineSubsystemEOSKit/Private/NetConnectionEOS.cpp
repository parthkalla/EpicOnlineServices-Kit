// Copyright (C) 2024, All Rights Reserved.

#include "NetConnectionEOS.h"
#include "NetDriverEOS.h"
#include "InternetAddrEOS.h"
#include "SocketEOS.h"
#include "OnlineSubsystemUtils.h"
#include "OnlineSubsystemEOSKit.h"
#include "OnlineIdentityEOSKit.h"
#include "Engine/Engine.h"
#include "Net/UnrealNetwork.h"
#include "GameFramework/OnlineReplStructs.h"
#if ENGINE_MAJOR_VERSION >= 5
#include UE_INLINE_GENERATED_CPP_BY_NAME(NetConnectionEOS)
#endif

// Match the prefix used in NetDriverEOS.cpp
#define EOS_CONNECTION_URL_PREFIX TEXT("EOS")

UNetConnectionEOS::UNetConnectionEOS(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
	, bIsPassthrough(false)
	, bHasP2PSession(false)
{
}

void UNetConnectionEOS::InitLocalConnection(UNetDriver* InDriver, FSocket* InSocket, const FURL& InURL, EConnectionState InState, int32 InMaxPacket, int32 InPacketOverhead)
{
	UNetDriverEOS* EOSDriver = Cast<UNetDriverEOS>(InDriver);
	if (EOSDriver)
	{
#if ENGINE_MAJOR_VERSION >= 5 && ENGINE_MINOR_VERSION >= 6
		// UE 5.6+ removed bIsUsingP2PSockets - always use EOS sockets when URL is EOS format
		bIsPassthrough = !InURL.Host.StartsWith(EOS_CONNECTION_URL_PREFIX, ESearchCase::IgnoreCase);
#else
		// UE 5.5 and below still check bIsUsingP2PSockets
		bIsPassthrough = !EOSDriver->bIsUsingP2PSockets || !InURL.Host.StartsWith(EOS_CONNECTION_URL_PREFIX, ESearchCase::IgnoreCase);
#endif
	}
	else
	{
		bIsPassthrough = true;
	}
	
	bHasP2PSession = !bIsPassthrough;
	  
	if (bHasP2PSession)
	{
		DisableAddressResolution();
		
		// CRITICAL: Set the client's own UniqueNetId BEFORE calling Super
		// This is needed for the handshake - the client must send its UniqueNetId to the server
		// Use the default OnlineSubsystem (which should be EOSKit based on DefaultEngine.ini)
		IOnlineSubsystem* OnlineSub = IOnlineSubsystem::Get();
		if (!OnlineSub)
		{
			// Fallback to EOSKit if default is not available
			OnlineSub = IOnlineSubsystem::Get(FName(TEXT("EOSKit")));
		}
		
		if (OnlineSub)
		{
			IOnlineIdentityPtr IdentityInterface = OnlineSub->GetIdentityInterface();
			if (IdentityInterface.IsValid())
			{
				FUniqueNetIdPtr LocalUserId = IdentityInterface->GetUniquePlayerId(0);
				if (LocalUserId.IsValid())
				{
					PlayerId = LocalUserId.ToSharedRef();
					UE_LOG(LogNet, Warning, TEXT("UNetConnectionEOS::InitLocalConnection: ✅ Set client PlayerId: %s (Type: %s, Subsystem: %s)"), 
						*LocalUserId->ToString(), *LocalUserId->GetType().ToString(), *OnlineSub->GetSubsystemName().ToString());
				}
				else
				{
					UE_LOG(LogNet, Error, TEXT("UNetConnectionEOS::InitLocalConnection: ❌ Failed to get local user UniqueNetId from IdentityInterface"));
				}
			}
			else
			{
				UE_LOG(LogNet, Error, TEXT("UNetConnectionEOS::InitLocalConnection: ❌ IdentityInterface is invalid"));
			}
		}
		else
		{
			UE_LOG(LogNet, Error, TEXT("UNetConnectionEOS::InitLocalConnection: ❌ Failed to get OnlineSubsystem (tried default and EOSKit)"));
		}
	}

	Super::InitLocalConnection(InDriver, InSocket, InURL, InState, InMaxPacket, InPacketOverhead);

	// We re-set the original port, as it might have been overwritten by Super::InitLocalConnection
	if (bHasP2PSession && InSocket)
	{
		RemoteAddr->SetPort(InSocket->GetPortNo());
	}
}

void UNetConnectionEOS::InitRemoteConnection(UNetDriver* InDriver, FSocket* InSocket, const FURL& InURL, const FInternetAddr& InRemoteAddr, EConnectionState InState, int32 InMaxPacket, int32 InPacketOverhead)
{
	UNetDriverEOS* EOSDriver = Cast<UNetDriverEOS>(InDriver);
	if (EOSDriver)
	{
		bIsPassthrough = EOSDriver->bIsPassthrough;
	}
	else
	{
		bIsPassthrough = true;
	}
	
	bHasP2PSession = !bIsPassthrough;

	if (bHasP2PSession)
	{
		DisableAddressResolution();
		
		// Extract UniqueNetId from EOS address for PreLogin validation
		// The RemoteAddr contains the client's ProductUserId which we need to convert to UniqueNetId
		if (const FInternetAddrEOS* EOSRemoteAddr = static_cast<const FInternetAddrEOS*>(&InRemoteAddr))
		{
#if WITH_EOS_SDK
			EOS_ProductUserId RemoteProductUserId = EOSRemoteAddr->GetRemoteUserId();
			if (EOS_ProductUserId_IsValid(RemoteProductUserId) == EOS_TRUE)
			{
				// Convert ProductUserId to UniqueNetId string format
				char ProductUserIdStr[EOS_PRODUCTUSERID_MAX_LENGTH + 1];
				int32_t ProductUserIdStrSize = sizeof(ProductUserIdStr);
				if (EOS_ProductUserId_ToString(RemoteProductUserId, ProductUserIdStr, &ProductUserIdStrSize) == EOS_EResult::EOS_Success)
				{
					FString UserIdString = UTF8_TO_TCHAR(ProductUserIdStr);
					
					// Create UniqueNetId using the OnlineSubsystem's IdentityInterface
					// This ensures the UniqueNetId type matches what the server's OnlineSubsystem expects
					IOnlineSubsystem* OnlineSub = IOnlineSubsystem::Get();
					if (!OnlineSub)
					{
						OnlineSub = IOnlineSubsystem::Get(FName(TEXT("EOSKit")));
					}
					
					FUniqueNetIdPtr UniqueNetId = nullptr;
					if (OnlineSub)
					{
						IOnlineIdentityPtr IdentityInterface = OnlineSub->GetIdentityInterface();
						if (IdentityInterface.IsValid())
						{
							// Use the IdentityInterface to create the UniqueNetId - this ensures compatibility
							UniqueNetId = IdentityInterface->CreateUniquePlayerId(UserIdString);
						}
					}
					
					// Fallback to direct creation if IdentityInterface method fails
					if (!UniqueNetId.IsValid())
					{
						// Create UniqueNetId - use just the ProductUserId string with EOS type
						// The format should match what the client sends
						// FUniqueNetIdString::Create expects just the ID part, the type is separate
						UniqueNetId = FUniqueNetIdString::Create(UserIdString, FName(TEXT("EOS")));
					}
					
					if (UniqueNetId.IsValid())
					{
						// Set the UniqueId on the connection BEFORE calling Super
						// This ensures PreLogin can validate the client's identity
						// The PlayerId property is accessible in our derived class
						PlayerId = UniqueNetId.ToSharedRef();
						
						UE_LOG(LogNet, Warning, TEXT("UNetConnectionEOS::InitRemoteConnection: ✅ Set PlayerId from EOS address: %s (Type: %s)"), 
							*UniqueNetId->ToString(), *UniqueNetId->GetType().ToString());
					}
					else
					{
						UE_LOG(LogNet, Error, TEXT("UNetConnectionEOS::InitRemoteConnection: ❌ Failed to create UniqueNetId from ProductUserId: %s"), 
							*UserIdString);
					}
				}
			}
			else
			{
				UE_LOG(LogNet, Warning, TEXT("UNetConnectionEOS::InitRemoteConnection: Invalid RemoteProductUserId in EOS address"));
			}
#endif
		}
		else
		{
			UE_LOG(LogNet, Warning, TEXT("UNetConnectionEOS::InitRemoteConnection: Failed to cast InRemoteAddr to FInternetAddrEOS"));
		}
	}

	Super::InitRemoteConnection(InDriver, InSocket, InURL, InRemoteAddr, InState, InMaxPacket, InPacketOverhead);
}

void UNetConnectionEOS::CleanUp()
{
	Super::CleanUp();

	if (bHasP2PSession)
	{
		DestroyEOSConnection();
	}
}

bool UNetConnectionEOS::LowLevelValidateRemoteUniqueId(FUniqueNetIdRepl& UniqueId)
{
	// CRITICAL: Bypass IP-based validation for EOS P2P connections
	// The parent UIpConnection would try to validate the UniqueNetId against the IP address,
	// which fails for EOS P2P because the "address" is a ProductUserId string, not an IP address.
	// EOS P2P socket authentication already handles security, so we trust valid EOS IDs.
	
	if (bHasP2PSession && UniqueId.IsValid())
	{
		// For EOS P2P connections, if the UniqueId is valid, we trust the EOS P2P socket authentication
		// EOS P2P already validated the connection, so we can bypass IP-based validation
		UE_LOG(LogNet, Verbose, TEXT("UNetConnectionEOS::LowLevelValidateRemoteUniqueId: ✅ Bypassing IP validation for EOS P2P connection (UniqueId is valid)"));
		return true;
	}
	
	// For non-EOS connections, fall back to standard validation
	// Since the parent method doesn't exist, we just return true for valid IDs
	// This allows non-EOS connections to work normally
	return UniqueId.IsValid();
}

void UNetConnectionEOS::DestroyEOSConnection()
{
#if ENGINE_MAJOR_VERSION == 5
	FSocket* CurSocket = GetSocket();
#else
	if (!Socket)
	{
		return;
	}
	FSocket* CurSocket = Socket;
#endif
	if (CurSocket != nullptr && bHasP2PSession)
	{
		bHasP2PSession = false;

		TSharedPtr<FInternetAddrEOS> RemoteAddrEOS = StaticCastSharedPtr<FInternetAddrEOS>(RemoteAddr);

		if (RemoteAddrEOS.IsValid())
		{
			static_cast<FSocketEOS*>(CurSocket)->Close(*RemoteAddrEOS);
		}
	}
}

