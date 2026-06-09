// Copyright (c) 2023 Betide Studio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Interfaces/OnlineIdentityInterface.h"
#include "Kismet/BlueprintAsyncActionBase.h"
#include "OnlineSubsystem.h"
#include "OnlineSubsystemEOS.h"
#include "UserManagerEOS.h"
#include "EOK_GetAppleAuthToken_AsyncFunction.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FEOK_OnGetAppleAuthTokenComplete, const FString&, AuthToken);

UCLASS()
class OnlineSubsystemEOK_API UEOK_GetAppleAuthToken_AsyncFunction : public UBlueprintAsyncActionBase
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, meta = (BlueprintInternalUseOnly = "true"), DisplayName="Get Apple Auth Token", Category = "Epic Online Services-Kit V2|Extra")
	static UEOK_GetAppleAuthToken_AsyncFunction* GetAppleAuthToken();

	UPROPERTY(BlueprintAssignable)
	FEOK_OnGetAppleAuthTokenComplete OnSuccess;

	UPROPERTY(BlueprintAssignable)
	FEOK_OnGetAppleAuthTokenComplete OnFailure;

private:
	void OnLoginComplete(int LocalUserNum, bool bWasSuccess, const FUniqueNetId& UniqueNetId, const FString& Error);
	virtual void Activate() override;
};
