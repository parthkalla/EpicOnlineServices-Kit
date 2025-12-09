// Copyright (C) 2024, All Rights Reserved.

#include "NetDriverEOS.h"
#include "NetConnectionEOS.h"
#include "OnlineBeaconHost.h"
#include "OnlineBeaconClient.h"
#include "EngineUtils.h"
#include "SocketEOS.h"
#include "SocketSubsystemEOS.h"
#include "Misc/EngineVersionComparison.h"
#include "Engine/Engine.h"
#include "Misc/ConfigCacheIni.h"
#include "InternetAddrEOS.h"
#include "IpConnection.h"

#define EOS_CONNECTION_URL_PREFIX TEXT("EOS")

#if ENGINE_MAJOR_VERSION >= 5
#include UE_INLINE_GENERATED_CPP_BY_NAME(NetDriverEOS)
#endif

UNetDriverEOS::UNetDriverEOS(const FObjectInitializer& ObjectInitializer)
	: UIpNetDriver(ObjectInitializer)
{
	bIsPassthrough = false;
	
	// Force it to use your class
	NetConnectionClass = UNetConnectionEOS::StaticClass();
	
	// DOUBLE CHECK: Ensure the string path matches your actual plugin name
	NetConnectionClassName = TEXT("/Script/OnlineSubsystemEOSKit.NetConnectionEOS");
	
	UE_LOG(LogTemp, Warning, TEXT("✅ UNetDriverEOS: CONSTRUCTOR - Set NetConnectionClass to UNetConnectionEOS: %s"), 
		*NetConnectionClass->GetName());
	
#if ENGINE_MAJOR_VERSION >= 5 && ENGINE_MINOR_VERSION >= 6
	// Check for deprecated config in UE 5.6+
	bool bUnused;
	if (GConfig->GetBool(TEXT("/Script/OnlineSubsystemEOSKit.NetDriverEOS"), TEXT("bIsUsingP2PSockets"), bUnused, GEngineIni))
	{
		UE_LOG(LogTemp, Warning, TEXT("EOSKit NetDriver: bIsUsingP2PSockets is deprecated, please remove any related config values"));
	}
#else
	// For UE 5.5 and below, initialize bIsUsingP2PSockets from config
	if (!GConfig->GetBool(TEXT("/Script/OnlineSubsystemEOSKit.NetDriverEOS"), TEXT("bIsUsingP2PSockets"), bIsUsingP2PSockets, GEngineIni))
	{
		bIsUsingP2PSockets = true; // Default to true
	}
#endif
}

bool UNetDriverEOS::IsAvailable() const
{
	// Use passthrough sockets if we are a dedicated server
	if (IsRunningDedicatedServer())
	{
		return false;
	}

	if (ISocketSubsystem* SocketSubsystem = ISocketSubsystem::Get(EOS_SOCKETSUBSYSTEM))
	{
		return true;
	}

	return false;
}

