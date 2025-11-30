// Copyright (C) 2024, All Rights Reserved.

#include "EOSKitPingClient.h"
#include "OnlineSubsystemUtils.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "Misc/ConfigCacheIni.h"

DEFINE_LOG_CATEGORY(LogEOSKitPingClient);

AEOSKitPingClient::AEOSKitPingClient(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
	, PingBeginTime(0)
	, PingMS(0)
{
}

void AEOSKitPingClient::OnFailure()
{
	Super::OnFailure();
	UE_LOG(LogEOSKitPingClient, Log, TEXT("EOSKitPingClient connection failed"));
	OnPingComplete.ExecuteIfBound(0, false);
	DestroyBeacon();
}

void AEOSKitPingClient::ClientPingBegin_Implementation()
{
	UE_LOG(LogEOSKitPingClient, Log, TEXT("EOSKitPingClient::ClientPingBegin_Implementation"));
	PingBeginTime = FDateTime::Now().GetTicks();
	ServerPong();
}

bool AEOSKitPingClient::ServerPong_Validate()
{
	return true;
}

void AEOSKitPingClient::ServerPong_Implementation()
{
	UE_LOG(LogEOSKitPingClient, Log, TEXT("EOSKitPingClient::ServerPong_Implementation"));
	ClientPingEnd();
}

void AEOSKitPingClient::ClientPingEnd_Implementation()
{
	UE_LOG(LogEOSKitPingClient, Log, TEXT("EOSKitPingClient::ClientPingEnd_Implementation"));
	int64 PingTime = FDateTime::Now().GetTicks() - PingBeginTime;
	PingMS = static_cast<int32>(PingTime / 10000);
	UE_LOG(LogEOSKitPingClient, Log, TEXT("Ping Complete: %d ms"), PingMS);
	OnPingComplete.ExecuteIfBound(PingMS, true);
	DestroyBeacon();
}

bool AEOSKitPingClient::ConnectToHost(FString Address, int32 Port, bool bPortOverride, FEOSKitPingComplete Ref)
{
	OnPingComplete = Ref;
	FURL url(nullptr, *Address, ETravelType::TRAVEL_Absolute);
	
	if (bPortOverride)
	{
		url.Port = Port;
	}
	else
	{
		int32 ListenPort;
		if (GConfig->GetInt(TEXT("/Script/OnlineSubsystemUtils.OnlineBeaconHost"), TEXT("ListenPort"), ListenPort, GEngineIni))
		{
			url.Port = ListenPort;
		}
		else
		{
			url.Port = 8888;
		}
	}
	
	UE_LOG(LogEOSKitPingClient, Log, TEXT("Connecting to %s:%d"), *url.Host, url.Port);
	return InitClient(url);
}

bool AEOSKitPingClient::ConnectToSession(const FBlueprintSessionResult& SessionResult, FEOSKitPingComplete Ref)
{
	OnPingComplete = Ref;
	
	if (IOnlineSubsystem* OnlineSub = Online::GetSubsystem(GetWorld()))
	{
		if (IOnlineSessionPtr Sessions = OnlineSub->GetSessionInterface())
		{
			FString Address;
			if (Sessions->GetResolvedConnectString(SessionResult.OnlineResult, NAME_BeaconPort, Address))
			{
				UE_LOG(LogEOSKitPingClient, Log, TEXT("Resolved Address: %s"), *Address);
				return ConnectToHost(Address, 0, false, Ref);
			}
			else
			{
				UE_LOG(LogEOSKitPingClient, Warning, TEXT("Failed to resolve connect string for session"));
			}
		}
	}
	
	UE_LOG(LogEOSKitPingClient, Error, TEXT("Failed to connect to session"));
	OnPingComplete.ExecuteIfBound(0, false);
	return false;
}

void AEOSKitPingClient::Disconnect()
{
	DestroyBeacon();
}

