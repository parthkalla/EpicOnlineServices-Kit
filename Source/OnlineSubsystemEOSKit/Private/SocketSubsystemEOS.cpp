// Copyright (C) 2024, All Rights Reserved.

#include "SocketSubsystemEOS.h"
#include "InternetAddrEOS.h"
#include "SocketEOS.h"
#include "SocketTypes.h"
#include "Containers/Ticker.h"
#include "Misc/ConfigCacheIni.h"
#include "SocketSubsystemModule.h"
#include "Modules/ModuleManager.h"
#include "Misc/OutputDeviceRedirector.h"
#include "OnlineSubsystemUtils.h"

#if WITH_EOS_SDK
	#include "eos_sdk.h"
#endif

TArray<FSocketSubsystemEOS*> FSocketSubsystemEOS::SocketSubsystemEOSInstances;
TMap<UWorld*, FSocketSubsystemEOS*> FSocketSubsystemEOS::SocketSubsystemEOSPerWorldMap;

FSocketSubsystemEOS::FSocketSubsystemEOS(void* InPlatformHandle, ISocketSubsystemEOSUtilsPtr InUtils)
	: P2PHandle(nullptr)
	, Utils(InUtils)
	, LastSocketError(ESocketErrors::SE_NO_ERROR)
{
#if WITH_EOS_SDK
	if (InPlatformHandle)
	{
		EOS_HPlatform EOSPlatformHandle = static_cast<EOS_HPlatform>(InPlatformHandle);
		P2PHandle = EOS_Platform_GetP2PInterface(EOSPlatformHandle);
		if (P2PHandle == nullptr)
		{
			UE_LOG(LogNet, Error, TEXT("FSocketSubsystemEOS: failed to init EOS platform, couldn't get p2p handle"));
		}
	}
#endif
}

FSocketSubsystemEOS::~FSocketSubsystemEOS()
{
	// Close all tracked sockets BEFORE destroying Utils
	// This ensures sockets can still access Utils during their cleanup
	for (auto& SocketPtr : TrackedSockets)
	{
		if (SocketPtr.IsValid())
		{
			SocketPtr->Close();
		}
	}
	TrackedSockets.Empty();
	
	// Now safe to clear Utils
	Utils = nullptr;
}

FSocketSubsystemEOS* FSocketSubsystemEOS::GetSocketSubsystemForWorld(UWorld* InWorld)
{
	FSocketSubsystemEOS** Result = SocketSubsystemEOSPerWorldMap.Find(InWorld);

	if(!Result)
	{
		for (FSocketSubsystemEOS* SocketSubsystem : SocketSubsystemEOSInstances)
		{
			const UWorld* NewWorld = GetWorldForOnline(SocketSubsystem->Utils->GetSubsystemInstanceName());

			if (NewWorld == InWorld)
			{
				SocketSubsystemEOSPerWorldMap.Add(InWorld, SocketSubsystem);

				Result = &SocketSubsystem;

				break;
			}
		}
	}

	return Result ? *Result : nullptr;
}

bool FSocketSubsystemEOS::Init(FString& Error)
{
	SocketSubsystemEOSInstances.Add(this);

	FSocketSubsystemModule& SocketSubsystem = FModuleManager::LoadModuleChecked<FSocketSubsystemModule>("Sockets");
	SocketSubsystem.RegisterSocketSubsystem(EOS_SOCKETSUBSYSTEM, this, false);

	return true;
}

void FSocketSubsystemEOS::Shutdown()
{
	RemoveFromStaticContainers();

	// Destruct our sockets before we finish destructing, as they maintain a reference to us
	TrackedSockets.Reset();

	if (FSocketSubsystemModule* SocketSubsystem = FModuleManager::GetModulePtr<FSocketSubsystemModule>("Sockets"))
	{
		SocketSubsystem->UnregisterSocketSubsystem(EOS_SOCKETSUBSYSTEM);
	}
}

void FSocketSubsystemEOS::RemoveFromStaticContainers()
{
	for (TMap<UWorld*, FSocketSubsystemEOS*>::TIterator Iter(SocketSubsystemEOSPerWorldMap); Iter; ++Iter)
	{
		if (Iter.Value() == this)
		{
			Iter.RemoveCurrent();
		}
	}

	SocketSubsystemEOSInstances.Remove(this);
}