bool UNetDriverEOS::InitBase(bool bInitAsClient, FNetworkNotify* InNotify, const FURL& URL, bool bReuseAddressAndPort, FString& Error)
{
	if (bIsPassthrough)
	{
		UE_LOG(LogTemp, Verbose, TEXT("Running as pass-through"));
		return Super::InitBase(bInitAsClient, InNotify, URL, bReuseAddressAndPort, Error);
	}
	if (!UNetDriver::InitBase(bInitAsClient, InNotify, URL, bReuseAddressAndPort, Error))
	{
		UE_LOG(LogTemp, Warning, TEXT("Failed to init driver base"));
		return false;
	}

	FSocketSubsystemEOS* const SocketSubsystem = static_cast<FSocketSubsystemEOS*>(GetSocketSubsystem());
	if (!SocketSubsystem)
	{
		if(!GetSocketSubsystem())
		{
			UE_LOG(LogTemp, Warning, TEXT("Could not get socket subsystem that is the base of EOS"));
		}
		UE_LOG(LogTemp, Warning, TEXT("Could not get socket subsystem"));
		return false;
	}

	// We don't care if our world is null, everything we uses handles it fine
	const UWorld* const MyWorld = FindWorld();

	// Get our local address (proves we're logged in)
	TSharedRef<FInternetAddr> LocalAddress = SocketSubsystem->GetLocalBindAddr(MyWorld, *GLog);
	if (!LocalAddress->IsValid())
	{
		// Not logged in?
		Error = TEXT("Could not bind local address");
		UE_LOG(LogTemp, Warning, TEXT("Could not bind local address"));
		return false;
	}


	// Create socket directly - it's managed by TrackedSockets
	FSocket* NewSocket = SocketSubsystem->CreateSocket(NAME_DGram, TEXT("UE4"), NAME_None);
	// Create a shared ptr with a no-op deleter since the socket is managed by TrackedSockets
	TSharedPtr<FSocket> SharedSocket(NewSocket, [](FSocket*) { /* Socket is managed by TrackedSockets */ });

	SetSocketAndLocalAddress(SharedSocket);

	if (GetSocket() == nullptr)
	{
		UE_LOG(LogTemp, Warning, TEXT("Could not create socket"));
		return false;
	}

	// Store our local address and set our port
	TSharedRef<FInternetAddrEOS> EOSLocalAddress = StaticCastSharedRef<FInternetAddrEOS>(LocalAddress);
	
	// Set LocalUserId (always needed)
	EOSLocalAddress->SetLocalUserId(SocketSubsystem->GetLocalUserId());
	
	if(IsBeaconDriver())
	{
		//Till we have a better solution, we will use a hardcoded port for the beacon driver
		EOSLocalAddress->SetSocketName(TEXT("BeaconSession"));
		// We will also use a hardcoded channel for the beacon driver
		EOSLocalAddress->SetChannel(71);
	}
	else
	{
		// For clients (bInitAsClient == true), don't set socket name/channel here
		// They will be set in BindNextPort() to match the remote host
		// For hosts (bInitAsClient == false), set them now
		if (!bInitAsClient)
		{
#if ENGINE_MAJOR_VERSION >= 5 && ENGINE_MINOR_VERSION >= 6
			// UE 5.6+ uses NetDriverDefinition instead of NetDriverName
			FString NetDriverDefinitionStr = GetNetDriverDefinition().ToString();
			EOSLocalAddress->SetChannel(GetTypeHash(NetDriverDefinitionStr));
			EOSLocalAddress->SetSocketName(NetDriverDefinitionStr);
#else
			// UE 5.5 and below use NetDriverName
			EOSLocalAddress->SetChannel(GetTypeHash(NetDriverName.ToString()));
			EOSLocalAddress->SetSocketName(NetDriverName.ToString());
#endif
		}
		// For clients, socket name and channel will be set in BindNextPort()
	}

	static_cast<FSocketEOS*>(GetSocket())->SetLocalAddress(*EOSLocalAddress);

	LocalAddr = LocalAddress;

	return true;
}

