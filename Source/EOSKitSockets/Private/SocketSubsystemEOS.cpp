// Copyright (C) 2024, All Rights Reserved.

#include "SocketSubsystemEOS.h"
#include "SocketEOS.h"
#include "EOSKitSubsystem.h"
#include "Misc/CommandLine.h"
#include "Misc/Parse.h"

FSocketSubsystemEOS* FSocketSubsystemEOS::SocketSingleton = nullptr;

FSocketSubsystemEOS* FSocketSubsystemEOS::Create()
{
	if (SocketSingleton == nullptr)
	{
		SocketSingleton = new FSocketSubsystemEOS();
	}

	return SocketSingleton;
}

void FSocketSubsystemEOS::Destroy()
{
	if (SocketSingleton != nullptr)
	{
		SocketSingleton->Shutdown();
		delete SocketSingleton;
		SocketSingleton = nullptr;
	}
}

FSocketSubsystemEOS::FSocketSubsystemEOS()
	: P2PHandle(nullptr)
	, LocalProductUserId(nullptr)
	, LastSocketError(SE_NO_ERROR)
{
}

bool FSocketSubsystemEOS::Init(FString& Error)
{
	// Get the EOSKit subsystem to retrieve the P2P handle and local user ID
	if (GEngine)
	{
		UGameInstance* GameInstance = nullptr;
		
		// Try to get the game instance from the first world
		for (const FWorldContext& Context : GEngine->GetWorldContexts())
		{
			if (Context.World() && Context.World()->GetGameInstance())
			{
				GameInstance = Context.World()->GetGameInstance();
				break;
			}
		}

		if (GameInstance)
		{
			UEOSKitSubsystem* EOSKitSubsystem = GameInstance->GetSubsystem<UEOSKitSubsystem>();
			if (EOSKitSubsystem && EOSKitSubsystem->GetPlatformHandle())
			{
				P2PHandle = EOS_Platform_GetP2PInterface(EOSKitSubsystem->GetPlatformHandle());
				LocalProductUserId = EOSKitSubsystem->GetProductUserId();

				if (P2PHandle && LocalProductUserId)
				{
					UE_LOG(LogNet, Log, TEXT("FSocketSubsystemEOS: Initialized successfully"));
					return true;
				}
				else
				{
					Error = TEXT("Failed to get P2P handle or ProductUserId from EOSKit");
					UE_LOG(LogNet, Error, TEXT("FSocketSubsystemEOS: %s"), *Error);
				}
			}
			else
			{
				Error = TEXT("EOSKitSubsystem not available or not initialized");
				UE_LOG(LogNet, Error, TEXT("FSocketSubsystemEOS: %s"), *Error);
			}
		}
		else
		{
			Error = TEXT("Game Instance not available");
			UE_LOG(LogNet, Error, TEXT("FSocketSubsystemEOS: %s"), *Error);
		}
	}
	else
	{
		Error = TEXT("GEngine not available");
		UE_LOG(LogNet, Error, TEXT("FSocketSubsystemEOS: %s"), *Error);
	}

	return false;
}

void FSocketSubsystemEOS::Shutdown()
{
	UE_LOG(LogNet, Log, TEXT("FSocketSubsystemEOS: Shutting down"));
	P2PHandle = nullptr;
	LocalProductUserId = nullptr;
}

FSocket* FSocketSubsystemEOS::CreateSocket(const FName& SocketType, const FString& SocketDescription, const FName& ProtocolType)
{
	if (!P2PHandle || !LocalProductUserId)
	{
		UE_LOG(LogNet, Error, TEXT("FSocketSubsystemEOS::CreateSocket: P2P not initialized"));
		return nullptr;
	}

	FSocket* NewSocket = new FSocketEOS(SocketDescription, P2PHandle, LocalProductUserId);
	UE_LOG(LogNet, Log, TEXT("FSocketSubsystemEOS: Created socket %s"), *SocketDescription);
	return NewSocket;
}

void FSocketSubsystemEOS::DestroySocket(FSocket* Socket)
{
	if (Socket)
	{
		delete Socket;
	}
}

FAddressInfoResult FSocketSubsystemEOS::GetAddressInfo(const TCHAR* HostName, const TCHAR* ServiceName,
	EAddressInfoFlags QueryFlags, const FName ProtocolTypeName, ESocketType SocketType)
{
	// Create an EOS address from the hostname (which should be a ProductUserId)
	FAddressInfoResult Result(HostName, ServiceName);
	
	TSharedRef<FInternetAddrEOS> EOSAddr = MakeShared<FInternetAddrEOS>();
	EOSAddr->SetProductUserId(HostName);
	
	if (EOSAddr->IsValid())
	{
		Result.Results.Add(FAddressInfoResultData(EOSAddr, 0, ProtocolTypeName, SocketType));
		Result.ReturnCode = SE_NO_ERROR;
	}
	else
	{
		Result.ReturnCode = SE_HOST_NOT_FOUND;
	}

	return Result;
}

