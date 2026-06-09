// Copyright Epic Games, Inc. All Rights Reserved.

#include "NetConnectionEOK.h"
#include "NetDriverEOKBase.h"
#include "InternetAddrEOK.h"
#include "SocketEOK.h"
#include "SocketSubsystemEOK.h"
#if ENGINE_MAJOR_VERSION >= 5
#include UE_INLINE_GENERATED_CPP_BY_NAME(NetConnectionEOK)
#else
#include "OnlineSubsystem.h"
#include "OnlineSubsystemNames.h"
#include "OnlineSubsystemUtils.h"
#include "Misc/EngineVersionComparison.h"
#endif

UNetConnectionEOK::UNetConnectionEOK(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
	, bIsPassthrough(false)
	, bHasP2PSession(false)
{
}

void UNetConnectionEOK::InitLocalConnection(UNetDriver* InDriver, FSocket* InSocket, const FURL& InURL, EConnectionState InState, int32 InMaxPacket, int32 InPacketOverhead)
{
	// Check if this is an EOS URL by checking the protocol or if the host starts with EOS
	// When URL is parsed, "EOS:..." becomes protocol="EOS", host="PUID:SocketName:Channel"
	bool bIsEOSURL = InURL.Protocol == EOS_CONNECTION_URL_PREFIX || 
	                 InURL.Host.StartsWith(EOS_CONNECTION_URL_PREFIX, ESearchCase::IgnoreCase);
	
#if ENGINE_MAJOR_VERSION >= 5 && ENGINE_MINOR_VERSION >= 6
	// UE 5.6+ removed bIsUsingP2PSockets - always use EOS sockets when URL is EOS format
	bIsPassthrough = !bIsEOSURL;
#else
	// UE 5.5 and below still check bIsUsingP2PSockets
	bIsPassthrough = !static_cast<UNetDriverEOKBase*>(InDriver)->bIsUsingP2PSockets || !bIsEOSURL;
#endif
	bHasP2PSession = !bIsPassthrough;
	  
	if (bHasP2PSession)
	{
		DisableAddressResolution();
	}

	Super::InitLocalConnection(InDriver, InSocket, InURL, InState, InMaxPacket, InPacketOverhead);

	// We re-set the original port, as if might have been overwritten by Super::InitLocalConnection
	if (bHasP2PSession)
	{
		RemoteAddr->SetPort(InSocket->GetPortNo());
	}
}

void UNetConnectionEOK::InitRemoteConnection(UNetDriver* InDriver, FSocket* InSocket, const FURL& InURL, const FInternetAddr& InRemoteAddr, EConnectionState InState, int32 InMaxPacket, int32 InPacketOverhead)
{
	bIsPassthrough = static_cast<UNetDriverEOKBase*>(InDriver)->bIsPassthrough;
	bHasP2PSession = !bIsPassthrough;

	if (bHasP2PSession)
	{
		DisableAddressResolution();
	}

	Super::InitRemoteConnection(InDriver, InSocket, InURL, InRemoteAddr, InState, InMaxPacket, InPacketOverhead);
}

void UNetConnectionEOK::CleanUp()
{
	Super::CleanUp();

	if (bHasP2PSession)
	{
		DestroyEOSConnection();
	}
}

void UNetConnectionEOK::DestroyEOSConnection()
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
			FSocketEOS* EOSSocket = static_cast<FSocketEOS*>(CurSocket);
			
			// Get the local address before closing (needed for unbinding)
			FInternetAddrEOS LocalAddrEOS;
			EOSSocket->GetAddress(LocalAddrEOS);
			
			// Close the remote connection
			EOSSocket->Close(*RemoteAddrEOS);
			
			// For client connections, unbind the channel to allow reconnection during travel
			// Server connections keep the channel bound for other clients
			if (Driver && !Driver->IsServer() && LocalAddrEOS.IsValid())
			{
				FSocketSubsystemEOK* SocketSubsystem = static_cast<FSocketSubsystemEOK*>(ISocketSubsystem::Get(EOS_SOCKETSUBSYSTEM));
				if (SocketSubsystem)
				{
					SocketSubsystem->UnbindChannel(LocalAddrEOS);
					UE_LOG(LogTemp, Verbose, TEXT("Unbound channel for client connection during travel: %s"), *LocalAddrEOS.ToString(true));
				}
			}
		}
	}
}

