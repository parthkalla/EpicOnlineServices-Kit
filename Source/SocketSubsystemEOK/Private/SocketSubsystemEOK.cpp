// Copyright Epic Games, Inc. All Rights Reserved.

#include "SocketSubsystemEOK.h"
#include "InternetAddrEOK.h"
#include "SocketEOK.h"
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

TArray<FSocketSubsystemEOK*> FSocketSubsystemEOK::SocketSubsystemEOSInstances;
TMap<UWorld*, FSocketSubsystemEOK*> FSocketSubsystemEOK::SocketSubsystemEOSPerWorldMap;

FSocketSubsystemEOK::FSocketSubsystemEOK(IEOKPlatformHandlePtr InPlatformHandle, ISocketSubsystemEOSUtilsPtr InUtils)
	: P2PHandle(nullptr)
	, Utils(InUtils)
	, LastSocketError(ESocketErrors::SE_NO_ERROR)
{
#if WITH_EOS_SDK
	P2PHandle = EOS_Platform_GetP2PInterface(*InPlatformHandle);
	if (P2PHandle == nullptr)
	{
		UE_LOG(LogSocketSubsystemEOS, Error, TEXT("FOnlineSubsystemEOS: failed to init EOS platform, couldn't get p2p handle"));
	}
#endif
}

FSocketSubsystemEOK::~FSocketSubsystemEOK()
{
	Utils = nullptr;
}