FSocket* FSocketSubsystemEOS::CreateSocket(const FName& SocketTypeName, const FString& SocketDescription, const FName& /*unused*/)
{
	return TrackedSockets.Emplace_GetRef(MakeUnique<FSocketEOS>(*this, SocketDescription)).Get();
}

FResolveInfoCached* FSocketSubsystemEOS::CreateResolveInfoCached(TSharedPtr<FInternetAddr> Addr) const
{
	return nullptr;
}

void FSocketSubsystemEOS::DestroySocket(FSocket* Socket)
{
	for (auto It = TrackedSockets.CreateIterator(); It; ++It)
	{
		if (It->IsValid() && It->Get() == Socket)
		{
			It.RemoveCurrent();
			return;
		}
	}
}

FAddressInfoResult FSocketSubsystemEOS::GetAddressInfo(const TCHAR* HostName, const TCHAR* ServiceName, EAddressInfoFlags /*unused*/, const FName /*unused*/, ESocketType /*unused*/)
{
	return FAddressInfoResult(HostName, ServiceName);
}

bool FSocketSubsystemEOS::RequiresChatDataBeSeparate()
{
	return false;
}

bool FSocketSubsystemEOS::RequiresEncryptedPackets()
{
	return false;
}

bool FSocketSubsystemEOS::GetHostName(FString& HostName)
{
	return false;
}

TSharedRef<FInternetAddr> FSocketSubsystemEOS::CreateInternetAddr()
{
	return MakeShared<FInternetAddrEOS>();
}

TSharedPtr<FInternetAddr> FSocketSubsystemEOS::GetAddressFromString(const FString& InString)
{
	bool bUnused;
	TSharedPtr<FInternetAddrEOS> NewAddress = StaticCastSharedRef<FInternetAddrEOS>(CreateInternetAddr());
	NewAddress->SetIp(*InString, bUnused);
	return NewAddress;
}

bool FSocketSubsystemEOS::HasNetworkDevice()
{
	return true;
}

const TCHAR* FSocketSubsystemEOS::GetSocketAPIName() const
{
	return TEXT("p2pSocketsEOS");
}

ESocketErrors FSocketSubsystemEOS::GetLastErrorCode()
{
	return TranslateErrorCode(LastSocketError);
}

ESocketErrors FSocketSubsystemEOS::TranslateErrorCode(int32 Code)
{
	return static_cast<ESocketErrors>(Code);
}

bool FSocketSubsystemEOS::GetLocalAdapterAddresses(TArray<TSharedPtr<FInternetAddr>>& OutAddresses)
{
	TSharedRef<FInternetAddr> AdapterAddress = GetLocalBindAddr(nullptr, *GLog);
	OutAddresses.Add(AdapterAddress);
	return true;
}

TArray<TSharedRef<FInternetAddr>> FSocketSubsystemEOS::GetLocalBindAddresses()
{
	TArray<TSharedRef<FInternetAddr>> OutAddresses;
	OutAddresses.Add(GetLocalBindAddr(nullptr, *GLog));
	return OutAddresses;
}

TSharedRef<FInternetAddr> FSocketSubsystemEOS::GetLocalBindAddr(FOutputDevice& Out)
{
	return GetLocalBindAddr(nullptr, Out);
}

#if WITH_EOS_SDK
EOS_HP2P FSocketSubsystemEOS::GetP2PHandle()
{
	check(P2PHandle != nullptr);
	return P2PHandle;
}

EOS_ProductUserId FSocketSubsystemEOS::GetLocalUserId()
{
	if (!Utils.IsValid())
	{
		UE_LOG(LogSocketSubsystemEOS, Warning, TEXT("GetLocalUserId called but Utils is invalid (likely during shutdown)"));
		return nullptr;
	}
	return Utils->GetLocalUserId();
}
#endif

