// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintAsyncActionBase.h"
#include "OnlineSubsystemUtils.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "OnlineSubsystem.h"
#include "Engine/LocalPlayer.h"
#include "Interfaces/OnlineIdentityInterface.h"
#include "Kismet/GameplayStatics.h"
#include "OnlineSubsystemEOK/Subsystem/EOK_Subsystem.h"
#include "EOK_CreateLobby_AsyncFunction.generated.h"

USTRUCT(BlueprintType)
struct FCreateLobbySettings
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, Category="Epic Online Services-Kit V2")
	bool bIsLanMatch = false;

	UPROPERTY(BlueprintReadWrite, Category="Epic Online Services-Kit V2")
	bool bAllowInvites = true;

	UPROPERTY(BlueprintReadWrite, Category="Epic Online Services-Kit V2")
	int32 NumberOfPrivateConnections = 0;

	UPROPERTY(BlueprintReadWrite, Category="Epic Online Services-Kit V2")
	bool bShouldAdvertise = true;

	UPROPERTY(BlueprintReadWrite, Category="Epic Online Services-Kit V2")
	bool bAllowJoinInProgress = true;

	UPROPERTY(BlueprintReadWrite, Category="Epic Online Services-Kit V2")
	ERegionInfo Region = ERegionInfo::RE_NoSelection;

	UPROPERTY(BlueprintReadWrite, Category="Epic Online Services-Kit V2")
	bool bUseVoiceChat = false;

	UPROPERTY(BlueprintReadWrite, Category="Epic Online Services-Kit V2")
	bool bUsePresence = false;

	UPROPERTY(BlueprintReadWrite, Category="Epic Online Services-Kit V2")
	FString BucketID = "";

	UPROPERTY(BlueprintReadWrite, Category="Epic Online Services-Kit V2")
	bool bSupportHostMigration = false;

	UPROPERTY(BlueprintReadWrite, Category="Epic Online Services-Kit V2")
	bool bEnableJoinViaID = false;

	UPROPERTY(BlueprintReadWrite, Category="Epic Online Services-Kit V2")
	FString LobbyIDOverride = "";
	
};


DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FCreateLobby_Delegate, const FString&, LobbyID);

UCLASS()
class OnlineSubsystemEOK_API UEOK_CreateLobby_AsyncFunction : public UBlueprintAsyncActionBase
{
	GENERATED_BODY()

public:
	int32 NumberOfPublicConnections;
	TMap<FString, FEOKAttribute> SessionSettings;
	TMap<FString, FEOKAttribute> MemberSettings;
	FCreateLobbySettings Var_CreateLobbySettings;
	bool bDelegateCalled = false;
	FName VSessionName;
	
	UPROPERTY(BlueprintAssignable, DisplayName="Success")
	FCreateLobby_Delegate OnSuccess;
	UPROPERTY(BlueprintAssignable, DisplayName="Failure")
	FCreateLobby_Delegate OnFail;

	virtual void Activate() override;

	void CreateLobby();

	void OnCreateLobbyCompleted(FName SessionName, bool bWasSuccessful);

	/*
	This function is used to create a lobby with the given settings and returns a result delegate which can be used to determine if the lobby was created successfully or not.
	@param SessionSettings - A map of session settings to be used when creating the lobby.
	@param NumberOfPublicConnections - The number of public connections to be used when creating the lobby.
	@param ExtraSettings - A struct containing extra settings to be used when creating the lobby which is completely optional.
	Documentation link: https://EOK.betide.studio/multiplayer/sessions/lobbies/
	*/
	UFUNCTION(BlueprintCallable, DisplayName="Create EOK Lobby",meta = (BlueprintInternalUseOnly = "true",AutoCreateRefTerm="SessionSettings,MemberSettings" ), Category="Epic Online Services-Kit V2 || Sessions")
	static UEOK_CreateLobby_AsyncFunction* CreateEOKLobby(
		TMap<FString, FEOKAttribute> SessionSettings,
		TMap<FString, FEOKAttribute> MemberSettings,
		FName SessionName,
		int32 NumberOfPublicConnections,
		FCreateLobbySettings ExtraSettings);
};
