// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintAsyncActionBase.h"
#include "OnlineSubsystemEOK/SdkFunctions/EOK_SharedFunctionFile.h"
#include "EOK_SanctionsAsyncFunction.generated.h"


DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FSanctionsDelegate, const TArray<FEOK_Sanctions_PlayerSanction>&, Sanctions);

UCLASS()
class OnlineSubsystemEOK_API UEOK_SanctionsAsyncFunction : public UBlueprintAsyncActionBase
{
	GENERATED_BODY()

public:
	/*
	This C++ method gets the logged in user's achievements from the EOS backend.
	Documentation link: https://betide-studio.gitbook.io/eos-integration-kit/authentication/
	*/
	UFUNCTION(BlueprintCallable, DisplayName="Get EOK Player Sanctions",meta = (BlueprintInternalUseOnly = "true"), Category="Epic Online Services-Kit V2 || Others")
	static UEOK_SanctionsAsyncFunction* GetEOKPlayerSanctionsAsyncFunction(FString LocalProductUserID, FString TargetProductUserID);

	FString Var_LocalProductUserID;
	FString Var_TargetProductUserID;

	UPROPERTY(BlueprintAssignable, Category = "Epic Online Services-Kit V2 | Sanctions")
	FSanctionsDelegate Success;

	UPROPERTY(BlueprintAssignable, Category = "Epic Online Services-Kit V2 | Sanctions")
	FSanctionsDelegate Failure;
	
	void Func_GetSanctions();

	static void EOS_CALL ReturnFunc(const EOS_Sanctions_QueryActivePlayerSanctionsCallbackInfo* Data);

	void GetFinalValues();
	void FireFailure();
	void Activate() override;
};
