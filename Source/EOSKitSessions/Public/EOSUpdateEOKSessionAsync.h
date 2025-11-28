// Copyright (C) 2024, All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintAsyncActionBase.h"
#include "OnlineSubsystemUtils.h"
#include "OnlineSubsystem.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "EOSKitSessionStructs.h"
#include "EOSUpdateEOKSessionAsync.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FUpdateEOKSession_Delegate);

/**
 * Update EOK Session - Async Blueprint node for updating EOS sessions
 */
UCLASS()
class EOSKITSESSIONS_API UEOSUpdateEOKSessionAsync : public UBlueprintAsyncActionBase
{
	GENERATED_BODY()

public:
	/**
	 * Update an existing EOS Kit Session with new settings
	 * @param WorldContextObject - World context
	 * @param SessionSettings - A map of session settings to update
	 * @param MemberSettings - Member-specific attributes to update
	 * @param SessionName - Name of the session to update
	 * @param bShouldAdvertise - Should the session be advertised
	 * @param bAllowJoinInProgress - Allow joining while in progress
	 * @param bAllowInvites - Allow invites
	 * @param bUsesPresence - Use presence
	 * @param bIsLANMatch - Is this a LAN match
	 * @param bIsDedicatedServer - Is this a dedicated server
	 * @param bIsLobbySession - Is this a lobby session
	 * @param NumberOfPublicConnections - Number of public connections
	 * @param NumberOfPrivateConnections - Number of private connections
	 */
	UFUNCTION(BlueprintCallable, DisplayName="Update EOK Session",
		meta = (BlueprintInternalUseOnly = "true", WorldContext = "WorldContextObject", AutoCreateRefTerm="SessionSettings,MemberSettings"),
		Category="EOSKit|Sessions")
	static UEOSUpdateEOKSessionAsync* UpdateEOKSession(
		UObject* WorldContextObject,
		TMap<FString, FEOSKitAttribute> SessionSettings,
		TArray<FEOSKitMemberSpecificAttribute> MemberSettings,
		FName SessionName = FName("GameSession"),
		bool bShouldAdvertise = true,
		bool bAllowJoinInProgress = true,
		bool bAllowInvites = true,
		bool bUsesPresence = false,
		bool bIsLANMatch = false,
		bool bIsDedicatedServer = false,
		bool bIsLobbySession = false,
		int32 NumberOfPublicConnections = 10,
		int32 NumberOfPrivateConnections = 10
	);

	UPROPERTY()
	UObject* Var_WorldContextObject;
	
	TMap<FString, FEOSKitAttribute> Var_SessionSettings;
	TArray<FEOSKitMemberSpecificAttribute> Var_MemberSettings;
	bool Var_bShouldAdvertise;
	bool Var_bAllowJoinInProgress;
	bool Var_bAllowInvites;
	bool Var_bIsLANMatch;
	bool Var_bIsDedicatedServer;
	bool Var_bUsesPresence;
	int32 Var_NumberOfPublicConnections;
	int32 Var_NumberOfPrivateConnections;
	FName Var_SessionName;
	bool Var_bIsLobbySession;

	void OnUpdateSessionComplete(FName Name, bool bArg);
	virtual void Activate() override;

	UPROPERTY(BlueprintAssignable, DisplayName="Success")
	FUpdateEOKSession_Delegate OnSuccess;

	UPROPERTY(BlueprintAssignable, DisplayName="Failure")
	FUpdateEOKSession_Delegate OnFailure;
};
