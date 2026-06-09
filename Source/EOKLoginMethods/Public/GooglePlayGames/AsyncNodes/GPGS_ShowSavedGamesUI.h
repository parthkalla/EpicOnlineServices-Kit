// Copyright (c) 2025 Asrock Studios. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintAsyncActionBase.h"
#include "GPGS_ShowSavedGamesUI.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FGPGS_ShowSavedGamesUIOutputSignature, const FString&, ID, const FString&, Error);

UCLASS()
class EOKLoginMethods_API UGPGS_ShowSavedGamesUI : public UBlueprintAsyncActionBase
{
	GENERATED_BODY()

public:
	/**
	 * Show the Saved Games UI from Google Play Games
	 * @param Max Max amount of SavedGames to show
	 */
	UFUNCTION(BlueprintCallable, meta=(BlueprintInternalUseOnly="true", WorldContext="WorldContextObject"), Category="Epic Online Services-Kit V2|Google Play Games|Saved Games")
	static UGPGS_ShowSavedGamesUI* ShowSavedGamesUI(UObject* WorldContextObject, int Max = 5);

	virtual void Activate() override;
	virtual void BeginDestroy() override;

	// Executed if a Saved Game is selected from the UI
	UPROPERTY(BlueprintAssignable, Category="Epic Online Services-Kit V2|Google Play Games")
	FGPGS_ShowSavedGamesUIOutputSignature Selected;

	// Executed if the '+' Floating Button is clicked from the UI
	UPROPERTY(BlueprintAssignable, Category="Epic Online Services-Kit V2|Google Play Games")
	FGPGS_ShowSavedGamesUIOutputSignature CreateNew;

	// Executed if Back was clicked from the UI
	UPROPERTY(BlueprintAssignable, Category="Epic Online Services-Kit V2|Google Play Games")
	FGPGS_ShowSavedGamesUIOutputSignature ClickedBack;

	// Executed if there was an error showing the UI
	UPROPERTY(BlueprintAssignable, Category="Epic Online Services-Kit V2|Google Play Games")
	FGPGS_ShowSavedGamesUIOutputSignature Failure;
	
	static TWeakObjectPtr<UGPGS_ShowSavedGamesUI> StaticInstance;

private:
	int32 Var_Max;
};
