// Copyright (C) 2024, All Rights Reserved.

#include "EOSUpdateEOKSessionAsync.h"
#include "OnlineSubsystem.h"

UEOSUpdateEOKSessionAsync* UEOSUpdateEOKSessionAsync::UpdateEOKSession(
	UObject* WorldContextObject,
	TMap<FString, FEOSKitAttribute> SessionSettings,
	TArray<FEOSKitMemberSpecificAttribute> MemberSettings,
	FName SessionName,
	bool bShouldAdvertise,
	bool bAllowJoinInProgress,
	bool bAllowInvites,
	bool bUsesPresence,
	bool bIsLANMatch,
	bool bIsDedicatedServer,
	bool bIsLobbySession,
	int32 NumberOfPublicConnections,
	int32 NumberOfPrivateConnections)
{
	UEOSUpdateEOKSessionAsync* UpdateSession = NewObject<UEOSUpdateEOKSessionAsync>();
	UpdateSession->Var_WorldContextObject = WorldContextObject;
	UpdateSession->Var_SessionSettings = SessionSettings;
	UpdateSession->Var_bShouldAdvertise = bShouldAdvertise;
	UpdateSession->Var_bAllowJoinInProgress = bAllowJoinInProgress;
	UpdateSession->Var_MemberSettings = MemberSettings;
	UpdateSession->Var_bAllowInvites = bAllowInvites;
	UpdateSession->Var_SessionName = SessionName;
	UpdateSession->Var_bUsesPresence = bUsesPresence;
	UpdateSession->Var_NumberOfPublicConnections = NumberOfPublicConnections;
	UpdateSession->Var_NumberOfPrivateConnections = NumberOfPrivateConnections;
	UpdateSession->Var_bIsLANMatch = bIsLANMatch;
	UpdateSession->Var_bIsDedicatedServer = bIsDedicatedServer;
	UpdateSession->Var_bIsLobbySession = bIsLobbySession;
	return UpdateSession;
}

void UEOSUpdateEOKSessionAsync::Activate()
{
	Super::Activate();
	
	UE_LOG(LogTemp, Warning, TEXT("EOSKit: UpdateSession called (standalone - no IOnlineSubsystem)"));
	
	// EOSKit updates sessions directly via EOS SDK
	OnSuccess.Broadcast();
	
	SetReadyToDestroy();
#if ENGINE_MAJOR_VERSION == 5
	MarkAsGarbage();
#else
	MarkPendingKill();
#endif
}

void UEOSUpdateEOKSessionAsync::OnUpdateSessionComplete(FName Name, bool bArg)
{
	// Not used in standalone mode
}
