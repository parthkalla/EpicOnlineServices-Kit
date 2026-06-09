// Copyright (c) 2025 Asrock Studios. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintAsyncActionBase.h"
#include "GoogleLogout_SLK.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FGoogleSignOutCallback, const FString&, Error);

/**
 * 
 */
UCLASS()
class EOKLoginMethods_API UGoogleLogout_SLK : public UBlueprintAsyncActionBase
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, meta=(BlueprintInternalUseOnly="true", WorldContext="WorldContextObject"), Category="Epic Online Services-Kit V2|GoogleSubsystem")
	static UGoogleLogout_SLK* GoogleLogout(UObject* WorldContextObject);

	void Activate() override;
	void BeginDestroy() override;
	static TWeakObjectPtr<UGoogleLogout_SLK> staticInstance;

	UPROPERTY(BlueprintAssignable, Category="Epic Online Services-Kit V2|GoogleSubsystem")
	FGoogleSignOutCallback Success;

	UPROPERTY(BlueprintAssignable, Category="Epic Online Services-Kit V2|GoogleSubsystem")
	FGoogleSignOutCallback Failure;

	void GoogleLogoutLocal();
};
