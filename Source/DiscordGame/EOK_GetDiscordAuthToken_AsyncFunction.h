// Copyright (c) 2023 Betide Studio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintAsyncActionBase.h"
#include "Runtime/Launch/Resources/Version.h"
#include "EOK_GetDiscordAuthToken_AsyncFunction.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FEOK_OnGetDiscordAuthTokenComplete, const FString&, AuthToken, const FString&, Error);

UCLASS()
class DISCORDGAME_API UEOK_GetDiscordAuthToken_AsyncFunction : public UBlueprintAsyncActionBase
{
	GENERATED_BODY()


public:
	UFUNCTION(BlueprintCallable, meta = (BlueprintInternalUseOnly = "true"), DisplayName="Get Discord Auth Token", Category = "Epic Online Services-Kit V2|Extra")
	static UEOK_GetDiscordAuthToken_AsyncFunction* GetDiscordAuthToken();

	UPROPERTY(BlueprintAssignable)
	FEOK_OnGetDiscordAuthTokenComplete OnSuccess;

	UPROPERTY(BlueprintAssignable)
	FEOK_OnGetDiscordAuthTokenComplete OnFailure;

private:
	virtual void Activate() override;
	
};
