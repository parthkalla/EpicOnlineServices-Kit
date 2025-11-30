// Copyright (C) 2024, All Rights Reserved.

#include "EOSCalculatePingForIpAsync.h"
#include "EOSKitPingClient.h"
#include "Engine/World.h"

UEOSCalculatePingForIpAsync* UEOSCalculatePingForIpAsync::CalculatePingForIp(
	UObject* WorldContextObject,
	const FString& IpAddress,
	int32 Port,
	bool bPortOverride)
{
	UEOSCalculatePingForIpAsync* BlueprintNode = NewObject<UEOSCalculatePingForIpAsync>();
	BlueprintNode->WorldContextObject = WorldContextObject;
	BlueprintNode->IpAddress = IpAddress;
	BlueprintNode->Port = Port;
	BlueprintNode->bPortOverride = bPortOverride;
	return BlueprintNode;
}

void UEOSCalculatePingForIpAsync::Activate()
{
	Super::Activate();

	if (!WorldContextObject)
	{
		UE_LOG(LogTemp, Error, TEXT("EOSCalculatePingForIpAsync: WorldContextObject is null"));
		OnFailure.Broadcast(0);
		SetReadyToDestroy();
		return;
	}

	UWorld* World = WorldContextObject->GetWorld();
	if (!World)
	{
		UE_LOG(LogTemp, Error, TEXT("EOSCalculatePingForIpAsync: World is null"));
		OnFailure.Broadcast(0);
		SetReadyToDestroy();
		return;
	}

	// Spawn ping client
	if (AEOSKitPingClient* PingClient = World->SpawnActor<AEOSKitPingClient>(AEOSKitPingClient::StaticClass()))
	{
		FEOSKitPingComplete OnPingComplete;
		OnPingComplete.BindDynamic(this, &UEOSCalculatePingForIpAsync::OnPingComplete);
		
		if (!PingClient->ConnectToHost(IpAddress, Port, bPortOverride, OnPingComplete))
		{
			UE_LOG(LogTemp, Error, TEXT("EOSCalculatePingForIpAsync: Failed to connect ping client"));
			OnFailure.Broadcast(0);
			SetReadyToDestroy();
		}
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("EOSCalculatePingForIpAsync: Failed to spawn PingClient"));
		OnFailure.Broadcast(0);
		SetReadyToDestroy();
	}
}

void UEOSCalculatePingForIpAsync::OnPingComplete(int32 Ping, bool bSuccess)
{
	if (bSuccess)
	{
		OnSuccess.Broadcast(Ping);
	}
	else
	{
		OnFailure.Broadcast(Ping);
	}
	SetReadyToDestroy();
}