FSocketSubsystemEOK* FSocketSubsystemEOK::GetSocketSubsystemForWorld(UWorld* InWorld)
{
	FSocketSubsystemEOK** Result = SocketSubsystemEOSPerWorldMap.Find(InWorld);

	if(!Result)
	{
		for (FSocketSubsystemEOK* SocketSubsystem : SocketSubsystemEOSInstances)
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

bool FSocketSubsystemEOK::Init(FString& Error)
{
	SocketSubsystemEOSInstances.Add(this);

	FSocketSubsystemModule& SocketSubsystem = FModuleManager::LoadModuleChecked<FSocketSubsystemModule>("Sockets");
	SocketSubsystem.RegisterSocketSubsystem(EOS_SOCKETSUBSYSTEM, this, false);

	return true;
}

void FSocketSubsystemEOK::Shutdown()
{
	RemoveFromStaticContainers();

	// Destruct our sockets before we finish destructing, as they maintain a reference to us
	TrackedSockets.Reset();

	if (FSocketSubsystemModule* SocketSubsystem = FModuleManager::GetModulePtr<FSocketSubsystemModule>("Sockets"))
	{
		SocketSubsystem->UnregisterSocketSubsystem(EOS_SOCKETSUBSYSTEM);
	}
}

void FSocketSubsystemEOK::RemoveFromStaticContainers()
{
	for (TMap<UWorld*, FSocketSubsystemEOK*>::TIterator Iter(SocketSubsystemEOSPerWorldMap); Iter; ++Iter)
	{
		if (Iter.Value() == this)
		{
			Iter.RemoveCurrent();
		}
	}

	SocketSubsystemEOSInstances.Remove(this);
}

FSocket* FSocketSubsystemEOK::CreateSocket(const FName& SocketTypeName, const FString& SocketDescription, const FName& /*unused*/)
{
	return TrackedSockets.Emplace_GetRef(MakeUnique<FSocketEOS>(*this, SocketDescription)).Get();
}

FResolveInfoCached* FSocketSubsystemEOK::CreateResolveInfoCached(TSharedPtr<FInternetAddr> Addr) const
{
	return nullptr;
}

void FSocketSubsystemEOK::DestroySocket(FSocket* Socket)
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

FAddressInfoResult FSocketSubsystemEOK::GetAddressInfo(const TCHAR* HostName, const TCHAR* ServiceName, EAddressInfoFlags /*unused*/, const FName /*unused*/, ESocketType /*unused*/)
{
	return FAddressInfoResult(HostName, ServiceName);
}

bool FSocketSubsystemEOK::RequiresChatDataBeSeparate()
{
	return false;
}

bool FSocketSubsystemEOK::RequiresEncryptedPackets()
{
	return false;
}

bool FSocketSubsystemEOK::GetHostName(FString& HostName)
{
	return false;
}

TSharedRef<FInternetAddr> FSocketSubsystemEOK::CreateInternetAddr()
{
	return MakeShared<FInternetAddrEOS>();
}

TSharedPtr<FInternetAddr> FSocketSubsystemEOK::GetAddressFromString(const FString& InString)
{
	bool bUnused;
	TSharedPtr<FInternetAddrEOS> NewAddress = StaticCastSharedRef<FInternetAddrEOS>(CreateInternetAddr());
	NewAddress->SetIp(*InString, bUnused);
	return NewAddress;
}

bool FSocketSubsystemEOK::HasNetworkDevice()
{
	return true;
}

const TCHAR* FSocketSubsystemEOK::GetSocketAPIName() const
{
	return TEXT("p2pSocketsEOS");
}

ESocketErrors FSocketSubsystemEOK::GetLastErrorCode()
{
	return TranslateErrorCode(LastSocketError);
}

ESocketErrors FSocketSubsystemEOK::TranslateErrorCode(int32 Code)
{
	return static_cast<ESocketErrors>(Code);
}

bool FSocketSubsystemEOK::GetLocalAdapterAddresses(TArray<TSharedPtr<FInternetAddr>>& OutAddresses)
{
	TSharedRef<FInternetAddr> AdapterAddress = GetLocalBindAddr(nullptr, *GLog);
	OutAddresses.Add(AdapterAddress);
	return true;
}

TArray<TSharedRef<FInternetAddr>> FSocketSubsystemEOK::GetLocalBindAddresses()
{
	TArray<TSharedRef<FInternetAddr>> OutAddresses;
	OutAddresses.Add(GetLocalBindAddr(nullptr, *GLog));
	return OutAddresses;
}

TSharedRef<FInternetAddr> FSocketSubsystemEOK::GetLocalBindAddr(FOutputDevice& Out)
{
	return GetLocalBindAddr(nullptr, Out);
}

#if WITH_EOS_SDK
EOS_HP2P FSocketSubsystemEOK::GetP2PHandle()
{
	check(P2PHandle != nullptr);
	return P2PHandle;
}

EOS_ProductUserId FSocketSubsystemEOK::GetLocalUserId()
{
	return Utils->GetLocalUserId();
}
#endif

TSharedRef<FInternetAddr> FSocketSubsystemEOK::GetLocalBindAddr(const UWorld* const OwningWorld, FOutputDevice& Out)
{
	TSharedRef<FInternetAddrEOS> BoundAddr = MakeShared<FInternetAddrEOS>();

#if WITH_EOS_SDK
	EOS_ProductUserId LocalUserId = GetLocalUserId();
	if (LocalUserId == nullptr)
	{
		UE_LOG(LogSocketSubsystemEOS, Error, TEXT("No local user to send p2p packets with"));
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

bool FSocketSubsystemEOK::IsSocketWaitSupported() const
{
	return false;
}

void FSocketSubsystemEOK::SetLastSocketError(const ESocketErrors NewSocketError)
{
	LastSocketError = NewSocketError;
}

bool FSocketSubsystemEOK::BindChannel(const FInternetAddrEOS& Address)
{
	if (!Address.IsValid())
	{
		SetLastSocketError(ESocketErrors::SE_EINVAL);
		return false;
	}

	const uint8 Channel = Address.GetChannel();

	FChannelSet& ExistingBoundPorts = BoundAddresses.FindOrAdd(Address.GetSocketName());
	if (ExistingBoundPorts.Contains(Channel))
	{
		SetLastSocketError(ESocketErrors::SE_EADDRINUSE);
		return false;
	}

	ExistingBoundPorts.Add(Channel);
	return true;
}

bool FSocketSubsystemEOK::UnbindChannel(const FInternetAddrEOS& Address)
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
