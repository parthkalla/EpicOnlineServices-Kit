// Copyright (C) 2024, All Rights Reserved.

#include "SocketEOS.h"
#include "Async/Async.h"

FSocketEOS::FSocketEOS(const FString& InSocketDescription, EOS_HP2P InP2PHandle, EOS_ProductUserId InLocalUserId)
	: FSocket(SOCKTYPE_Datagram, InSocketDescription, FName(TEXT("EOS")))
	, P2PHandle(InP2PHandle)
	, LocalUserId(InLocalUserId)
	, RemoteUserId(nullptr)
	, SocketDescription(InSocketDescription)
	, bIsNonBlocking(true)
	, ConnectionState(SCS_NotConnected)
	, ReliabilityMode(EOS_EPacketReliability::EOS_PR_UnreliableUnordered)
{
}

FSocketEOS::~FSocketEOS()
{
	Close();
}

bool FSocketEOS::Shutdown(ESocketShutdownMode Mode)
{
	// For EOS P2P, shutdown is the same as close
	return Close();
}

bool FSocketEOS::Close()
{
	if (P2PHandle && RemoteUserId)
	{
		EOS_P2P_CloseConnectionOptions CloseOptions = {};
		CloseOptions.ApiVersion = EOS_P2P_CLOSECONNECTION_API_LATEST;
		CloseOptions.LocalUserId = LocalUserId;
		CloseOptions.RemoteUserId = RemoteUserId;
		CloseOptions.SocketId = nullptr; // Use default socket ID

		EOS_P2P_CloseConnection(P2PHandle, &CloseOptions);
	}

	ConnectionState = SCS_NotConnected;
	return true;
}

bool FSocketEOS::Bind(const FInternetAddr& Addr)
{
	// EOS P2P doesn't require traditional binding
	// The ProductUserId acts as the address
	return true;
}

bool FSocketEOS::Connect(const FInternetAddr& Addr)
{
	const FInternetAddrEOS& EOSAddr = static_cast<const FInternetAddrEOS&>(Addr);
	RemoteUserId = EOSAddr.GetProductUserId();

	if (RemoteUserId)
	{
		ConnectionState = SCS_Connected;
		return true;
	}

	return false;
}

bool FSocketEOS::Listen(int32 MaxBacklog)
{
	// EOS P2P handles listening automatically
	ConnectionState = SCS_Connected;
	return true;
}

bool FSocketEOS::WaitForPendingConnection(bool& bHasPendingConnection, const FTimespan& WaitTime)
{
	// Check if there are any pending incoming connections
	uint32 PendingDataSize = 0;
	bHasPendingConnection = HasPendingData(PendingDataSize);
	return true;
}

bool FSocketEOS::HasPendingData(uint32& PendingDataSize)
{
	if (!P2PHandle || !LocalUserId)
	{
		return false;
	}

	EOS_P2P_GetNextReceivedPacketSizeOptions SizeOptions = {};
	SizeOptions.ApiVersion = EOS_P2P_GETNEXTRECEIVEDPACKETSIZE_API_LATEST;
	SizeOptions.LocalUserId = LocalUserId;
	SizeOptions.RequestedChannel = nullptr; // All channels

	EOS_EResult Result = EOS_P2P_GetNextReceivedPacketSize(P2PHandle, &SizeOptions, &PendingDataSize);
	return Result == EOS_EResult::EOS_Success && PendingDataSize > 0;
}

FSocket* FSocketEOS::Accept(const FString& InSocketDescription)
{
	FInternetAddrEOS DummyAddr;
	return Accept(DummyAddr, InSocketDescription);
}

FSocket* FSocketEOS::Accept(FInternetAddr& OutAddr, const FString& InSocketDescription)
{
	// Create a new socket for the accepted connection
	FSocketEOS* NewSocket = new FSocketEOS(InSocketDescription, P2PHandle, LocalUserId);
	NewSocket->ConnectionState = SCS_Connected;
	return NewSocket;
}

