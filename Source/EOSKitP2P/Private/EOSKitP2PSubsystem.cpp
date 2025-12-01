// Copyright (C) 2024, All Rights Reserved.

#include "EOSKitP2PSubsystem.h"
#include "EOSKitSubsystem.h"
#include "Kismet/GameplayStatics.h"
#if WITH_EOS_SDK
#include "Windows/AllowWindowsPlatformTypes.h"
#include "eos_sdk.h"
#include "eos_p2p.h"
#include "eos_p2p_types.h"
#include "Windows/HideWindowsPlatformTypes.h"
#endif
#include "EOSKitSharedTypes.h"
#include "Async/Async.h"

void UEOSKitP2PSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	NextNotificationId = 1;
	UE_LOG(LogTemp, Log, TEXT("EOSKitP2PSubsystem: Initialized"));
}

void UEOSKitP2PSubsystem::Deinitialize()
{
	// Remove all notifications
	for (const auto& Pair : NotificationIdMap)
	{
		EOS_HP2P P2PHandle = GetP2PHandle();
		if (P2PHandle)
		{
			EOS_P2P_RemoveNotifyPeerConnectionRequest(P2PHandle, Pair.Value);
		}
	}
	NotificationIdMap.Empty();

	Super::Deinitialize();
}

UEOSKitSubsystem* UEOSKitP2PSubsystem::GetEOSKitSubsystem() const
{
	if (UGameInstance* GameInstance = GetGameInstance())
	{
		return GameInstance->GetSubsystem<UEOSKitSubsystem>();
	}
	return nullptr;
}

EOS_HP2P UEOSKitP2PSubsystem::GetP2PHandle() const
{
	UEOSKitSubsystem* EOSKitSubsystem = GetEOSKitSubsystem();
	if (!EOSKitSubsystem || !EOSKitSubsystem->GetPlatformHandle())
	{
		return nullptr;
	}
	return EOS_Platform_GetP2PInterface(EOSKitSubsystem->GetPlatformHandle());
}

EEOSResult UEOSKitP2PSubsystem::QueryNATType()
{
	EOS_HP2P P2PHandle = GetP2PHandle();
	if (!P2PHandle)
	{
		UE_LOG(LogTemp, Error, TEXT("EOSKitP2P: Failed to get P2P Handle"));
		return EEOSResult::EOS_NotConfigured;
	}

	EOS_P2P_QueryNATTypeOptions Options = {};
	Options.ApiVersion = EOS_P2P_QUERYNATTYPE_API_LATEST;

	// Note: This is a synchronous wrapper, but QueryNATType is async
	// The actual async operation should be done through async nodes
	return EEOSResult::EOS_Success;
}

bool UEOSKitP2PSubsystem::GetNATType(EEOSKitNATType& OutNATType)
{
	EOS_HP2P P2PHandle = GetP2PHandle();
	if (!P2PHandle)
	{
		UE_LOG(LogTemp, Error, TEXT("EOSKitP2P: Failed to get P2P Handle"));
		return false;
	}

	EOS_P2P_GetNATTypeOptions Options = {};
	Options.ApiVersion = EOS_P2P_GETNATTYPE_API_LATEST;

	EOS_ENATType NATType = EOS_ENATType::EOS_NAT_Unknown;
	EOS_EResult Result = EOS_P2P_GetNATType(P2PHandle, &Options, &NATType);
	if (Result == EOS_EResult::EOS_Success)
	{
		OutNATType = static_cast<EEOSKitNATType>(NATType);
		return true;
	}
	return false;
}

int32 UEOSKitP2PSubsystem::AddNotifyPeerConnectionRequest(const FEOSKitProductUserId& LocalUserId, const FEOSKitP2PSocketId& SocketId, const FOnEOSPeerConnectionRequestDelegate& Callback)
{
	EOS_HP2P P2PHandle = GetP2PHandle();
	if (!P2PHandle)
	{
		UE_LOG(LogTemp, Error, TEXT("EOSKitP2P: Failed to get P2P Handle"));
		return 0;
	}

	EOS_P2P_AddNotifyPeerConnectionRequestOptions Options = {};
	Options.ApiVersion = EOS_P2P_ADDNOTIFYPEERCONNECTIONREQUEST_API_LATEST;
	Options.LocalUserId = LocalUserId.GetValueAsEosType();

	EOS_P2P_SocketId SocketIdData = SocketId.GetAsEosData();
	Options.SocketId = SocketId.SocketName.IsEmpty() ? nullptr : &SocketIdData;
	
	// In SDK 1.18.1, ClientData and Callback are separate parameters, not in Options
	EOS_NotificationId NotificationId = ::EOS_P2P_AddNotifyPeerConnectionRequest(
		P2PHandle,
		&Options,
		this,  // ClientData
		&UEOSKitP2PSubsystem::OnPeerConnectionRequestCallback);  // Callback

	if (NotificationId != EOS_INVALID_NOTIFICATIONID)
	{
		int32 OurNotificationId = NextNotificationId++;
		NotificationIdMap.Add(OurNotificationId, NotificationId);
		
		// Store the callback delegate with EOS notification ID as key
		PeerConnectionRequestCallbacks.Add(NotificationId, Callback);
		
		return OurNotificationId;
	}

	return 0;
}

