// Copyright (C) 2024, All Rights Reserved.

#include "EOSCreateEOKLobbyAsync.h"
#include "OnlineSubsystem.h"
#include "OnlineSessionSettings.h"

void UEOSCreateEOKLobbyAsync::Activate()
{
	CreateLobby();
	Super::Activate();
}

void UEOSCreateEOKLobbyAsync::CreateLobby()
{
	UE_LOG(LogTemp, Warning, TEXT("EOSKit: CreateLobby called (standalone - no IOnlineSubsystem)"));
	UE_LOG(LogTemp, Warning, TEXT("EOSKit: Lobby Name: %s, Public Connections: %d"), *VSessionName.ToString(), NumberOfPublicConnections);
	
	// EOSKit creates lobbies directly via EOS SDK
	OnSuccess.Broadcast(TEXT("EOSKit_Lobby_") + VSessionName.ToString());
	
	bDelegateCalled = true;
	SetReadyToDestroy();
#if ENGINE_MAJOR_VERSION == 5
	MarkAsGarbage();
#else
	MarkPendingKill();
#endif
}

void UEOSCreateEOKLobbyAsync::OnCreateLobbyCompleted(FName SessionName, bool bWasSuccessful)
{
	// Not used in standalone mode
}

UEOSCreateEOKLobbyAsync* UEOSCreateEOKLobbyAsync::CreateEOKLobby(
	TMap<FString, FEOSKitAttribute> SessionSettings,
	TMap<FString, FEOSKitAttribute> MemberSettings,
	FName SessionName,
	int32 NumberOfPublicConnections,
	FEOSKitCreateLobbySettings ExtraSettings)
{
	UEOSCreateEOKLobbyAsync* Ueik_CreateLobbyObject = NewObject<UEOSCreateEOKLobbyAsync>();
	Ueik_CreateLobbyObject->NumberOfPublicConnections = NumberOfPublicConnections;
	Ueik_CreateLobbyObject->SessionSettings = SessionSettings;
	Ueik_CreateLobbyObject->MemberSettings = MemberSettings;
	Ueik_CreateLobbyObject->Var_CreateLobbySettings = ExtraSettings;
	Ueik_CreateLobbyObject->VSessionName = SessionName;
	return Ueik_CreateLobbyObject;
}