bool UNetDriverEOS::InitConnect(FNetworkNotify* InNotify, const FURL& ConnectURL, FString& Error)
{
	
	bool bIsEOSURL = ConnectURL.Host.StartsWith(EOS_CONNECTION_URL_PREFIX, ESearchCase::IgnoreCase);
	bool bIsAvailableResult = IsAvailable();
	
	
#if ENGINE_MAJOR_VERSION >= 5 && ENGINE_MINOR_VERSION >= 6
	// UE 5.6+ removed bIsUsingP2PSockets checks - engine now always uses EOS sockets when available
	if (!bIsAvailableResult || !bIsEOSURL)
#else
	// UE 5.5 and below still check bIsUsingP2PSockets
	if (!bIsUsingP2PSockets || !bIsAvailableResult || !bIsEOSURL)
#endif
	{

		bIsPassthrough = true;
		return Super::InitConnect(InNotify, ConnectURL, Error);
	}

	bool bIsValid = false;
	TSharedRef<FInternetAddrEOS> RemoteHost = MakeShared<FInternetAddrEOS>();
	RemoteHost->SetIp(*ConnectURL.Host, bIsValid);
	if (!bIsValid || ConnectURL.Port < 0)
	{
		Error = TEXT("Invalid remote address");
		UE_LOG(LogTemp, Warning, TEXT("Invalid Remote Address. ConnectUrl = (%s)"), *ConnectURL.ToString());
		return false;
	}


	if (!InitBase(true, InNotify, ConnectURL, false, Error))
	{
		return false;
	}

	// Set the address to what was parsed (us + remote)
	// Note: LocalAddr in UNetDriver stores the address we're connecting TO (remote)
	// Ensure RemoteHost has our LocalUserId set (needed for address validation and EOS P2P)
	FSocketSubsystemEOS* const SocketSubsystem = static_cast<FSocketSubsystemEOS*>(GetSocketSubsystem());
	if (SocketSubsystem)
	{
		RemoteHost->SetLocalUserId(SocketSubsystem->GetLocalUserId());
	}
	
	LocalAddr = RemoteHost;
	
	UE_LOG(LogNet, Verbose, TEXT("UNetDriverEOS::InitConnect: Parsed RemoteHost - LocalUserId: %s, RemoteUserId: %s, SocketName: %s, Channel: %d"), 
		RemoteHost->GetLocalUserId() ? TEXT("Valid") : TEXT("Invalid"),
		RemoteHost->GetRemoteUserId() ? TEXT("Valid") : TEXT("Invalid"),
		UTF8_TO_TCHAR(RemoteHost->GetSocketName()),
		RemoteHost->GetChannel());

	// Reference to our newly created socket
	FSocket* CurSocket = GetSocket();

	// Bind our local port (BindNextPort will create a local address with socket name/channel from RemoteHost)
	check(SocketSubsystem);
	if (!SocketSubsystem->BindNextPort(CurSocket, *RemoteHost, MaxPortCountToTry + 1, 1))
	{
		// Failure
		Error = TEXT("Could not bind local port");
		UE_LOG(LogTemp, Warning, TEXT("Could not bind local port in %d attempts"), MaxPortCountToTry);
		return false;
	}

	// Create an unreal connection to the server (use custom EOS connection class)
	UNetConnectionEOS* EOSConnection = NewObject<UNetConnectionEOS>(NetConnectionClass);
	check(EOSConnection);

	// Set it as the server connection before everything else so everything knows this is a client
	// Cast to base class for assignment (ServerConnection is TObjectPtr<UNetConnection>)
	ServerConnection = EOSConnection;
	
	// Ensure the connection's RemoteAddr is set to our parsed RemoteHost (EOS address)
	// This is critical for EOS P2P - the connection needs the remote ProductUserId
	EOSConnection->RemoteAddr = RemoteHost;
	
	EOSConnection->InitLocalConnection(this, CurSocket, ConnectURL, USOCK_Pending);

	// Log the connection setup for debugging
	UE_LOG(LogNet, Warning, TEXT("UNetDriverEOS::InitConnect: ✅ Connection created successfully"));
	UE_LOG(LogNet, Warning, TEXT("UNetDriverEOS::InitConnect: RemoteAddr: %s"), 
		*RemoteHost->ToString(false));
	UE_LOG(LogNet, Warning, TEXT("UNetDriverEOS::InitConnect: Socket: %p, Connection: %p"), CurSocket, EOSConnection);

	CreateInitialClientChannels();

	UE_LOG(LogNet, Warning, TEXT("UNetDriverEOS::InitConnect: ✅ InitConnect completed - connection should start sending handshake packets"));

	return true;
}

