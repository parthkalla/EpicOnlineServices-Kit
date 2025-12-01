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
#include "EOSCreateEOKLobbyAsync.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FCreateEOKLobby_Delegate, const FString&, LobbyID);

/**
 * Create EOK Lobby - Async Blueprint node for creating EOS lobbies
 */
UCLASS()
class EOSKITSESSIONS_API UEOSCreateEOKLobbyAsync : public UBlueprintAsyncActionBase
{
	GENERATED_BODY()

public:
	int32 NumberOfPublicConnections;
	TMap<FString, FEOSKitAttribute> SessionSettings;
	TMap<FString, FEOSKitAttribute> MemberSettings;
	FEOSKitCreateLobbySettings Var_CreateLobbySettings;
	bool bDelegateCalled = false;
	FName VSessionName;
	
	// Store the world context object
	UPROPERTY()
	TObjectPtr<UObject> CachedWorldContextObject;
	
	UPROPERTY(BlueprintAssignable, DisplayName="Success")
	FCreateEOKLobby_Delegate OnSuccess;
	
	UPROPERTY(BlueprintAssignable, DisplayName="Failure")
	FCreateEOKLobby_Delegate OnFail;

	virtual void Activate() override;

	void CreateLobby();

	void OnCreateLobbyCompleted(FName SessionName, bool bWasSuccessful);
	
	void FinishAndCleanup();

	/**
	 * This function is used to create a lobby with the given settings and returns a result delegate which can be used to determine if the lobby was created successfully or not.
	 * @param WorldContextObject - The world context object (usually 'self' in Blueprint).
	 * @param SessionSettings - A map of session settings to be used when creating the lobby.
	 * @param MemberSettings - A map of member settings to be used when creating the lobby.
	 * @param SessionName - The name to give the session locally.
	 * @param NumberOfPublicConnections - The number of public connections to be used when creating the lobby.
	 * @param ExtraSettings - A struct containing extra settings to be used when creating the lobby which is completely optional.
	 */
	UFUNCTION(BlueprintCallable, DisplayName="Create EOK Lobby", meta = (BlueprintInternalUseOnly = "true", WorldContext = "WorldContextObject", AutoCreateRefTerm="SessionSettings,MemberSettings"), Category="EOSKit|Lobby")
	static UEOSCreateEOKLobbyAsync* CreateEOKLobby(
		UObject* WorldContextObject,
		TMap<FString, FEOSKitAttribute> SessionSettings,
		TMap<FString, FEOSKitAttribute> MemberSettings,
		FName SessionName,
		int32 NumberOfPublicConnections,
		FEOSKitCreateLobbySettings ExtraSettings);
};