TSharedRef<FInternetAddr> FSocketSubsystemEOS::GetLocalBindAddr(const UWorld* const OwningWorld, FOutputDevice& Out)
{
	TSharedRef<FInternetAddrEOS> BoundAddr = MakeShared<FInternetAddrEOS>();

#if WITH_EOS_SDK
	EOS_ProductUserId LocalUserId = GetLocalUserId();
	if (LocalUserId == nullptr)
	{
		UE_LOG(LogNet, Error, TEXT("No local user to send p2p packets with"));
		return BoundAddr;
	}
	BoundAddr->SetLocalUserId(LocalUserId);
#else
	return BoundAddr;
#endif

	FString SessionId = Utils->GetSessionId();

	if (SessionId.IsEmpty())
	{
		SessionId = FName(NAME_GameSession).ToString();
	}

	BoundAddr->SetSocketName(SessionId);

	return BoundAddr;
}

bool FSocketSubsystemEOS::IsSocketWaitSupported() const
{
	return false;
}

void FSocketSubsystemEOS::SetLastSocketError(const ESocketErrors NewSocketError)
{
	LastSocketError = NewSocketError;
}

bool FSocketSubsystemEOS::BindChannel(const FInternetAddrEOS& Address)
{
	if (!Address.IsValid())
	{
		SetLastSocketError(ESocketErrors::SE_EINVAL);
		UE_LOG(LogSocketSubsystemEOS, Warning, TEXT("BindChannel failed: Invalid address"));
		return false;
	}

	const uint8 Channel = Address.GetChannel();
	const FString SocketName = Address.GetSocketName();

	FChannelSet& ExistingBoundPorts = BoundAddresses.FindOrAdd(SocketName);
	
	// In EOS P2P, multiple sockets can use the same socket name + channel combination
	// (e.g., multiple clients connecting to the same host on the same machine)
	// So we allow the binding even if it's already in the set
	// The actual uniqueness is enforced by EOS P2P itself based on LocalUserId + RemoteUserId
	if (ExistingBoundPorts.Contains(Channel))
	{
		UE_LOG(LogSocketSubsystemEOS, Verbose, TEXT("BindChannel: Channel %d already in bound set for socket '%hs', but allowing (multiple clients can share same socket/channel in EOS P2P)"), 
			Channel, Address.GetSocketName());
		// Don't return false - allow the binding
		// The channel is already tracked, so we don't need to add it again
	}
	else
	{
		ExistingBoundPorts.Add(Channel);
		UE_LOG(LogSocketSubsystemEOS, Verbose, TEXT("BindChannel succeeded: Channel %d bound for socket '%s'"), Channel, *SocketName);
	}
	
	return true;
}

bool FSocketSubsystemEOS::UnbindChannel(const FInternetAddrEOS& Address)
{
	if (!Address.IsValid())
	{
		SetLastSocketError(ESocketErrors::SE_EINVAL);
		return false;
	}

	const FString SocketName = Address.GetSocketName();
	const uint8 Channel = Address.GetChannel();

	// Find our sessions collection of ports
	FChannelSet* ExistingBoundPorts = BoundAddresses.Find(SocketName);
	if (!ExistingBoundPorts)
	{
		SetLastSocketError(ESocketErrors::SE_ENOTSOCK);
		return false;
	}

	// Remove our port and check if we had it bound
	if (ExistingBoundPorts->Remove(Channel) == 0)
	{
		SetLastSocketError(ESocketErrors::SE_ENOTSOCK);
		return false;
	}

	// Remove any empty sets
	if (ExistingBoundPorts->Num() == 0)
	{
		BoundAddresses.Remove(SocketName);
		ExistingBoundPorts = nullptr;
	}

	return true;
}

TUniquePtr<FSocket> FSocketSubsystemEOS::CreateUniqueSocket(const FName& SocketType, const FString& SocketDescription, const FName& ProtocolType)
{
	// Note: This method is kept for compatibility but is not currently used.
	// NetDriverEOS uses CreateSocket() directly instead.
	// If needed in the future, this would need to return a unique_ptr with a custom deleter
	// that doesn't delete the socket (since it's managed by TrackedSockets).
	// For now, we return an empty unique_ptr to avoid compilation errors.
	return TUniquePtr<FSocket>();
}