bool UNetDriverEOS::InitListen(FNetworkNotify* InNotify, FURL& LocalURL, bool bReuseAddressAndPort, FString& Error)
{
	
	bool bIsAvailableResult = IsAvailable();
	bool bHasLanMatch = LocalURL.HasOption(TEXT("bIsLanMatch"));
	bool bUseIPSockets = LocalURL.HasOption(TEXT("bUseIPSockets"));
	
	
#if ENGINE_MAJOR_VERSION >= 5 && ENGINE_MINOR_VERSION >= 6
	// UE 5.6+ removed bIsUsingP2PSockets checks - engine now always uses EOS sockets when available
	if (!bIsAvailableResult || bHasLanMatch || bUseIPSockets)
#else
	// UE 5.5 and below still check bIsUsingP2PSockets
	if (!bIsUsingP2PSockets || !bIsAvailableResult || bHasLanMatch || bUseIPSockets)
#endif
	{

		bIsPassthrough = true;
		return Super::InitListen(InNotify, LocalURL, bReuseAddressAndPort, Error);
	}


	if (!InitBase(false, InNotify, LocalURL, bReuseAddressAndPort, Error))
	{
		return false;
	}

	// Bind our specified port if provided
	FSocket* CurSocket = GetSocket();
	if (!CurSocket->Listen(0))
	{
		Error = TEXT("Could not listen");
		UE_LOG(LogTemp, Warning, TEXT("Could not listen on socket"));
		return false;
	}

	InitConnectionlessHandler();

	UE_LOG(LogTemp, Verbose, TEXT("Initialized as an EOSP2P listen server"));

	// Ensure we have a valid world context
	UWorld* TWorld = FindWorld();
	if (!TWorld)
	{
		Error = TEXT("Invalid world context");
		return false;
	}
	
	// Check if a Beacon Host already exists
	bool bBeaconHostExists = false;
    for (TActorIterator<AOnlineBeaconHost> It(TWorld); It; ++It)
	{
		if (*It)
		{
			bBeaconHostExists = true;
			break;
		}
	}

	/* Initialize Beacon Host if it doesn't already exist
	if (!bBeaconHostExists)
	{
        AOnlineBeaconHost* BeaconHost = TWorld->SpawnActor<AOnlineBeaconHost>();
		if (BeaconHost)
		{
			UE_LOG(LogTemp, Error, TEXT("Spawned Beacon Host"));
			BeaconHost->SetNetDriverName(NAME_BeaconNetDriver);
			BeaconHost->InitHost();
			BeaconHost->PauseBeaconRequests(false);
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("Failed to spawn Beacon Host"));
		}
	}
	*/

	return true;
}

ISocketSubsystem* UNetDriverEOS::GetSocketSubsystem()
{
	if (bIsPassthrough)
	{
		return ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM);
	}
	UWorld* CurrentWorld = FindWorld();
	FSocketSubsystemEOS* DefaultSocketSubsystem = static_cast<FSocketSubsystemEOS*>(ISocketSubsystem::Get(EOS_SOCKETSUBSYSTEM));
	if (!DefaultSocketSubsystem)
	{
		return ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM);
	}
	return DefaultSocketSubsystem->GetSocketSubsystemForWorld(CurrentWorld);
}

