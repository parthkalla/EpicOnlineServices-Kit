// Copyright (C) 2024, All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintAsyncActionBase.h"
#include "OnlineSubsystemUtils.h"
#include "OnlineSubsystem.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "Engine/LocalPlayer.h"
#include "Interfaces/OnlineIdentityInterface.h"
#include "Kismet/GameplayStatics.h"
#include "EOSKitSessionStructs.h"
#include "EOSCreateEOKSessionAsync.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FCreateEOKSession_Delegate, const FString&, SessionID);

UCLASS()
class EOSKITSESSIONS_API UEOSCreateEOKSessionAsync : public UBlueprintAsyncActionBase
{
	GENERATED_BODY()

public:
	int32 NumberOfPublicConnections;
	TMap<FString, FEOSKitAttribute> SessionSettings;
	FEOSKitDedicatedServerSettings DedicatedServerSettings;
	FEOSKitCreateSessionSettings ExtraSettings;
	FName VSessionName;

	bool bDelegateCalled = false;
	UPROPERTY(BlueprintAssignable)
	FCreateEOKSession_Delegate OnSuccess;
	UPROPERTY(BlueprintAssignable)
	FCreateEOKSession_Delegate OnFail;

	virtual void Activate() override;

	void CreateSession();

	void OnCreateSessionCompleted(FName SessionName, bool bWasSuccessful);
	
	// CRITICAL: Callback for StartSession (called automatically after CreateSession succeeds)
	void OnStartSessionCompleted(FName SessionName, bool bWasSuccessful);

	/*
	This C++ method creates a session in EOS using the selected method and sets up a callback function to handle the response.
	Documentation link: https://betide-studio.gitbook.io/eos-integration-kit/sessions/
	For Input Parameters, please refer to the documentation link above.
	*/
	UFUNCTION(BlueprintCallable, DisplayName="Create EOK Session", meta = (BlueprintInternalUseOnly = "true",AutoCreateRefTerm=SessionSettings), Category="EOSKit|Sessions")
	static UEOSCreateEOKSessionAsync* CreateEOKSession(
        TMap<FString, FEOSKitAttribute> SessionSettings,
        FName SessionName,
		int32 NumberOfPublicConnections ,
		FEOSKitDedicatedServerSettings DedicatedServerSettings, 
		FEOSKitCreateSessionSettings ExtraSettings
	);
};