bool FSocketSubsystemEOS::BindNextPort(FSocket* Socket, const FInternetAddrEOS& Address, int32 PortCount, int32 PortIncrement)
{
	// For EOS P2P, we bind to a local address
	// For clients: Use the socket's existing local address (from InitBase) but match the remote's socket name/channel
	// For hosts: Use the address parameter directly
	if (!Socket)
	{
		SetLastSocketError(ESocketErrors::SE_EINVAL);
		UE_LOG(LogSocketSubsystemEOS, Warning, TEXT("BindNextPort failed: Invalid socket"));
		return false;
	}

	// Check if socket already has a local address set (from InitBase)
	FSocketEOS* EOSSocket = static_cast<FSocketEOS*>(Socket);
	FInternetAddrEOS LocalBindAddress;
	
	if (EOSSocket)
	{
		FInternetAddrEOS ExistingLocalAddr;
		EOSSocket->GetAddress(ExistingLocalAddr);
		
		// If socket already has a valid local address, use it but update socket name/channel to match remote
		if (ExistingLocalAddr.IsValid() && ExistingLocalAddr.GetLocalUserId() != nullptr)
		{
			LocalBindAddress = ExistingLocalAddr;
			// Update to match remote's socket name and channel (required for P2P to work)
			LocalBindAddress.SetSocketName(Address.GetSocketName());
			LocalBindAddress.SetChannel(Address.GetChannel());
		}
		else
		{
			// No existing address, create new one matching remote
			LocalBindAddress.SetLocalUserId(GetLocalUserId());
			LocalBindAddress.SetSocketName(Address.GetSocketName());
			LocalBindAddress.SetChannel(Address.GetChannel());
		}
	}
	else
	{
		// Fallback: create new address matching remote
		LocalBindAddress.SetLocalUserId(GetLocalUserId());
		LocalBindAddress.SetSocketName(Address.GetSocketName());
		LocalBindAddress.SetChannel(Address.GetChannel());
	}

	UE_LOG(LogSocketSubsystemEOS, Verbose, TEXT("BindNextPort: Attempting to bind socket '%hs' to channel %d"), 
		Address.GetSocketName(), Address.GetChannel());

	// Check if this socket already has this channel bound (to avoid double-binding)
	// If the socket already has the same socket name and channel, we can skip BindChannel
	bool bNeedsChannelBind = true;
	if (EOSSocket)
	{
		FInternetAddrEOS CurrentLocalAddr;
		EOSSocket->GetAddress(CurrentLocalAddr);
		if (CurrentLocalAddr.IsValid() && 
			CurrentLocalAddr.GetSocketName() == LocalBindAddress.GetSocketName() &&
			CurrentLocalAddr.GetChannel() == LocalBindAddress.GetChannel())
		{
			// Socket already has this exact binding, skip channel bind
			bNeedsChannelBind = false;
			UE_LOG(LogSocketSubsystemEOS, Verbose, TEXT("BindNextPort: Socket already has channel %d bound for socket '%hs', skipping BindChannel"), 
				Address.GetChannel(), Address.GetSocketName());
		}
	}

	// Bind the channel first (if needed)
	if (bNeedsChannelBind)
	{
		if (!BindChannel(LocalBindAddress))
		{
			UE_LOG(LogSocketSubsystemEOS, Error, TEXT("BindNextPort failed: Could not bind channel %d for socket '%hs'. Error: %d"), 
				Address.GetChannel(), Address.GetSocketName(), (int32)GetLastErrorCode());
			return false;
		}
	}

	// Then bind the socket
	if (!Socket->Bind(LocalBindAddress))
	{
		UE_LOG(LogSocketSubsystemEOS, Error, TEXT("BindNextPort failed: Socket->Bind() failed for socket '%hs' channel %d"), 
			Address.GetSocketName(), Address.GetChannel());
		if (bNeedsChannelBind)
		{
			UnbindChannel(LocalBindAddress);
		}
		return false;
	}

	// Update the socket's local address
	if (EOSSocket)
	{
		EOSSocket->SetLocalAddress(LocalBindAddress);
	}

	UE_LOG(LogSocketSubsystemEOS, Verbose, TEXT("BindNextPort succeeded: Socket '%hs' bound to channel %d"), 
		Address.GetSocketName(), Address.GetChannel());
	return true;
}
