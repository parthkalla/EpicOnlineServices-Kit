// Copyright (c) 2023 Betide Studio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Interfaces/OnlineIdentityInterface.h"
#include "Kismet/BlueprintAsyncActionBase.h"
#include "EOK_GetPlatformAuthToken_AsyncFunction.generated.h"

UENUM(BlueprintType)
enum EEOK_PlatformToUse
{
	Steam,
	Apple,
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnGetPlatformAuthTokenComplete, const FString&, AuthToken);

UCLASS()
class OnlineSubsystemEOK_API UEOK_GetPlatformAuthToken_AsyncFunction : public UBlueprintAsyncActionBase
{
	GENERATED_BODY()

public:

	UFUNCTION(BlueprintCallable, meta = (BlueprintInternalUseOnly = "true"), DisplayName="Get Steam Auth Token", Category = "Epic Online Services-Kit V2|Extra")
	static UEOK_GetPlatformAuthToken_AsyncFunction* GetPlatformAuthToken();
	
	UPROPERTY(BlueprintAssignable)
	FOnGetPlatformAuthTokenComplete OnSuccess;

	UPROPERTY(BlueprintAssignable)
	FOnGetPlatformAuthTokenComplete OnFailure;
private:
	void OnGetPlatformAuthTokenComplete(int I, bool bArg, const FExternalAuthToken& ExternalAuthToken);
	virtual void Activate() override;
	
};