bool FSocketEOS::SendTo(const uint8* Data, int32 Count, int32& BytesSent, const FInternetAddr& Destination)
{
	if (!P2PHandle || !LocalUserId)
	{
		BytesSent = 0;
		return false;
	}

	const FInternetAddrEOS& EOSAddr = static_cast<const FInternetAddrEOS&>(Destination);
	EOS_ProductUserId TargetUserId = EOSAddr.GetProductUserId();

	if (!TargetUserId)
	{
		BytesSent = 0;
		return false;
	}

	EOS_P2P_SendPacketOptions SendOptions = {};
	SendOptions.ApiVersion = EOS_P2P_SENDPACKET_API_LATEST;
	SendOptions.LocalUserId = LocalUserId;
	SendOptions.RemoteUserId = TargetUserId;
	SendOptions.Channel = P2P_CHANNEL;
	SendOptions.DataLengthBytes = Count;
	SendOptions.Data = Data;
	SendOptions.Reliability = ReliabilityMode;
	SendOptions.bAllowDelayedDelivery = EOS_TRUE;

	EOS_EResult Result = EOS_P2P_SendPacket(P2PHandle, &SendOptions);

	if (Result == EOS_EResult::EOS_Success)
	{
		BytesSent = Count;
		return true;
	}

	BytesSent = 0;
	return false;
}

bool FSocketEOS::Send(const uint8* Data, int32 Count, int32& BytesSent)
{
	if (!RemoteUserId)
	{
		BytesSent = 0;
		return false;
	}

	FInternetAddrEOS RemoteAddr;
	RemoteAddr.SetProductUserId(TEXT(""));  // RemoteUserId is already set
	return SendTo(Data, Count, BytesSent, RemoteAddr);
}

bool FSocketEOS::RecvFrom(uint8* Data, int32 BufferSize, int32& BytesRead, FInternetAddr& Source, ESocketReceiveFlags::Type Flags)
{
	if (!P2PHandle || !LocalUserId)
	{
		BytesRead = 0;
		return false;
	}

	EOS_P2P_ReceivePacketOptions ReceiveOptions = {};
	ReceiveOptions.ApiVersion = EOS_P2P_RECEIVEPACKET_API_LATEST;
	ReceiveOptions.LocalUserId = LocalUserId;
	ReceiveOptions.MaxDataSizeBytes = BufferSize;
	ReceiveOptions.RequestedChannel = nullptr; // All channels

	// EOS_P2P_ReceivePacket signature: EOS_EResult EOS_P2P_ReceivePacket(EOS_HP2P Handle, const EOS_P2P_ReceivePacketOptions* Options, EOS_ProductUserId* OutPeerId, EOS_P2P_SocketId* OutSocketId, uint8_t* OutChannel, void* OutData, uint32_t* OutBytesWritten)
	EOS_ProductUserId OutPeerId;
	EOS_P2P_SocketId OutSocketId;
	uint8_t OutChannel;
	uint32_t OutBytesWritten;

	EOS_EResult Result = EOS_P2P_ReceivePacket(P2PHandle, &ReceiveOptions, &OutPeerId, &OutSocketId, &OutChannel, Data, &OutBytesWritten);

	if (Result == EOS_EResult::EOS_Success)
	{
		BytesRead = OutBytesWritten;

		// Set the source address
		char RemoteUserIdStr[EOS_PRODUCTUSERID_MAX_LENGTH + 1];
		int32_t RemoteUserIdStrSize = sizeof(RemoteUserIdStr);
		if (EOS_ProductUserId_ToString(OutPeerId, RemoteUserIdStr, &RemoteUserIdStrSize) == EOS_EResult::EOS_Success)
		{
			FInternetAddrEOS& EOSSource = static_cast<FInternetAddrEOS&>(Source);
			EOSSource.SetProductUserId(UTF8_TO_TCHAR(RemoteUserIdStr));
		}

		return true;
	}

	BytesRead = 0;
	return false;
}

bool FSocketEOS::Recv(uint8* Data, int32 BufferSize, int32& BytesRead, ESocketReceiveFlags::Type Flags)
{
	FInternetAddrEOS DummyAddr;
	return RecvFrom(Data, BufferSize, BytesRead, DummyAddr, Flags);
}

bool FSocketEOS::Wait(ESocketWaitConditions::Type Condition, FTimespan WaitTime)
{
	if (Condition == ESocketWaitConditions::WaitForRead)
	{
		uint32 PendingDataSize;
		return HasPendingData(PendingDataSize);
	}

	// Always ready to write for EOS P2P
	return true;
}

ESocketConnectionState FSocketEOS::GetConnectionState()
{
	return ConnectionState;
}