void UEOSKitP2PSubsystem::RemoveNotifyPeerConnectionRequest(int32 NotificationId)
{
	EOS_NotificationId* EOSNotificationId = NotificationIdMap.Find(NotificationId);
	if (EOSNotificationId)
	{
		EOS_HP2P P2PHandle = GetP2PHandle();
		if (P2PHandle)
		{
			EOS_P2P_RemoveNotifyPeerConnectionRequest(P2PHandle, *EOSNotificationId);
		}
		PeerConnectionRequestCallbacks.Remove(*EOSNotificationId);
		NotificationIdMap.Remove(NotificationId);
	}
}

EEOSResult UEOSKitP2PSubsystem::AcceptConnection(const FEOSKitProductUserId& LocalUserId, const FEOSKitProductUserId& RemoteUserId, const FEOSKitP2PSocketId& SocketId)
{
	EOS_HP2P P2PHandle = GetP2PHandle();
	if (!P2PHandle)
	{
		UE_LOG(LogTemp, Error, TEXT("EOSKitP2P: Failed to get P2P Handle"));
		return EEOSResult::EOS_NotConfigured;
	}

	EOS_P2P_AcceptConnectionOptions Options = {};
	Options.ApiVersion = EOS_P2P_ACCEPTCONNECTION_API_LATEST;
	Options.LocalUserId = LocalUserId.GetValueAsEosType();
	Options.RemoteUserId = RemoteUserId.GetValueAsEosType();
	
	EOS_P2P_SocketId SocketIdData = SocketId.GetAsEosData();
	Options.SocketId = &SocketIdData;

	EOS_EResult Result = EOS_P2P_AcceptConnection(P2PHandle, &Options);
	return ConvertEOSResultToEEOSResult(Result);
}

EEOSResult UEOSKitP2PSubsystem::CloseConnection(const FEOSKitProductUserId& LocalUserId, const FEOSKitProductUserId& RemoteUserId, const FEOSKitP2PSocketId& SocketId)
{
	EOS_HP2P P2PHandle = GetP2PHandle();
	if (!P2PHandle)
	{
		UE_LOG(LogTemp, Error, TEXT("EOSKitP2P: Failed to get P2P Handle"));
		return EEOSResult::EOS_NotConfigured;
	}

	EOS_P2P_CloseConnectionOptions Options = {};
	Options.ApiVersion = EOS_P2P_CLOSECONNECTION_API_LATEST;
	Options.LocalUserId = LocalUserId.GetValueAsEosType();
	Options.RemoteUserId = RemoteUserId.GetValueAsEosType();
	
	if (SocketId.SocketName.IsEmpty())
	{
		Options.SocketId = nullptr;
	}
	else
	{
		EOS_P2P_SocketId SocketIdData = SocketId.GetAsEosData();
		Options.SocketId = &SocketIdData;
	}

	EOS_EResult Result = EOS_P2P_CloseConnection(P2PHandle, &Options);
	return ConvertEOSResultToEEOSResult(Result);
}

