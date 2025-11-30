// Copyright (C) 2024, All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintAsyncActionBase.h"
#include "EOSKitBaseWebApi.generated.h"

/**
 * Base response structure for Web API calls
 */
USTRUCT(BlueprintType)
struct FEOSKitWebApiResponse
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, Category = "EOSKit|Web")
	int32 StatusCode = -1;

	UPROPERTY(BlueprintReadWrite, Category = "EOSKit|Web")
	FString Response = TEXT("");

	FEOSKitWebApiResponse()
		: StatusCode(-1)
		, Response(TEXT(""))
	{
	}

	FEOSKitWebApiResponse(int32 InStatusCode, const FString& InResponse)
		: StatusCode(InStatusCode)
		, Response(InResponse)
	{
	}
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnEOSKitWebApiResponse, bool, bSuccess, const FEOSKitWebApiResponse&, Response);

/**
 * Base class for EOSKit Web API functions
 */
UCLASS()
class EOSKITWEB_API UEOSKitBaseWebApi : public UBlueprintAsyncActionBase
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintAssignable, Category = "EOSKit|Web")
	FOnEOSKitWebApiResponse OnResponse;

protected:
	void OnResponseReceived(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful);
	void DestroyAsyncTask();
	FString APIEndpoint = TEXT("https://api.epicgames.dev");
};