void FSocketEOS::GetAddress(FInternetAddr& OutAddr)
{
	FInternetAddrEOS& EOSAddr = static_cast<FInternetAddrEOS&>(OutAddr);
	if (LocalUserId)
	{
		char LocalUserIdStr[EOS_PRODUCTUSERID_MAX_LENGTH + 1];
		int32_t LocalUserIdStrSize = sizeof(LocalUserIdStr);
		if (EOS_ProductUserId_ToString(LocalUserId, LocalUserIdStr, &LocalUserIdStrSize) == EOS_EResult::EOS_Success)
		{
			EOSAddr.SetProductUserId(UTF8_TO_TCHAR(LocalUserIdStr));
		}
	}
}

bool FSocketEOS::GetPeerAddress(FInternetAddr& OutAddr)
{
	FInternetAddrEOS& EOSAddr = static_cast<FInternetAddrEOS&>(OutAddr);
	if (RemoteUserId)
	{
		char RemoteUserIdStr[EOS_PRODUCTUSERID_MAX_LENGTH + 1];
		int32_t RemoteUserIdStrSize = sizeof(RemoteUserIdStr);
		if (EOS_ProductUserId_ToString(RemoteUserId, RemoteUserIdStr, &RemoteUserIdStrSize) == EOS_EResult::EOS_Success)
		{
			EOSAddr.SetProductUserId(UTF8_TO_TCHAR(RemoteUserIdStr));
		}
		return true;
	}
	return false;
}

bool FSocketEOS::SetNonBlocking(bool bInIsNonBlocking)
{
	bIsNonBlocking = bInIsNonBlocking;
	return true;
}

bool FSocketEOS::SetBroadcast(bool bAllowBroadcast)
{
	// Not supported for EOS P2P
	return false;
}

bool FSocketEOS::SetNoDelay(bool bIsNoDelay)
{
	// Set reliability mode based on NoDelay
	ReliabilityMode = bIsNoDelay 
		? EOS_EPacketReliability::EOS_PR_UnreliableUnordered 
		: EOS_EPacketReliability::EOS_PR_ReliableOrdered;
	return true;
}

bool FSocketEOS::JoinMulticastGroup(const FInternetAddr& GroupAddress)
{
	// Not supported for EOS P2P
	return false;
}

bool FSocketEOS::JoinMulticastGroup(const FInternetAddr& GroupAddress, const FInternetAddr& InterfaceAddress)
{
	// Not supported for EOS P2P
	return false;
}

bool FSocketEOS::LeaveMulticastGroup(const FInternetAddr& GroupAddress)
{
	// Not supported for EOS P2P
	return false;
}

bool FSocketEOS::LeaveMulticastGroup(const FInternetAddr& GroupAddress, const FInternetAddr& InterfaceAddress)
{
	// Not supported for EOS P2P
	return false;
}

bool FSocketEOS::SetMulticastLoopback(bool bLoopback)
{
	// Not supported for EOS P2P
	return false;
}

bool FSocketEOS::SetMulticastTtl(uint8 TimeToLive)
{
	// Not supported for EOS P2P
	return false;
}

bool FSocketEOS::SetMulticastInterface(const FInternetAddr& InterfaceAddress)
{
	// Not supported for EOS P2P
	return false;
}

bool FSocketEOS::SetReuseAddr(bool bAllowReuse)
{
	// Not applicable for EOS P2P
	return true;
}

bool FSocketEOS::SetLinger(bool bShouldLinger, int32 Timeout)
{
	// Not applicable for EOS P2P
	return true;
}

bool FSocketEOS::SetRecvErr(bool bUseErrorQueue)
{
	// Not supported for EOS P2P
	return false;
}

bool FSocketEOS::SetSendBufferSize(int32 Size, int32& NewSize)
{
	// EOS P2P manages its own buffers
	NewSize = Size;
	return true;
}

bool FSocketEOS::SetReceiveBufferSize(int32 Size, int32& NewSize)
{
	// EOS P2P manages its own buffers
	NewSize = Size;
	return true;
}

int32 FSocketEOS::GetPortNo()
{
	// Not applicable for EOS P2P
	return 0;
}

void FSocketEOS::SetRemoteAddr(const FInternetAddrEOS& InRemoteAddr)
{
	RemoteUserId = InRemoteAddr.GetProductUserId();
	if (RemoteUserId)
	{
		ConnectionState = SCS_Connected;
	}
}