TSharedPtr<FInternetAddr> FSocketSubsystemEOS::GetAddressFromString(const FString& InAddress)
{
	TSharedRef<FInternetAddrEOS> EOSAddr = MakeShared<FInternetAddrEOS>();
	EOSAddr->SetProductUserId(InAddress);
	return EOSAddr;
}

bool FSocketSubsystemEOS::GetHostName(FString& HostName)
{
	if (LocalProductUserId)
	{
		char ProductUserIdStr[EOS_PRODUCTUSERID_MAX_LENGTH + 1];
		int32_t ProductUserIdStrSize = sizeof(ProductUserIdStr);
		if (EOS_ProductUserId_ToString(LocalProductUserId, ProductUserIdStr, &ProductUserIdStrSize) == EOS_EResult::EOS_Success)
		{
			HostName = UTF8_TO_TCHAR(ProductUserIdStr);
			return true;
		}
	}
	return false;
}

TSharedRef<FInternetAddr> FSocketSubsystemEOS::CreateInternetAddr()
{
	return MakeShared<FInternetAddrEOS>();
}

bool FSocketSubsystemEOS::HasNetworkDevice()
{
	// EOS P2P is always available if we have a valid P2P handle
	return P2PHandle != nullptr && LocalProductUserId != nullptr;
}

const TCHAR* FSocketSubsystemEOS::GetSocketAPIName() const
{
	return TEXT("EOS");
}

ESocketErrors FSocketSubsystemEOS::GetLastErrorCode()
{
	return LastSocketError;
}

ESocketErrors FSocketSubsystemEOS::TranslateErrorCode(int32 Code)
{
	// Map EOS error codes to socket errors
	EOS_EResult EOSResult = static_cast<EOS_EResult>(Code);

	switch (EOSResult)
	{
	case EOS_EResult::EOS_Success:
		return SE_NO_ERROR;
	case EOS_EResult::EOS_NotFound:
		return SE_HOST_NOT_FOUND;
	case EOS_EResult::EOS_NoConnection:
		return SE_NO_RECOVERY;
	case EOS_EResult::EOS_InvalidParameters:
		return SE_EINVAL;
	case EOS_EResult::EOS_TimedOut:
		return SE_ETIMEDOUT;
	default:
		return SE_NO_ERROR;
	}
}

bool FSocketSubsystemEOS::GetLocalAdapterAddresses(TArray<TSharedPtr<FInternetAddr>>& OutAddresses)
{
	if (LocalProductUserId)
	{
		TSharedRef<FInternetAddrEOS> LocalAddr = MakeShared<FInternetAddrEOS>();
		char ProductUserIdStr[EOS_PRODUCTUSERID_MAX_LENGTH + 1];
		int32_t ProductUserIdStrSize = sizeof(ProductUserIdStr);
		if (EOS_ProductUserId_ToString(LocalProductUserId, ProductUserIdStr, &ProductUserIdStrSize) == EOS_EResult::EOS_Success)
		{
			LocalAddr->SetProductUserId(UTF8_TO_TCHAR(ProductUserIdStr));
			OutAddresses.Add(LocalAddr);
			return true;
		}
	}
	return false;
}

TArray<TSharedRef<FInternetAddr>> FSocketSubsystemEOS::GetLocalBindAddresses()
{
	TArray<TSharedRef<FInternetAddr>> Results;
	
	if (LocalProductUserId)
	{
		TSharedRef<FInternetAddrEOS> LocalAddr = MakeShared<FInternetAddrEOS>();
		char ProductUserIdStr[EOS_PRODUCTUSERID_MAX_LENGTH + 1];
		int32_t ProductUserIdStrSize = sizeof(ProductUserIdStr);
		if (EOS_ProductUserId_ToString(LocalProductUserId, ProductUserIdStr, &ProductUserIdStrSize) == EOS_EResult::EOS_Success)
		{
			LocalAddr->SetProductUserId(UTF8_TO_TCHAR(ProductUserIdStr));
			Results.Add(LocalAddr);
		}
	}
	
	return Results;
}

bool FSocketSubsystemEOS::RequiresChatDataBeSeparate()
{
	// EOS P2P does not require chat data to be separate
	return false;
}

bool FSocketSubsystemEOS::RequiresEncryptedPackets()
{
	// EOS P2P handles encryption internally
	return false;
}

bool FSocketSubsystemEOS::IsSocketWaitSupported() const
{
	// Socket wait is supported
	return true;
}
