// Copyright (C) 2024, All Rights Reserved.

#include "NetDriverEOS.h"
#include "SocketSubsystemEOS.h"
#include "SocketSubsystem.h"
#include "IPAddress.h"
#include "Sockets.h"
#include "Engine/World.h"
#include "Engine/NetConnection.h"
#include "Interfaces/IPv4/IPv4Address.h"
#include "IpConnection.h"

UNetDriverEOS::UNetDriverEOS(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
	, EOSSocketSubsystem(nullptr)
	, bIsInitialized(false)
{
	UE_LOG(LogNet, Warning, TEXT("UNetDriverEOS: Constructor called - NetDriver class is being loaded!"));
}

bool UNetDriverEOS::IsAvailable() const
{
	// Lazily ensure the EOS socket subsystem is initialized
	const FName EOSSubsystemName(TEXT("EOS"));
	ISocketSubsystem* SocketSub = ISocketSubsystem::Get(EOSSubsystemName);

	if (!SocketSub)
	{
		UE_LOG(LogNet, Warning, TEXT("UNetDriverEOS::IsAvailable: EOS socket subsystem not found, attempting lazy initialization"));

		FString Error;
		FSocketSubsystemEOS* EOSSub = FSocketSubsystemEOS::Create();
		if (EOSSub && EOSSub->Init(Error))
		{
			SocketSub = ISocketSubsystem::Get(EOSSubsystemName);
			UE_LOG(LogNet, Warning, TEXT("UNetDriverEOS::IsAvailable: Lazy initialization succeeded, SocketSub=%p"), SocketSub);
		}
		else
		{
			UE_LOG(LogNet, Error, TEXT("UNetDriverEOS::IsAvailable: Failed to initialize EOS socket subsystem: %s"), *Error);
		}
	}

	const bool bAvailable = (SocketSub != nullptr);
	UE_LOG(LogNet, Warning, TEXT("UNetDriverEOS::IsAvailable: SocketSub=%p, bAvailable=%d"), SocketSub, bAvailable);
	return bAvailable;
}

ISocketSubsystem* UNetDriverEOS::GetSocketSubsystem()
{
	// Always return the EOS socket subsystem for EOS P2P connections
	if (!EOSSocketSubsystem)
	{
		EOSSocketSubsystem = ISocketSubsystem::Get(FName("EOS"));
	}
	return EOSSocketSubsystem ? EOSSocketSubsystem : Super::GetSocketSubsystem();
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
	// Initialize the EOS socket subsystem first
	if (!InitializeSocketSubsystem(Error))
	{
		return false;
	}

	// Call UNetDriver::InitBase directly (not UIpNetDriver::InitBase) to avoid creating IP sockets
	if (!UNetDriver::InitBase(bInitAsClient, InNotify, URL, bReuseAddressAndPort, Error))
	{
		UE_LOG(LogNet, Error, TEXT("UNetDriverEOS: Failed to init driver base"));
		return false;
	}

	// Get the EOS socket subsystem
	FSocketSubsystemEOS* SocketSubsystem = static_cast<FSocketSubsystemEOS*>(GetSocketSubsystem());
	if (!SocketSubsystem)
	{
		Error = TEXT("Could not get EOS socket subsystem");
		UE_LOG(LogNet, Error, TEXT("UNetDriverEOS: %s"), *Error);
		return false;
	}

	// Get the world context
	UWorld* MyWorld = GetWorld();
	if (!MyWorld)
	{
		// Try to find world from engine
		if (GEngine)
		{
			for (const FWorldContext& Context : GEngine->GetWorldContexts())
			{
				if (Context.World())
				{
					MyWorld = Context.World();
					break;
				}
			}
		}
	}

	// Get our local address (proves we're logged in)
	TSharedRef<FInternetAddr> LocalAddress = SocketSubsystem->GetLocalBindAddr(*GLog);
	if (!LocalAddress->IsValid())
	{
		Error = TEXT("Could not bind local address - not logged in?");
		UE_LOG(LogNet, Error, TEXT("UNetDriverEOS: %s"), *Error);
		return false;
	}

	// Create EOS socket
	FSocket* NewSocket = SocketSubsystem->CreateSocket(NAME_DGram, TEXT("Unreal"), NAME_None);
	if (!NewSocket)
	{
		Error = TEXT("Could not create EOS socket");
		UE_LOG(LogNet, Error, TEXT("UNetDriverEOS: %s"), *Error);
		return false;
	}

	// Set the socket and local address (UNetDriver method)
	TSharedPtr<FSocket> SharedSocket(NewSocket);
	SetSocketAndLocalAddress(SharedSocket);

	if (GetSocket() == nullptr)
	{
		Error = TEXT("Could not set socket");
		UE_LOG(LogNet, Error, TEXT("UNetDriverEOS: %s"), *Error);
		return false;
	}

	// Store our local address and set socket name/channel
	TSharedRef<FInternetAddrEOS> EOSLocalAddress = StaticCastSharedRef<FInternetAddrEOS>(LocalAddress);
	
	// Set socket name and channel based on NetDriverName (UE 5.5) or NetDriverDefinition (UE 5.6+)
	FString NetDriverNameStr;
#if ENGINE_MAJOR_VERSION >= 5 && ENGINE_MINOR_VERSION >= 6
	NetDriverNameStr = GetNetDriverDefinition().ToString();
#else
	NetDriverNameStr = NetDriverName.ToString();
#endif
	EOSLocalAddress->SetSocketName(NetDriverNameStr);
	// Use hash of NetDriverName as channel (clamp to uint8 range)
	uint32 NameHash = GetTypeHash(NetDriverNameStr);
	EOSLocalAddress->SetChannel(static_cast<uint8>(NameHash % 256));

	// Set local address
	LocalAddr = LocalAddress;

	bIsInitialized = true;
	UE_LOG(LogNet, Log, TEXT("UNetDriverEOS: InitBase completed - IsClient: %d, SocketName=%s, Channel=%d"), 
		bInitAsClient, *EOSLocalAddress->GetSocketName(), EOSLocalAddress->GetChannel());
	return true;
}

bool UNetDriverEOS::InitConnect(FNetworkNotify* InNotify, const FURL& ConnectURL, FString& Error)
{
	// Check if this is an EOS URL
	// EIK checks ConnectURL.Host.StartsWith("EOS") - the Host should contain the full EOS address
	// Format: EOS:ProductUserId:GameNetDriver:26
	// Also check Protocol in case Unreal parses it differently
	bool bIsEOSURL = ConnectURL.Host.StartsWith(TEXT("EOS"), ESearchCase::IgnoreCase) || 
	                 ConnectURL.Protocol.Equals(TEXT("EOS"), ESearchCase::IgnoreCase);
	bool bIsAvailableResult = IsAvailable();
	
	UE_LOG(LogNet, Warning, TEXT("UNetDriverEOS::InitConnect: FullURL=%s, Host=%s, Protocol=%s, Port=%d, bIsEOSURL=%d, bIsAvailable=%d"), 
		*ConnectURL.ToString(), *ConnectURL.Host, *ConnectURL.Protocol, ConnectURL.Port, bIsEOSURL, bIsAvailableResult);
	
	// If not an EOS URL or EOS not available, fall back to IpNetDriver (passthrough)
	if (!bIsAvailableResult || !bIsEOSURL)
	{
		UE_LOG(LogNet, Log, TEXT("UNetDriverEOS: Not an EOS URL or EOS not available, falling back to IpNetDriver"));
		return Super::InitConnect(InNotify, ConnectURL, Error);
	}

	// This is an EOS P2P connection - initialize base
	if (!InitBase(true, InNotify, ConnectURL, false, Error))
	{
		return false;
	}

	// Parse the remote EOS address from the URL
	// URL format: EOS:ProductUserId:GameNetDriver:26
	// Extract just the ProductUserId from the full EOS address
	FString ProductUserIdStr = ConnectURL.Host;
	
	// Remove "EOS:" prefix if present
	if (ProductUserIdStr.StartsWith(TEXT("EOS:"), ESearchCase::IgnoreCase))
	{
		ProductUserIdStr = ProductUserIdStr.RightChop(4);
	}
	
	// Parse ProductUserId from format: ProductUserId:GameNetDriver:26
	TArray<FString> Parts;
	ProductUserIdStr.ParseIntoArray(Parts, TEXT(":"), true);
	if (Parts.Num() >= 1)
	{
		ProductUserIdStr = Parts[0]; // Extract just the ProductUserId
	}
	
	UE_LOG(LogNet, Warning, TEXT("UNetDriverEOS::InitConnect: Extracted ProductUserId: %s (length: %d) from URL: %s"), 
		*ProductUserIdStr, ProductUserIdStr.Len(), *ConnectURL.Host);
	
	bool bIsValid = false;
	TSharedRef<FInternetAddrEOS> RemoteHost = MakeShared<FInternetAddrEOS>();
	RemoteHost->SetProductUserId(ProductUserIdStr);
	bIsValid = RemoteHost->IsValid();
	
	UE_LOG(LogNet, Warning, TEXT("UNetDriverEOS::InitConnect: SetProductUserId result - IsValid: %d, Address: %s"), 
		bIsValid, *RemoteHost->ToString(false));
	
	if (!bIsValid || ConnectURL.Port < 0)
	{
		Error = FString::Printf(TEXT("Invalid remote address. ProductUserId: %s, IsValid: %d"), *ProductUserIdStr, bIsValid);
		UE_LOG(LogNet, Error, TEXT("UNetDriverEOS: %s. ConnectUrl = (%s)"), *Error, *ConnectURL.ToString());
		return false;
	}

	// Get the socket created in InitBase
	FSocket* CurSocket = GetSocket();
	if (!CurSocket)
	{
		Error = TEXT("Socket is null");
		UE_LOG(LogNet, Error, TEXT("UNetDriverEOS: %s"), *Error);
		return false;
	}

	// Get local bind address and set socket name/channel on RemoteHost
	FSocketSubsystemEOS* SocketSubsystem = static_cast<FSocketSubsystemEOS*>(GetSocketSubsystem());
	if (!SocketSubsystem)
	{
		Error = TEXT("Could not get EOS socket subsystem");
		UE_LOG(LogNet, Error, TEXT("UNetDriverEOS: %s"), *Error);
		return false;
	}

	// Get local bind address
	TSharedRef<FInternetAddr> LocalAddress = SocketSubsystem->GetLocalBindAddr(*GLog);
	TSharedRef<FInternetAddrEOS> EOSLocalAddress = StaticCastSharedRef<FInternetAddrEOS>(LocalAddress);
	
	// Set socket name and channel on RemoteHost to match local address
	FString NetDriverNameStr;
#if ENGINE_MAJOR_VERSION >= 5 && ENGINE_MINOR_VERSION >= 6
	NetDriverNameStr = GetNetDriverDefinition().ToString();
#else
	NetDriverNameStr = NetDriverName.ToString();
#endif
	RemoteHost->SetSocketName(NetDriverNameStr);
	uint32 NameHash = GetTypeHash(NetDriverNameStr);
	RemoteHost->SetChannel(static_cast<uint8>(NameHash % 256));

	// Bind the socket to the local address (EOS socket binding)
	if (!CurSocket->Bind(*LocalAddress))
	{
		Error = TEXT("Could not bind socket");
		UE_LOG(LogNet, Error, TEXT("UNetDriverEOS: %s"), *Error);
		return false;
	}

	// Set the address to what was parsed (this will be used for the connection)
	// Note: LocalAddr in UNetDriver stores the address we're connecting TO (remote)
	LocalAddr = RemoteHost;

	// Initialize connectionless handler
	InitConnectionlessHandler();

	// Create an unreal connection to the server
	// Use UIpConnection since we don't have a custom EOS connection class
	UIpConnection* Connection = NewObject<UIpConnection>(NetConnectionClass);
	check(Connection);

	// Set it as the server connection before anything else so everything knows this is a client
	ServerConnection = Connection;
	Connection->InitLocalConnection(this, CurSocket, ConnectURL, USOCK_Pending);

	// Create initial client channels
	CreateInitialClientChannels();

	// Log the actual ProductUserId from the address
	char RemotePUIDStr[EOS_PRODUCTUSERID_MAX_LENGTH + 1];
	int32_t RemotePUIDStrSize = sizeof(RemotePUIDStr);
	EOS_ProductUserId RemotePUID = RemoteHost->GetProductUserId();
	if (EOS_ProductUserId_IsValid(RemotePUID) && EOS_ProductUserId_ToString(RemotePUID, RemotePUIDStr, &RemotePUIDStrSize) == EOS_EResult::EOS_Success)
	{
		UE_LOG(LogNet, Log, TEXT("UNetDriverEOS: InitConnect completed - Connected to ProductUserId: %s"), UTF8_TO_TCHAR(RemotePUIDStr));
	}
	else
	{
		UE_LOG(LogNet, Warning, TEXT("UNetDriverEOS: InitConnect completed but ProductUserId is invalid. Address: %s"), *RemoteHost->ToString(false));
	}
	return true;
}

bool UNetDriverEOS::InitListen(FNetworkNotify* InNotify, FURL& LocalURL, bool bReuseAddressAndPort, FString& Error)
{
	// Check if EOS is available
	bool bIsAvailableResult = IsAvailable();
	if (!bIsAvailableResult)
	{
		UE_LOG(LogNet, Log, TEXT("UNetDriverEOS: EOS not available, falling back to IpNetDriver"));
		return Super::InitListen(InNotify, LocalURL, bReuseAddressAndPort, Error);
	}

	// Initialize base (creates EOS socket)
	if (!InitBase(false, InNotify, LocalURL, bReuseAddressAndPort, Error))
	{
		return false;
	}

	// Initialize connectionless handler
	InitConnectionlessHandler();

	// Set up the socket for listening (EOS socket's Listen method)
	FSocket* CurSocket = GetSocket();
	if (!CurSocket)
	{
		Error = TEXT("Socket is null");
		UE_LOG(LogNet, Error, TEXT("UNetDriverEOS: %s"), *Error);
		return false;
	}

	if (!CurSocket->Listen(0))
	{
		Error = TEXT("Could not listen on EOS socket");
		UE_LOG(LogNet, Error, TEXT("UNetDriverEOS: %s"), *Error);
		return false;
	}

	UE_LOG(LogNet, Log, TEXT("UNetDriverEOS: InitListen completed - EOS P2P listen server initialized"));
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
