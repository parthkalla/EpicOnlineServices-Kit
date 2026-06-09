// Copyright (c) 2024 Betide Studio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintAsyncActionBase.h"
#include "Runtime/Launch/Resources/Version.h"
#include "Engine/World.h"
#include "OnlineSubsystemEOK/Subsystem/EOK_Subsystem.h"
#include "EOK_CalculatePingForSession_AsyncFunction.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FEOK_OnCalculatePingForSessionComplete, int32, Ping);
UCLASS()
class OnlineSubsystemEOK_API UEOK_CalculatePingForSession_AsyncFunction : public UBlueprintAsyncActionBase
{
	GENERATED_BODY()

public:

	UFUNCTION(BlueprintCallable, Category = "Epic Online Services-Kit V2 | Beacons | Ping", meta = (BlueprintInternalUseOnly = "true", WorldContext = "WorldContextObject"), DisplayName = "Calculate Ping For Session")
	static UEOK_CalculatePingForSession_AsyncFunction* CalculatePingForSession(FSessionFindStruct Session, UObject* WorldContextObject);

	UPROPERTY(BlueprintAssignable, Category = "Epic Online Services-Kit V2 | Beacons | Ping")
	FEOK_OnCalculatePingForSessionComplete OnSuccess;

	UPROPERTY(BlueprintAssignable, Category = "Epic Online Services-Kit V2 | Beacons | Ping")
	FEOK_OnCalculatePingForSessionComplete OnFailure;

private:
	UFUNCTION()
	void OnPingComplete(int32 Ping, bool bSuccess);
	virtual void Activate() override;

	FSessionFindStruct Var_SessionFindStruct;

	UPROPERTY()
	UObject* WorldContextObject;
};