void UNetDriverEOS::Shutdown()
{
	Super::Shutdown();

	// Kill our P2P sessions now, instead of when garbage collection kicks in later
	if (!bIsPassthrough)
	{
		if(ServerConnection)
		{
			// Close EOS connection if it's an EOS connection
			// Access socket - in UE 5.5, GetSocket() might not be available, so we use Socket directly
			FSocket* CurSocket = nullptr;
			if (UIpConnection* IpConnection = Cast<UIpConnection>(ServerConnection))
			{
#if ENGINE_MAJOR_VERSION == 5 && ENGINE_MINOR_VERSION >= 6
				CurSocket = IpConnection->GetSocket();
#else
				// UE 5.5 - Socket is accessible directly (deprecated but still works)
				PRAGMA_DISABLE_DEPRECATION_WARNINGS
				CurSocket = IpConnection->Socket;
				PRAGMA_ENABLE_DEPRECATION_WARNINGS
#endif
			}
			if (CurSocket)
			{
				FSocketEOS* EOSSocket = static_cast<FSocketEOS*>(CurSocket);
				if (EOSSocket && ServerConnection->RemoteAddr.IsValid())
				{
					// Safely cast to FInternetAddrEOS - check if address string indicates EOS format
					TSharedPtr<FInternetAddr> RemoteAddr = ServerConnection->RemoteAddr;
					if (RemoteAddr.IsValid())
					{
						FString AddrString = RemoteAddr->ToString(false);
						// Check if it's an EOS address by checking the format
						if (AddrString.StartsWith(TEXT("EOS:")))
						{
							TSharedPtr<FInternetAddrEOS> RemoteAddrEOS = StaticCastSharedPtr<FInternetAddrEOS>(RemoteAddr);
							if (RemoteAddrEOS.IsValid())
							{
								EOSSocket->Close(*RemoteAddrEOS);
							}
						}
					}
				}
			}
		}
		for (UNetConnection* Client : ClientConnections)
		{
			if(Client)
			{
				// Access socket - in UE 5.5, GetSocket() might not be available, so we use Socket directly
				FSocket* CurSocket = nullptr;
				if (UIpConnection* IpConnection = Cast<UIpConnection>(Client))
				{
#if ENGINE_MAJOR_VERSION == 5 && ENGINE_MINOR_VERSION >= 6
					CurSocket = IpConnection->GetSocket();
#else
					// UE 5.5 - Socket is accessible directly (deprecated but still works)
					PRAGMA_DISABLE_DEPRECATION_WARNINGS
					CurSocket = IpConnection->Socket;
					PRAGMA_ENABLE_DEPRECATION_WARNINGS
#endif
				}
				if (CurSocket)
				{
					FSocketEOS* EOSSocket = static_cast<FSocketEOS*>(CurSocket);
					if (EOSSocket && Client->RemoteAddr.IsValid())
					{
						// Safely cast to FInternetAddrEOS - check if address string indicates EOS format
						TSharedPtr<FInternetAddr> RemoteAddr = Client->RemoteAddr;
						if (RemoteAddr.IsValid())
						{
							FString AddrString = RemoteAddr->ToString(false);
							// Check if it's an EOS address by checking the format
							if (AddrString.StartsWith(TEXT("EOS:")))
							{
								TSharedPtr<FInternetAddrEOS> RemoteAddrEOS = StaticCastSharedPtr<FInternetAddrEOS>(RemoteAddr);
								if (RemoteAddrEOS.IsValid())
								{
									EOSSocket->Close(*RemoteAddrEOS);
								}
							}
						}
					}
				}
			}
		}
	}
}

int UNetDriverEOS::GetClientPort()
{
	if (bIsPassthrough)
	{
		return Super::GetClientPort();
	}

	// Starting range of dynamic/private/ephemeral ports
	return 49152;
}

bool UNetDriverEOS::IsBeaconDriver() const
{
	if (!GEngine) return false;

	for (const auto &WorldContext : GEngine->GetWorldContexts())
	{
		if (UWorld *ItWorld = WorldContext.World())
		{
			for (AOnlineBeacon* Beacon : TActorRange<AOnlineBeacon>(ItWorld))
			{
				if (Beacon->GetNetDriver() == this)
				{
					return true;
				}
			}
		}
	}
	return false;
}

	

UWorld* UNetDriverEOS::FindWorld() const
{
	UWorld* MyWorld = GetWorld();
	
	// If we don't have a world, we may be a pending net driver
	if (!MyWorld && GEngine)
	{
		if (FWorldContext* WorldContext = GEngine->GetWorldContextFromPendingNetGameNetDriver(this))
		{
			MyWorld = WorldContext->World();
		}
	}

	if(!MyWorld)
	{
		if (GEngine != nullptr)
		{
			for (const auto &WorldContext : GEngine->GetWorldContexts())
			{
				UWorld *ItWorld = WorldContext.World();
				if (ItWorld != nullptr)
				{
					for (TActorIterator<AOnlineBeacon> It(ItWorld); It; ++It)
					{
						if (It->GetNetDriver() == this)
						{
							return ItWorld;
						}
					}
				}
			}
		}
	}

	return MyWorld;
}