EEOSResult UEOSKitP2PSubsystem::SendPacket(const FEOSKitProductUserId& LocalUserId, 
	const FEOSKitProductUserId& RemoteUserId, 
	const FEOSKitP2PSocketId& SocketId, 
	int32 Channel, 
	const TArray<uint8>& Data, 
	bool bAllowDelayedDelivery, 
	bool bDisableAutoAcceptConnection, 
	EEOSKitPacketReliability Reliability)
{
	EOS_HP2P P2PHandle = GetP2PHandle();
	if (!P2PHandle)
	{
		UE_LOG(LogTemp, Error, TEXT("EOSKitP2P: Failed to get P2P Handle"));
		return EEOSResult::EOS_NotConfigured;
	}

	if (Data.Num() == 0)
	{
		UE_LOG(LogTemp, Error, TEXT("EOSKitP2P: Data array is empty"));
		return EEOSResult::EOS_InvalidParameters;
	}

	if (Channel < 0 || Channel > 255)
	{
		UE_LOG(LogTemp, Error, TEXT("EOSKitP2P: Channel must be between 0 and 255"));
		return EEOSResult::EOS_InvalidParameters;
	}

	EOS_P2P_SendPacketOptions Options = {};
	Options.ApiVersion = EOS_P2P_SENDPACKET_API_LATEST;
	Options.LocalUserId = LocalUserId.GetValueAsEosType();
	Options.RemoteUserId = RemoteUserId.GetValueAsEosType();
	
	EOS_P2P_SocketId SocketIdData = SocketId.GetAsEosData();
	Options.SocketId = &SocketIdData;
	
	Options.Channel = static_cast<uint8_t>(Channel);
	Options.DataLengthBytes = Data.Num();
	Options.Data = Data.GetData();
	Options.bAllowDelayedDelivery = bAllowDelayedDelivery ? EOS_TRUE : EOS_FALSE;
	Options.bDisableAutoAcceptConnection = bDisableAutoAcceptConnection ? EOS_TRUE : EOS_FALSE;
	Options.Reliability = static_cast<EOS_EPacketReliability>(Reliability);

	EOS_EResult Result = EOS_P2P_SendPacket(P2PHandle, &Options);
	return ConvertEOSResultToEEOSResult(Result);
}

bool UEOSKitP2PSubsystem::GetNextReceivedPacketSize(const FEOSKitProductUserId& LocalUserId, int32 RequestedChannel, int32& OutPacketSizeBytes)
{
	EOS_HP2P P2PHandle = GetP2PHandle();
	if (!P2PHandle)
	{
		UE_LOG(LogTemp, Error, TEXT("EOSKitP2P: Failed to get P2P Handle"));
		return false;
	}

	EOS_P2P_GetNextReceivedPacketSizeOptions Options = {};
	Options.ApiVersion = EOS_P2P_GETNEXTRECEIVEDPACKETSIZE_API_LATEST;
	Options.LocalUserId = LocalUserId.GetValueAsEosType();

	// RequestedChannel: -1 means any channel, otherwise use the specified channel
	uint8_t ChannelValue = 0;
	if (RequestedChannel >= 0 && RequestedChannel <= 255)
	{
		ChannelValue = static_cast<uint8_t>(RequestedChannel);
		Options.RequestedChannel = &ChannelValue;
	}
	else
	{
		Options.RequestedChannel = nullptr; // Any channel
	}

	uint32_t PacketSizeBytes = 0;
	EOS_EResult Result = EOS_P2P_GetNextReceivedPacketSize(P2PHandle, &Options, &PacketSizeBytes);

	if (Result == EOS_EResult::EOS_Success)
	{
		OutPacketSizeBytes = static_cast<int32>(PacketSizeBytes);
		return true;
	}

	return false;
}

bool UEOSKitP2PSubsystem::ReceivePacket(const FEOSKitProductUserId& LocalUserId, 
	int32 MaxDataSizeBytes, 
	int32 RequestedChannel, 
	FEOSKitProductUserId& OutPeerId, 
	FEOSKitP2PSocketId& OutSocketId, 
	int32& OutChannel, 
	TArray<uint8>& OutData, 
	int32& OutBytesWritten)
{
	EOS_HP2P P2PHandle = GetP2PHandle();
	if (!P2PHandle)
	{
		UE_LOG(LogTemp, Error, TEXT("EOSKitP2P: Failed to get P2P Handle"));
		return false;
	}

	EOS_P2P_ReceivePacketOptions Options = {};
	Options.ApiVersion = EOS_P2P_RECEIVEPACKET_API_LATEST;
	Options.LocalUserId = LocalUserId.GetValueAsEosType();
	Options.MaxDataSizeBytes = MaxDataSizeBytes;

	// RequestedChannel: -1 means any channel, otherwise use the specified channel
	uint8_t ChannelValue = 0;
	if (RequestedChannel >= 0 && RequestedChannel <= 255)
	{
		ChannelValue = static_cast<uint8_t>(RequestedChannel);
		Options.RequestedChannel = &ChannelValue;
	}
	else
	{
		Options.RequestedChannel = nullptr; // Any channel
	}

	EOS_ProductUserId PeerId = nullptr;
	EOS_P2P_SocketId SocketId = {};
	uint8_t Channel = 0;
	TArray<uint8> DataBuffer;
	DataBuffer.SetNumUninitialized(MaxDataSizeBytes);
	uint32_t BytesWritten = 0;

	EOS_EResult Result = EOS_P2P_ReceivePacket(P2PHandle, &Options, &PeerId, &SocketId, &Channel, DataBuffer.GetData(), &BytesWritten);

	if (Result == EOS_EResult::EOS_Success)
	{
		OutPeerId = FEOSKitProductUserId(PeerId);
		
		OutSocketId.SocketName = UTF8_TO_TCHAR(SocketId.SocketName);
		
		OutChannel = static_cast<int32>(Channel);
		
		OutData.SetNumUninitialized(BytesWritten);
		FMemory::Memcpy(OutData.GetData(), DataBuffer.GetData(), BytesWritten);
		
		OutBytesWritten = static_cast<int32>(BytesWritten);
		return true;
	}

	return false;
}

