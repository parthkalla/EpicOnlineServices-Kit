// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "Runtime/Launch/Resources/Version.h"
#if ENGINE_MAJOR_VERSION == 5
#include "Online/CoreOnline.h"
#else
#include "UObject/CoreOnline.h"
#endif
#include "Kismet/BlueprintAsyncActionBase.h"
#include "EOK_SetPlayerData_AsyncFunction.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FSetDataResult);

UCLASS()
class OnlineSubsystemEOK_API UEOK_SetPlayerData_AsyncFunction : public UBlueprintAsyncActionBase
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintAssignable)
	FSetDataResult OnSuccess;
	
	UPROPERTY(BlueprintAssignable)
	FSetDataResult OnFail;

	bool bDelegateCalled = false;

	FString FileName;
	TArray<uint8> DataToSave;
	/*
	This C++ method updates the player data in the online subsystem using the selected method and sets up a callback function to handle the response.
	Documentation link: https://betide-studio.gitbook.io/eos-integration-kit/playerdata/
	For Input Parameters, please refer to the documentation link above.
	*/
	UFUNCTION(BlueprintCallable, DisplayName="Set EOK Player Storage", meta = (BlueprintInternalUseOnly = "true"), Category="Epic Online Services-Kit V2 || Storage")
	static UEOK_SetPlayerData_AsyncFunction* SetPlayerData(FString FileName, const TArray<uint8>& DataToSave);

	virtual void Activate() override;

	void SetPlayerData();

	void OnWriteFileComplete(bool bSuccess, const FUniqueNetId& UserID, const FString& Var_FileName);

};
