// Copyright (c) 2025 Asrock Studios. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintAsyncActionBase.h"
#include "GPGS_ReadSavedGame.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FGPGS_ReadSavedGameCallbackSignature, const FString&, ID, const TArray<uint8>&, Data, const FString&, Error);

/**
 * 
 */
UCLASS()
class EOKLoginMethods_API UGPGS_ReadSavedGame : public UBlueprintAsyncActionBase
{
	GENERATED_BODY()

public:
	/**
	 * Read Saved Game from Google Play Games
	 * @param ID ID of the Saved Game
	 * @return SavedGame Data
	 */
	UFUNCTION(BlueprintCallable, meta=(BlueprintInternalUseOnly="true", WorldContext="WorldContextObject"), Category="Epic Online Services-Kit V2|Google Play Games|Saved Games")
	static UGPGS_ReadSavedGame* ReadSavedGame(UObject* WorldContextObject, const FString& ID);

	virtual void Activate() override;
	virtual void BeginDestroy() override;

	// Executed after SavedGame read successfully
	UPROPERTY(BlueprintAssignable, Category="Epic Online Services-Kit V2|Google Play Games")
	FGPGS_ReadSavedGameCallbackSignature Success;

	// Executed if there was an error loading the Saved Game
	UPROPERTY(BlueprintAssignable, Category="Epic Online Services-Kit V2|Google Play Games")
	FGPGS_ReadSavedGameCallbackSignature Failure;
	
	static TWeakObjectPtr<UGPGS_ReadSavedGame> StaticInstance;

private:
	FString Var_ID;
};
