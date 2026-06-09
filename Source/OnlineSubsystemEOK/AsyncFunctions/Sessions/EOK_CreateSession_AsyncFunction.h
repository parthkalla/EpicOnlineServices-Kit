// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintAsyncActionBase.h"
#include "OnlineSubsystemUtils.h"
#include "OnlineSubsystem.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "Engine/LocalPlayer.h"
#include "Interfaces/OnlineIdentityInterface.h"
#include "Kismet/GameplayStatics.h"
#include "OnlineSubsystemEOK/Subsystem/EOK_Subsystem.h"
#include "EOK_CreateSession_AsyncFunction.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FCreateSession_Delegate, const FString&, SessionID);

USTRUCT(BlueprintType)
struct FDedicatedServerSettings
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, Category="Epic Online Services-Kit V2")
	bool bIsDedicatedServer = false;

	UPROPERTY(BlueprintReadWrite, Category="Epic Online Services-Kit V2")
	int32 PortInfo = 7777;
};

USTRUCT(BlueprintType)
struct FCreateSessionExtraSettings
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, Category="Epic Online Services-Kit V2")
	bool bIsLanMatch = false;

	UPROPERTY(BlueprintReadWrite, Category="Epic Online Services-Kit V2")
	int32 NumberOfPrivateConnections = 0;

	UPROPERTY(BlueprintReadWrite, Category="Epic Online Services-Kit V2")
	bool bShouldAdvertise = true;

	UPROPERTY(BlueprintReadWrite, Category="Epic Online Services-Kit V2")
	bool bAllowJoinInProgress = true;

	UPROPERTY(BlueprintReadWrite, Category="Epic Online Services-Kit V2")
	ERegionInfo Region = ERegionInfo::RE_NoSelection;

	UPROPERTY(BlueprintReadWrite, Category="Epic Online Services-Kit V2")
	bool bUsePresence = false;

	UPROPERTY(BlueprintReadWrite, Category="Epic Online Services-Kit V2")
	bool bAllowJoinViaPresence = false;
	
	UPROPERTY(BlueprintReadWrite, Category="Epic Online Services-Kit V2")
	bool bAllowJoinViaPresenceFriendsOnly = false;

	UPROPERTY(BlueprintReadWrite, Category="Epic Online Services-Kit V2")
	bool bEnforceSanctions = false;
};

UCLASS()
class OnlineSubsystemEOK_API UEOK_CreateSession_AsyncFunction : public UBlueprintAsyncActionBase
{
	GENERATED_BODY()

public:
	int32 NumberOfPublicConnections;
	TMap<FString, FEOKAttribute> SessionSettings;
	FDedicatedServerSettings DedicatedServerSettings;
	FCreateSessionExtraSettings ExtraSettings;
	FName VSessionName;

	bool bDelegateCalled = false;
	UPROPERTY(BlueprintAssignable)
	FCreateSession_Delegate OnSuccess;
	UPROPERTY(BlueprintAssignable)
	FCreateSession_Delegate OnFail;

	virtual void Activate() override;

	void CreateSession();

	void OnCreateSessionCompleted(FName SessionName, bool bWasSuccessful);

	/*
	This C++ method creates a session in EOS using the selected method and sets up a callback function to handle the response.
	Documentation link: https://betide-studio.gitbook.io/eos-integration-kit/sessions/
	For Input Parameters, please refer to the documentation link above.
	*/
	UFUNCTION(BlueprintCallable, DisplayName="Create EOK Session", meta = (BlueprintInternalUseOnly = "true",AutoCreateRefTerm=SessionSettings), Category="Epic Online Services-Kit V2 || Sessions")
	static UEOK_CreateSession_AsyncFunction* CreateEOKSession(
        TMap<FString, FEOKAttribute> SessionSettings,
        FName SessionName,
		int32 NumberOfPublicConnections ,
		FDedicatedServerSettings DedicatedServerSettings, 
		FCreateSessionExtraSettings ExtraSettings
	);};
