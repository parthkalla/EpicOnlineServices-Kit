// Copyright (C) 2024, All Rights Reserved.

#include "EOSCalculatePingForSessionAsync.h"
#include "EOSKitPingClient.h"
#include "Engine/World.h"
#include "OnlineSubsystemUtils.h"
#include "Interfaces/OnlineSessionInterface.h"

UEOSCalculatePingForSessionAsync* UEOSCalculatePingForSessionAsync::CalculatePingForSession(
	UObject* WorldContextObject,
	const FBlueprintSessionResult& SessionResult)
{
	UEOSCalculatePingForSessionAsync* BlueprintNode = NewObject<UEOSCalculatePingForSessionAsync>();
	BlueprintNode->WorldContextObject = WorldContextObject;
	BlueprintNode->SessionResult = SessionResult;
	return BlueprintNode;
}

void UEOSCalculatePingForSessionAsync::Activate()
{
	Super::Activate();

	if (!WorldContextObject)
	{
		UE_LOG(LogTemp, Error, TEXT("EOSCalculatePingForSessionAsync: WorldContextObject is null"));
		OnFailure.Broadcast(0);
		SetReadyToDestroy();
		return;
	}

	UWorld* World = WorldContextObject->GetWorld();
	if (!World)
	{
		UE_LOG(LogTemp, Error, TEXT("EOSCalculatePingForSessionAsync: World is null"));
		OnFailure.Broadcast(0);
		SetReadyToDestroy();
		return;
	}

	// Spawn ping client and connect using the session result
	if (AEOSKitPingClient* PingClient = World->SpawnActor<AEOSKitPingClient>(AEOSKitPingClient::StaticClass()))
	{
		FEOSKitPingComplete OnPingComplete;
		OnPingComplete.BindDynamic(this, &UEOSCalculatePingForSessionAsync::OnPingComplete);
		
		if (!PingClient->ConnectToSession(SessionResult, OnPingComplete))
		{
			UE_LOG(LogTemp, Error, TEXT("EOSCalculatePingForSessionAsync: Failed to connect ping client"));
			OnFailure.Broadcast(0);
			SetReadyToDestroy();
		}
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("EOSCalculatePingForSessionAsync: Failed to spawn PingClient"));
		OnFailure.Broadcast(0);
		SetReadyToDestroy();
	}
}

void UEOSCalculatePingForSessionAsync::OnPingComplete(int32 Ping, bool bSuccess)
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

