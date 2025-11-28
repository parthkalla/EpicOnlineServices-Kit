// Copyright (C) 2024, All Rights Reserved.

#include "NetDriverEOS.h"
#include "SocketSubsystemEOS.h"
#include "SocketSubsystem.h"
#include "IPAddress.h"
#include "Sockets.h"
#include "Engine/World.h"
#include "Engine/NetConnection.h"

bool UNetDriverEOS::IsAvailable() const
{
	// Check if EOS socket subsystem is available
	ISocketSubsystem* SocketSub = ISocketSubsystem::Get(FName("EOS"));
	return SocketSub != nullptr;
}

bool UNetDriverEOS::InitializeSocketSubsystem(FString& Error)
{
	// Get or create the EOS socket subsystem
	EOSSocketSubsystem = ISocketSubsystem::Get(FName("EOS"));
	
	if (!EOSSocketSubsystem)
	{
		Error = TEXT("Failed to get EOS Socket Subsystem");
		UE_LOG(LogNet, Error, TEXT("UNetDriverEOS: %s"), *Error);
		return false;
	}

	UE_LOG(LogNet, Log, TEXT("UNetDriverEOS: Socket subsystem initialized"));
	return true;
}

bool UNetDriverEOS::InitBase(bool bInitAsClient, FNetworkNotify* InNotify, const FURL& URL, bool bReuseAddressAndPort, FString& Error)
{
	if (!Super::InitBase(bInitAsClient, InNotify, URL, bReuseAddressAndPort, Error))
	{
		return false;
	}

	// Initialize the EOS socket subsystem
	if (!InitializeSocketSubsystem(Error))
	{
		return false;
	}

	bIsInitialized = true;
	UE_LOG(LogNet, Log, TEXT("UNetDriverEOS: InitBase completed - IsClient: %d"), bInitAsClient);
	return true;
}

bool UNetDriverEOS::InitConnect(FNetworkNotify* InNotify, const FURL& ConnectURL, FString& Error)
{
	if (!InitBase(true, InNotify, ConnectURL, false, Error))
	{
		return false;
	}

	// Initialize connectionless handler
	InitConnectionlessHandler();

	// Parse the EOS ProductUserId from the URL
	FString ProductUserIdStr = ConnectURL.Host;
	UE_LOG(LogNet, Log, TEXT("UNetDriverEOS: Connecting to ProductUserId: %s"), *ProductUserIdStr);

	// The actual connection will be established by the underlying socket system
	if (!Super::InitConnect(InNotify, ConnectURL, Error))
	{
		return false;
	}

	UE_LOG(LogNet, Log, TEXT("UNetDriverEOS: InitConnect completed"));
	return true;
}

bool UNetDriverEOS::InitListen(FNetworkNotify* InNotify, FURL& LocalURL, bool bReuseAddressAndPort, FString& Error)
{
	if (!InitBase(false, InNotify, LocalURL, bReuseAddressAndPort, Error))
	{
		return false;
	}

	// Initialize connectionless handler
	InitConnectionlessHandler();

	// Set up the socket for listening
	if (!Super::InitListen(InNotify, LocalURL, bReuseAddressAndPort, Error))
	{
		return false;
	}

	UE_LOG(LogNet, Log, TEXT("UNetDriverEOS: InitListen completed on port %d"), LocalURL.Port);
	return true;
}

void UNetDriverEOS::TickDispatch(float DeltaTime)
{
	Super::TickDispatch(DeltaTime);

	// Handle any EOS-specific network ticking here
	if (bIsInitialized && EOSSocketSubsystem)
	{
		// EOS P2P events are processed automatically by the SDK
		// Additional processing can be added here if needed
	}
}

void UNetDriverEOS::ProcessRemoteFunction(AActor* Actor, UFunction* Function, void* Parameters, FOutParmRec* OutParms, FFrame* Stack, UObject* SubObject)
{
	// Process remote function calls through EOS network
	// This uses the standard Unreal replication system, which will use our EOS sockets
	Super::ProcessRemoteFunction(Actor, Function, Parameters, OutParms, Stack, SubObject);
}

void UNetDriverEOS::LowLevelSend(TSharedPtr<const FInternetAddr> Address, void* Data, int32 CountBits, FOutPacketTraits& Traits)
{
	// Send data through EOS network layer
	if (bIsInitialized && EOSSocketSubsystem)
	{
		// The actual sending is handled by FSocketEOS through the base class
		Super::LowLevelSend(Address, Data, CountBits, Traits);
	}
	else
	{
		UE_LOG(LogNet, Warning, TEXT("UNetDriverEOS::LowLevelSend: Driver not initialized"));
	}
}

void UNetDriverEOS::Shutdown()
{
	UE_LOG(LogNet, Log, TEXT("UNetDriverEOS: Shutting down"));
	
	bIsInitialized = false;
	EOSSocketSubsystem = nullptr;
	
	Super::Shutdown();
}