int32 UEOSKitP2PSubsystem::GetRelayControlCount()
{
	// This function doesn't exist in the EOS SDK
	// Relay control is a single setting, not a count
	// Returning 1 to indicate there is one relay control setting
	return 1;
}

EEOSResult UEOSKitP2PSubsystem::SetRelayControl(int32 RelayControl)
{
	EOS_HP2P P2PHandle = GetP2PHandle();
	if (!P2PHandle)
	{
		UE_LOG(LogTemp, Error, TEXT("EOSKitP2P: Failed to get P2P Handle"));
		return EEOSResult::EOS_NotConfigured;
	}

	if (RelayControl < 0 || RelayControl > 2)
	{
		UE_LOG(LogTemp, Error, TEXT("EOSKitP2P: RelayControl must be 0 (NoRelays), 1 (AllowRelays), or 2 (ForceRelays)"));
		return EEOSResult::EOS_InvalidParameters;
	}

	EOS_P2P_SetRelayControlOptions Options = {};
	Options.ApiVersion = EOS_P2P_SETRELAYCONTROL_API_LATEST;
	Options.RelayControl = static_cast<EOS_ERelayControl>(RelayControl);

	EOS_EResult Result = EOS_P2P_SetRelayControl(P2PHandle, &Options);
	return ConvertEOSResultToEEOSResult(Result);
}

bool UEOSKitP2PSubsystem::GetRelayControl(int32& OutRelayControl)
{
	EOS_HP2P P2PHandle = GetP2PHandle();
	if (!P2PHandle)
	{
		UE_LOG(LogTemp, Error, TEXT("EOSKitP2P: Failed to get P2P Handle"));
		return false;
	}

	EOS_P2P_GetRelayControlOptions Options = {};
	Options.ApiVersion = EOS_P2P_GETRELAYCONTROL_API_LATEST;

	EOS_ERelayControl RelayControl = EOS_ERelayControl::EOS_RC_NoRelays;
	EOS_EResult Result = EOS_P2P_GetRelayControl(P2PHandle, &Options, &RelayControl);
	if (Result == EOS_EResult::EOS_Success)
	{
		OutRelayControl = static_cast<int32>(RelayControl);
		return true;
	}
	return false;
}

void EOS_CALL UEOSKitP2PSubsystem::OnPeerConnectionRequestCallback(const EOS_P2P_OnIncomingConnectionRequestInfo* Data)
{
	if (!Data || !Data->ClientData)
	{
		return;
	}

	UEOSKitP2PSubsystem* Self = static_cast<UEOSKitP2PSubsystem*>(Data->ClientData);

	// Convert Product User IDs
	FEOSKitProductUserId RemoteUserId(Data->RemoteUserId);
	FEOSKitProductUserId LocalUserId(Data->LocalUserId);

	// Convert Socket ID
	FEOSKitP2PSocketId SocketId;
	if (Data->SocketId)
	{
		SocketId.SocketName = UTF8_TO_TCHAR(Data->SocketId->SocketName);
	}

	// Find and call the stored callback delegate
	// Note: EOS_P2P_OnIncomingConnectionRequestInfo does not have NotificationId
	// We need to call all registered callbacks since we can't identify which specific one to call
	for (auto& Pair : Self->PeerConnectionRequestCallbacks)
	{
		FOnEOSPeerConnectionRequestDelegate CallbackCopy = Pair.Value;
		AsyncTask(ENamedThreads::GameThread, [CallbackCopy, RemoteUserId, SocketId, LocalUserId]()
		{
			CallbackCopy.ExecuteIfBound(RemoteUserId, SocketId, LocalUserId);
		});
	}

	// Broadcast to multicast delegate
	AsyncTask(ENamedThreads::GameThread, [Self, RemoteUserId, SocketId, LocalUserId]()
	{
		Self->OnPeerConnectionRequest.Broadcast(RemoteUserId, SocketId, LocalUserId);
	});
}

