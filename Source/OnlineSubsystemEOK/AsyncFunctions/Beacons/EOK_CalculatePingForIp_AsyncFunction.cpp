// Copyright (c) 2024 Betide Studio. All Rights Reserved.


#include "EOK_CalculatePingForIp_AsyncFunction.h"
#include "Engine/World.h"
#include "PingClient.h"

UEOK_CalculatePingForIp_AsyncFunction* UEOK_CalculatePingForIp_AsyncFunction::CalculatePingForIp(FString IpAddress,
                                                                                                 int32 Port, bool bPortOverride)
{
	UEOK_CalculatePingForIp_AsyncFunction* BlueprintNode = NewObject<UEOK_CalculatePingForIp_AsyncFunction>();
	BlueprintNode->Var_IpAddress = IpAddress;
	BlueprintNode->Var_Port = Port;
	BlueprintNode->Var_bPortOverride = bPortOverride;
	return BlueprintNode;
}

void UEOK_CalculatePingForIp_AsyncFunction::OnPingComplete(int32 Ping, bool bSuccess)
{
	if(bSuccess)
	{
		OnSuccess.Broadcast(Ping);
	}
	else
	{
		OnFailure.Broadcast(Ping);
	}
	SetReadyToDestroy();
#if ENGINE_MAJOR_VERSION == 5
	MarkAsGarbage();
#else
	MarkPendingKill();
#endif
}

void UEOK_CalculatePingForIp_AsyncFunction::Activate()
{
	Super::Activate();
	if(GetWorld())
	{
		if(APingClient* PingClient = GetWorld()->SpawnActor<APingClient>(APingClient::StaticClass()))
		{
			FEOK_PingComplete OnPingComplete;
			OnPingComplete.BindDynamic(this, &UEOK_CalculatePingForIp_AsyncFunction::OnPingComplete);
			if(!PingClient->ConnectToHost(Var_IpAddress, Var_Port, Var_bPortOverride, OnPingComplete))
			{
				OnFailure.Broadcast(0);
				SetReadyToDestroy();
#if ENGINE_MAJOR_VERSION == 5
				MarkAsGarbage();
#else
				MarkPendingKill();
#endif
			}
		}
		else
		{
			OnFailure.Broadcast(0);
			SetReadyToDestroy();
		}
	}
	else
	{
		OnFailure.Broadcast(0);
		SetReadyToDestroy();
	}
}
