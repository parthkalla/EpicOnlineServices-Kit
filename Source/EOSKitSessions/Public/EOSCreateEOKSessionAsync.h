// Copyright (C) 2024, All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintAsyncActionBase.h"
#include "OnlineSubsystemUtils.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "OnlineSubsystem.h"
#include "Engine/LocalPlayer.h"
#include "Interfaces/OnlineIdentityInterface.h"
#include "Kismet/GameplayStatics.h"
#include "EOSKitSessionStructs.h"
#include "EOSCreateEOKSessionAsync.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FCreateEOKSession_Delegate, const FString&, SessionID);

/**
 * Create EOK Session - Async Blueprint node for creating EOS sessions
 */
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
	
	// Store the world context object
	UPROPERTY()
	TObjectPtr<UObject> CachedWorldContextObject;

	UPROPERTY(BlueprintAssignable, DisplayName="Success")
	FCreateEOKSession_Delegate OnSuccess;
	
	UPROPERTY(BlueprintAssignable, DisplayName="Failure")
	FCreateEOKSession_Delegate OnFail;

	virtual void Activate() override;

	void CreateSession();

	void OnCreateSessionCompleted(FName SessionName, bool bWasSuccessful);

	/**
	 * This C++ method creates a session in EOS using the selected method and sets up a callback function to handle the response.
	 * @param WorldContextObject - The world context object (usually 'self' in Blueprint).
	 * @param SessionSettings - A map of session settings to be used when creating the session.
	 * @param SessionName - The name to give the session locally.
	 * @param NumberOfPublicConnections - The number of public connections to be used when creating the session.
	 * @param DedicatedServerSettings - Settings for dedicated server configuration.
	 * @param ExtraSettings - A struct containing extra settings to be used when creating the session which is completely optional.
	 */
	UFUNCTION(BlueprintCallable, DisplayName="Create EOK Session", meta = (BlueprintInternalUseOnly = "true", WorldContext = "WorldContextObject", AutoCreateRefTerm="SessionSettings"), Category="EOSKit|Sessions")
	static UEOSCreateEOKSessionAsync* CreateEOKSession(
		UObject* WorldContextObject,
		TMap<FString, FEOSKitAttribute> SessionSettings,
		FName SessionName,
		int32 NumberOfPublicConnections,
		FEOSKitDedicatedServerSettings DedicatedServerSettings,
		FEOSKitCreateSessionSettings ExtraSettings
	);
};
