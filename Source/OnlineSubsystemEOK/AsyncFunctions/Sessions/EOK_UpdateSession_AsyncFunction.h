// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "OnlineSubsystemUtils.h"
#include "OnlineSubsystem.h"
#include "Kismet/BlueprintAsyncActionBase.h"
#include "OnlineSubsystemEOK/Subsystem/EOK_Subsystem.h"
#include "EOK_UpdateSession_AsyncFunction.generated.h"


USTRUCT(BlueprintType)
struct FEOK_MemberSpecificAttribute
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, Category="Epic Online Services-Kit V2")
	FEOKUniqueNetId MemberId;

	UPROPERTY(BlueprintReadWrite, Category="Epic Online Services-Kit V2")
	TMap<FString, FEOKAttribute> Attributes;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FUpdateSessionDelegate);

UCLASS()
class OnlineSubsystemEOK_API UEOK_UpdateSession_AsyncFunction : public UBlueprintAsyncActionBase
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, DisplayName="Update EOK Session",
		meta = (BlueprintInternalUseOnly = "true", WorldContext = "WorldContextObject"),
		Category="Epic Online Services-Kit V2 || Sessions")
	static UEOK_UpdateSession_AsyncFunction* UpdateEOKSessions(UObject* WorldContextObject,
	                                                           TMap<FString, FEOKAttribute> SessionSettings,
	                                                           TArray<FEOK_MemberSpecificAttribute> MemberSettings,
	                                                           FName SessionName = "GameSession",
	                                                           bool bShouldAdvertise = true,
	                                                           bool bAllowJoinInProgress = true,
	                                                           bool bAllowInvites = true,
	                                                           bool bUsesPresence = false,
	                                                           bool bIsLANMatch = false,
	                                                           bool bIsDedicatedServer = false,
	                                                           bool bIsLobbySession = false,
	                                                           int32 NumberOfPublicConnections = 10,
	                                                           int32 NumberOfPrivateConnections = 10);

	
	UPROPERTY()
	UObject* Var_WorldContextObject;
	TMap<FString, FEOKAttribute> Var_SessionSettings;
	TArray<FEOK_MemberSpecificAttribute> Var_MemberSettings;
	bool Var_bShouldAdvertise;
	bool Var_bAllowJoinInProgress;
	bool Var_bAllowInvites;
	bool Var_bIsLANMatch;
	bool Var_bIsDedicatedServer;
	bool Var_bUsesPresence;
	int32 Var_NumberOfPublicConnections;
	int32 Var_NumberOfPrivateConnections;
	FName Var_SessionName;
	bool Var_bIsLanMatch;
	bool Var_bIsLobbySession;

	void OnUpdateSessionComplete(FName Name, bool bArg);
	virtual void Activate() override;

	UPROPERTY(BlueprintAssignable)
	FUpdateSessionDelegate OnSuccess;

	UPROPERTY(BlueprintAssignable)
	FUpdateSessionDelegate OnFailure;
};
