// Copyright (c) 2025 Asrock Studios. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintAsyncActionBase.h"
#include "GooglePlayGames/GooglePlayGamesStructures.h"
#include "GPGS_LoadFriends.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FGPGS_LoadFriendsCallbackSignature, const TArray<FGPGS_Player>&, Friends, const FString&, error);

/**
 * 
 */
UCLASS()
class EOKLoginMethods_API UGPGS_LoadFriends : public UBlueprintAsyncActionBase
{
	GENERATED_BODY()

public:
	/**
	 * Load Friends of the Currently Signed In Player
	 * @param Max Max amount of Friends to Load
	 * @param bForceReload Force Reload from server instead of using Cache
	 * @return Array of Friends data
	 */
	UFUNCTION(BlueprintCallable, meta=(BlueprintInternalUseOnly="true", WorldContext="WorldContextObject"), Category="Epic Online Services-Kit V2|Google Play Games|Friends")
	static UGPGS_LoadFriends* LoadFriends(UObject* WorldContextObject, int Max = 10, bool bForceReload = false);

	virtual void Activate() override;
	virtual void BeginDestroy() override;

	// Executed after Friends loaded successfully
	UPROPERTY(BlueprintAssignable, Category="Epic Online Services-Kit V2|Google Play Games")
	FGPGS_LoadFriendsCallbackSignature Success;

	// Executed if there was an error Loading Friends
	UPROPERTY(BlueprintAssignable, Category="Epic Online Services-Kit V2|Google Play Games")
	FGPGS_LoadFriendsCallbackSignature Failure;

	void HandleCallback(bool bSuccess, const FString& FriendsJson, const FString& Error);
	
	static TWeakObjectPtr<UGPGS_LoadFriends> StaticInstance;

private:
	int Var_Max;
	bool Var_bForceReload;
};
