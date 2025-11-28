// Copyright (C) 2024, All Rights Reserved.

#include "NetDriverEOSKitBase.h"
#include "SocketSubsystem.h"
#include "IPAddress.h"
#include "Sockets.h"
#include "Engine/World.h"
#include "Engine/NetConnection.h"

UNetDriverEOSKitBase::UNetDriverEOSKitBase(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
	, bIsInitialized(false)
	, SocketSubsystem(nullptr)
{
}

bool UNetDriverEOSKitBase::IsAvailable() const
{
	// Check if EOS socket subsystem is available
	ISocketSubsystem* SocketSub = ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM);
	return SocketSub != nullptr;
}

bool UNetDriverEOSKitBase::InitBase(bool bInitAsClient, FNetworkNotify* InNotify, const FURL& URL, bool bReuseAddressAndPort, FString& Error)
{
	if (!Super::InitBase(bInitAsClient, InNotify, URL, bReuseAddressAndPort, Error))
	{
		return false;
	}

	// Get the socket subsystem
	SocketSubsystem = ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM);
	if (!SocketSubsystem)
	{
		Error = TEXT("Failed to get socket subsystem");
		return false;
	}

	bIsInitialized = true;
	return true;
}

bool UNetDriverEOSKitBase::InitConnect(FNetworkNotify* InNotify, const FURL& ConnectURL, FString& Error)
{
	if (!InitBase(true, InNotify, ConnectURL, false, Error))
	{
		return false;
	}

	// Initialize connectionless handler
	InitConnectionlessHandler();

	// Initialize the connection to the server
	UE_LOG(LogNet, Log, TEXT("EOSKit NetDriver: Connecting to %s"), *ConnectURL.ToString());
	
	return Super::InitConnect(InNotify, ConnectURL, Error);
}

bool UNetDriverEOSKitBase::InitListen(FNetworkNotify* InNotify, FURL& LocalURL, bool bReuseAddressAndPort, FString& Error)
{
	if (!InitBase(false, InNotify, LocalURL, bReuseAddressAndPort, Error))
	{
		return false;
	}

	// Initialize connectionless handler
	InitConnectionlessHandler();

	// Initialize as server
	UE_LOG(LogNet, Log, TEXT("EOSKit NetDriver: Listening on %s"), *LocalURL.ToString());
	
	return Super::InitListen(InNotify, LocalURL, bReuseAddressAndPort, Error);
}

void UNetDriverEOSKitBase::TickDispatch(float DeltaTime)
{
	Super::TickDispatch(DeltaTime);

	// Handle any EOS-specific network ticking here
	if (bIsInitialized && SocketSubsystem)
	{
		// Process any pending EOS network events
	}
}

void UNetDriverEOSKitBase::ProcessRemoteFunction(AActor* Actor, UFunction* Function, void* Parameters, FOutParmRec* OutParms, FFrame* Stack, UObject* SubObject)
{
	// Process remote function calls through EOS network
	Super::ProcessRemoteFunction(Actor, Function, Parameters, OutParms, Stack, SubObject);
}

void UNetDriverEOSKitBase::LowLevelSend(TSharedPtr<const FInternetAddr> Address, void* Data, int32 CountBits, FOutPacketTraits& Traits)
{
	// Send data through EOS network layer
	if (bIsInitialized && SocketSubsystem)
	{
		Super::LowLevelSend(Address, Data, CountBits, Traits);
	}
}

void UNetDriverEOSKitBase::Shutdown()
{
	UE_LOG(LogNet, Log, TEXT("EOSKit NetDriver: Shutting down"));
	
	bIsInitialized = false;
	SocketSubsystem = nullptr;
	
	Super::Shutdown();
}
